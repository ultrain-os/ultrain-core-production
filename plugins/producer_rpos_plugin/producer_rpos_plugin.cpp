/**
 *  @file
 *  @copyright defined in ultrain/LICENSE.txt
 */
#include <ultrainio/producer_rpos_plugin/producer_rpos_plugin.hpp>
#include <ultrainio/chain/producer_object.hpp>
#include <ultrainio/chain/plugin_interface.hpp>
#include <ultrainio/chain/global_property_object.hpp>
#include <ultrainio/chain/resource_limits.hpp>
#include <ultrainio/chain/transaction_object.hpp>
#include <ultrainio/chain/merkle.hpp>
#include <ultrainio/chain/worldstate.hpp>

#include <fc/io/json.hpp>
#include <fc/smart_ref_impl.hpp>
#include <fc/scoped_exit.hpp>

#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <iostream>
#include <algorithm>
#include <ctime>
#include <chrono>
#include <tuple>
#include <boost/range/adaptor/map.hpp>
#include <boost/function_output_iterator.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/signals2/connection.hpp>

#include <base/StringUtils.h>
#include <rpos/Config.h>
#include <rpos/Genesis.h>
#include <rpos/Node.h>
#include <rpos/NodeInfo.h>
#include <core/utils.h>

namespace bmi = boost::multi_index;
using bmi::indexed_by;
using bmi::ordered_non_unique;
using bmi::member;
using bmi::tag;
using bmi::hashed_unique;

using boost::multi_index_container;

using std::string;
using std::vector;
using boost::signals2::scoped_connection;

// HACK TO EXPOSE LOGGER MAP

namespace fc {
   extern std::unordered_map<std::string,logger>& get_logger_map();
}

const fc::string logger_name("producer_rpos_plugin");
fc::logger _log;

namespace {
    bool failure_is_subjective(const fc::exception& e) {
        auto code = e.code();
        return ((code == ultrainio::chain::block_cpu_usage_exceeded::code_value) ||
                (code == ultrainio::chain::block_net_usage_exceeded::code_value));
    }
}

namespace ultrainio {

static appbase::abstract_plugin& _producer_rpos_plugin = app().register_plugin<producer_rpos_plugin>();

using namespace ultrainio::chain;
using namespace ultrainio::chain::plugin_interface;

struct transaction_id_with_expiry {
   transaction_id_type     trx_id;
   fc::time_point          expiry;
};

struct by_id;
struct by_expiry;

using transaction_id_with_expiry_index = multi_index_container<
   transaction_id_with_expiry,
   indexed_by<
      hashed_unique<tag<by_id>, BOOST_MULTI_INDEX_MEMBER(transaction_id_with_expiry, transaction_id_type, trx_id)>,
      ordered_non_unique<tag<by_expiry>, BOOST_MULTI_INDEX_MEMBER(transaction_id_with_expiry, fc::time_point, expiry)>
   >
>;


#define CATCH_AND_CALL(NEXT)\
   catch ( const fc::exception& err ) {\
      NEXT(err.dynamic_copy_exception());\
   } catch ( const std::exception& e ) {\
      fc::exception fce( \
         FC_LOG_MESSAGE( warn, "rethrow ${what}: ", ("what",e.what())),\
         fc::std_exception_code,\
         BOOST_CORE_TYPEID(e).name(),\
         e.what() ) ;\
      NEXT(fce.dynamic_copy_exception());\
   } catch( ... ) {\
      fc::unhandled_exception e(\
         FC_LOG_MESSAGE(warn, "rethrow"),\
         std::current_exception());\
      NEXT(e.dynamic_copy_exception());\
   }

class producer_rpos_plugin_impl : public std::enable_shared_from_this<producer_rpos_plugin_impl> {
   public:
      producer_rpos_plugin_impl(boost::asio::io_service& io)
      : _transaction_ack_channel(app().get_channel<compat::channels::transaction_ack>())
      {
      }

      boost::program_options::variables_map _options;
      std::string _genesis_time = std::string();
      std::string _my_sk_as_committee;
      std::string _my_sk_as_account;
      std::string _my_bls_sk;
      std::string _my_account_as_committee;
      std::string _genesis_pk                      = Genesis::s_genesisPk;
      bool     _pause_production                   = false;
      bool     _is_non_producing_node              = false;
      bool     _is_config_encrypt                  = true;
      bool     _allow_report_evil                  = false;
      int32_t  _genesis_startup_time               = Genesis::s_genesisStartupTime;
      int32_t  _max_round_seconds                  = Config::s_maxRoundSeconds;
      int32_t  _max_phase_seconds                  = Config::s_maxPhaseSeconds;
      int32_t  _max_trxs_seconds                   = Config::s_maxTrxMicroSeconds;
      using signature_provider_type = std::function<chain::signature_type(chain::digest_type)>;
      std::map<chain::public_key_type, signature_provider_type> _signature_providers;
      std::set<chain::account_name>                             _producers;
      transaction_id_with_expiry_index                          _persistent_transactions;

      int32_t                                                   _max_transaction_time_ms;
      fc::microseconds                                          _max_irreversible_block_age_us;
      fc::time_point                                            _irreversible_block_time;
      fc::microseconds                                          _kultraind_provider_timeout_us;

      time_point _last_signed_block_time;
      time_point _start_time = fc::time_point::now();
      uint32_t   _last_signed_block_num = 0;

      producer_rpos_plugin* _self = nullptr;

      incoming::channels::block::channel_type::handle         _incoming_block_subscription;
      incoming::channels::transaction::channel_type::handle   _incoming_transaction_subscription;

      compat::channels::transaction_ack::channel_type&        _transaction_ack_channel;

      incoming::methods::block_sync::method_type::handle        _incoming_block_sync_provider;
      incoming::methods::transaction_async::method_type::handle _incoming_transaction_async_provider;
      transaction_id_with_expiry_index                         _blacklisted_transactions;

      fc::optional<scoped_connection>                          _accepted_block_connection;
      fc::optional<scoped_connection>                          _irreversible_block_connection;

      /*
       * HACK ALERT
       * Boost timers can be in a state where a handler has not yet executed but is not abortable.
       * As this method needs to mutate state handlers depend on for proper functioning to maintain
       * invariants for other code (namely accepting incoming transactions in a nearly full block)
       * the handlers capture a corelation ID at the time they are set.  When they are executed
       * they must check that correlation_id against the global ordinal.  If it does not match that
       * implies that this method has been called with the handler in the state where it should be
       * cancelled but wasn't able to be.
       */
      uint32_t _timer_corelation_id = 0;

      // path to write the worldstates to
      bfs::path _worldstates_dir;

      void on_irreversible_block( const signed_block_ptr& lib ) {
         _irreversible_block_time = lib->timestamp.to_time_point();
         ilog("refresh _irreversible_block_time = ${_irreversible_block_time}", ("_irreversible_block_time", _irreversible_block_time));
      }

      template<typename Type, typename Channel, typename F>
      auto publish_results_of(const Type &data, Channel& channel, F f) {
         auto publish_success = fc::make_scoped_exit([&, this](){
            channel.publish(std::pair<fc::exception_ptr, Type>(nullptr, data));
         });

         try {
            auto trace = f();
            if (trace->except) {
               publish_success.cancel();
               channel.publish(std::pair<fc::exception_ptr, Type>(trace->except->dynamic_copy_exception(), data));
            }
            return trace;
         } catch (const fc::exception& e) {
            publish_success.cancel();
            channel.publish(std::pair<fc::exception_ptr, Type>(e.dynamic_copy_exception(), data));
            throw e;
         } catch( const std::exception& e ) {
            publish_success.cancel();
            auto fce = fc::exception(
               FC_LOG_MESSAGE( info, "Caught std::exception: ${what}", ("what",e.what())),
               fc::std_exception_code,
               BOOST_CORE_TYPEID(e).name(),
               e.what()
            );
            channel.publish(std::pair<fc::exception_ptr, Type>(fce.dynamic_copy_exception(),data));
            throw fce;
         } catch( ... ) {
            publish_success.cancel();
            auto fce = fc::unhandled_exception(
               FC_LOG_MESSAGE( info, "Caught unknown exception"),
               std::current_exception()
            );

            channel.publish(std::pair<fc::exception_ptr, Type>(fce.dynamic_copy_exception(), data));
            throw fce;
         }
      };

      void on_incoming_block(const signed_block_ptr& block) {
         fc_dlog(_log, "received incoming block ${id}", ("id", block->id()));
      }

      std::vector<std::tuple<packed_transaction_ptr, bool, next_function<transaction_trace_ptr>>> _pending_incoming_transactions;

      void on_incoming_transaction_async(const packed_transaction_ptr& trx, bool from_network, bool persist_until_expired, next_function<transaction_trace_ptr> next) {
          // We do pre-run here only for returning results asap to the transaction caller.
          #define MAKE_TRANSACTION_ACK_TUPLE std::tuple<const fc::exception_ptr, const transaction_trace_ptr, const packed_transaction_ptr>

          chain::controller& chain = app().get_plugin<chain_plugin>().chain();

          static auto last_incoming_trx_report_time = fc::time_point::now();
          static int incoming_trx_count = 0;
          const int report_period = 10;
          const int report_trx_count_thresh = 2000;

          auto time_delta = fc::time_point::now() - last_incoming_trx_report_time;
          incoming_trx_count++;

          // For producer we don't pre-run the trx cause it will run at the fixed timepoint
          // (just before ba0 propose msg)
          if (!_is_non_producing_node) {
              // non-producer does not allow local triggerred trx.
              if (!from_network) {
                  ilog("on_incoming_transaction_async drop local");
                  auto e = std::make_shared<transaction_exception>(
                      FC_LOG_MESSAGE(error, "drop local transaction on producer node"));
                  _transaction_ack_channel.publish(MAKE_TRANSACTION_ACK_TUPLE(e, nullptr, trx));
                  return;
              }

              transaction_metadata_ptr trx_ptr = std::make_shared<transaction_metadata>(*trx);
              auto block_time = chain.head_block_state()->header.timestamp.to_time_point();

              if( fc::time_point(trx_ptr->trx.expiration) < block_time ) {
                  ilog("on_incoming_transaction_async cache trx.id = ${id}, but expired so drop", ("id", trx_ptr->id));
                  auto e = std::make_shared<expired_tx_exception>(FC_LOG_MESSAGE(error, "expired transaction ${id}", ("id", trx_ptr->id)));
                  _transaction_ack_channel.publish(MAKE_TRANSACTION_ACK_TUPLE(e, nullptr, trx));
                  return;
              }

              if (chain.is_known_unexpired_transaction(trx_ptr->id)) {
                  ilog("on_incoming_transaction_async cache trx.id = ${id}, but known so drop", ("id", trx_ptr->id));
                  auto e = std::make_shared<tx_duplicate>(FC_LOG_MESSAGE(error, "duplicate transaction ${id}", ("id", trx_ptr->id)));
                  _transaction_ack_channel.publish(MAKE_TRANSACTION_ACK_TUPLE(e, nullptr, trx));
                  return;
              }

              std::pair<bool, bool> ret = chain.push_into_pending_transaction(trx_ptr);
              //         ilog("on_incoming_transaction_async cache trx.id = ${id}, overflow ${o}, duplicate ${d}",
              //              ("id", trx->id())("o", ret.first)("d", ret.second));;
              if (!ret.first && !ret.second) {
                  _transaction_ack_channel.publish(MAKE_TRANSACTION_ACK_TUPLE(nullptr, nullptr, trx));
              } else {
                  auto e = std::make_shared<tx_duplicate>(
                      FC_LOG_MESSAGE(error, "overflow ${o} or duplicate ${d} transaction ${id}",
                                     ("o", ret.first)("d", ret.second)("id", trx_ptr->id)));
                  _transaction_ack_channel.publish(MAKE_TRANSACTION_ACK_TUPLE(e, nullptr, trx));
              }

              if (time_delta > fc::seconds(report_period) || incoming_trx_count > report_trx_count_thresh) {
                  ilog("on_incoming_transaction_async cache ${count} trxs in last ${delta} seconds",
                       ("count", incoming_trx_count)
                       ("delta", time_delta.to_seconds()));
                  last_incoming_trx_report_time = fc::time_point::now();
                  incoming_trx_count = 0;
              }

              return;
          }

          if (time_delta > fc::seconds(report_period) || incoming_trx_count > report_trx_count_thresh) {
              ilog("on_incoming_transaction_async pre-run ${count} trxs in last ${delta} seconds",
                   ("count", incoming_trx_count)
                   ("delta", time_delta.to_seconds()));
              last_incoming_trx_report_time = fc::time_point::now();
              incoming_trx_count = 0;
          }

          //	 ilog("on_incoming_transaction_async pre-run trx.id = ${id}", ("id", trx->id()));

          if (!chain.pending_block_state()) {
              auto block_timestamp = chain.get_proper_next_block_timestamp();
              ilog("on_transaction: start block at ${time} and block_timestamp is ${timestamp}",
                   ("time", fc::time_point::now())("timestamp", block_timestamp));
              // The committee_mroot here should not matter cause the pending block will
              // be aborted anyway.
              chain.start_block(block_timestamp, chain::checksum256_type(), "");
              // non-producing node does not seem to have a good way to clear dead unapplied trx queue.
              chain.clear_unapplied_transaction();
          }

          auto block_time = chain.pending_block_state()->header.timestamp.to_time_point();

          auto send_response = [this, &trx, &next](const fc::static_variant<fc::exception_ptr, transaction_trace_ptr>& response) {
                                   next(response);
                                   if (response.contains<fc::exception_ptr>()) {
                                       _transaction_ack_channel.publish(MAKE_TRANSACTION_ACK_TUPLE(response.get<fc::exception_ptr>(), nullptr, trx));
                                   } else {
                                       _transaction_ack_channel.publish(MAKE_TRANSACTION_ACK_TUPLE(nullptr, response.get<transaction_trace_ptr>(), trx));
                                   }
                               };

          auto id = trx->id();

          // This is not good in the case of api node + producer node, where api node must pre-run trx for producer node.
          //          if (from_network) {
          //              send_response(std::static_pointer_cast<fc::exception>(std::make_shared<discard_network_trx_for_non_producing_node>(FC_LOG_MESSAGE(error, "network trx discarded for this is non-producing node") )));
          //              return;
          //          }

          if (Node::getInstance() && Node::getInstance()->isSyncing()) {
              send_response(std::static_pointer_cast<fc::exception>(std::make_shared<node_is_syncing>(FC_LOG_MESSAGE(error, "trx discarded, node is in block syncing status") )));
              return;
          }

          if( fc::time_point(trx->expiration()) < block_time ) {
              //ilog("-------pre-run expired trx");
              send_response(std::static_pointer_cast<fc::exception>(std::make_shared<expired_tx_exception>(FC_LOG_MESSAGE(error, "expired transaction ${id}", ("id", id)) )));
              return;
          }

          if( chain.is_known_unexpired_transaction(id) ) {
              //ilog("-------pre-run duplicate trx");
              send_response(std::static_pointer_cast<fc::exception>(std::make_shared<tx_duplicate>(FC_LOG_MESSAGE(error, "duplicate transaction ${id}", ("id", id)) )));
              return;
          }

          auto deadline = fc::time_point::now() + fc::milliseconds(_max_transaction_time_ms);
          // TODO -- yufengshen - setting deadline
          //         bool deadline_is_subjective = false;
          //         if (_max_transaction_time_ms < 0 || (_pending_block_mode == pending_block_mode::producing && block_time < deadline) ) {
          //            deadline_is_subjective = true;
          //            deadline = block_time;
          //         }

          try {
              auto ptrx = std::make_shared<transaction_metadata>(*trx);
              auto trace = chain.push_transaction(ptrx, deadline);
              if (trace->except) {
                  if (failure_is_subjective(*trace->except)) {
                      ilog("---------Failure is subjective, abort block--------");
                      chain.abort_block();
                  }
                  auto e_ptr = trace->except->dynamic_copy_exception();
                  send_response(e_ptr);
              } else {
                  send_response(trace);
              }
          } catch ( boost::interprocess::bad_alloc& ) {
              raise(SIGUSR1);
          } CATCH_AND_CALL(send_response);
      }

      fc::microseconds get_irreversible_block_age() {
         auto now = fc::time_point::now();
         if (now < _irreversible_block_time) {
            return fc::microseconds(0);
         } else {
            return now - _irreversible_block_time;
         }
      }

      bool production_disabled_by_policy() {
         return _pause_production || (_max_irreversible_block_age_us.count() >= 0 && get_irreversible_block_age() >= _max_irreversible_block_age_us);
      }

      enum class start_block_result {
         succeeded,
         failed,
         exhausted
      };
};
producer_rpos_plugin::producer_rpos_plugin()
   : my(new producer_rpos_plugin_impl(app().get_io_service())) {
      my->_self = this;
   }

producer_rpos_plugin::~producer_rpos_plugin() {}

void producer_rpos_plugin::set_program_options(
   boost::program_options::options_description& command_line_options,
   boost::program_options::options_description& config_file_options)
{
   auto default_priv_key = private_key_type::regenerate<fc::ecc::private_key_shim>(fc::sha256::hash(std::string("nathan")));
   auto private_key_default = std::make_pair(default_priv_key.get_public_key(), default_priv_key );

   boost::program_options::options_description producer_options;

   producer_options.add_options()
         ("is-non-producing-node", boost::program_options::bool_switch()->notifier([this](bool e){my->_is_non_producing_node = e;}), "If this is a non-producing node (listener).")
         ("encrypt-config", bpo::value<bool>()->default_value(false), "If the config file is encrpt,default true")
         ("pause-on-startup,x", boost::program_options::bool_switch()->notifier([this](bool p){my->_pause_production = p;}), "Start this node in a state where production is paused")
         ("max-transaction-time", bpo::value<int32_t>()->default_value(30),
          "Limits the maximum time (in milliseconds) that is allowed a pushed transaction's code to execute before being considered invalid")
         ("max-irreversible-block-age", bpo::value<int32_t>()->default_value( -1 ),
          "Limits the maximum age (in seconds) of the DPOS Irreversible Block for a chain this node will produce blocks on (use negative value to indicate unlimited)")
         ("producer-name,p", boost::program_options::value<vector<string>>()->composing()->multitoken(),
          "ID of producer controlled by this node (e.g. inita; may specify multiple times)")
         ("private-key", boost::program_options::value<vector<string>>()->composing()->multitoken(),
          "(DEPRECATED - Use signature-provider instead) Tuple of [public key, WIF private key] (may specify multiple times)")
         ("signature-provider", boost::program_options::value<vector<string>>()->composing()->multitoken()->default_value({std::string(default_priv_key.get_public_key()) + "=KEY:" + std::string(default_priv_key)}, std::string(default_priv_key.get_public_key()) + "=KEY:" + std::string(default_priv_key)),
          "Key=Value pairs in the form <public-key>=<provider-spec>\n"
          "Where:\n"
          "   <public-key>    \tis a string form of a vaild ULTRAINIO public key\n\n"
          "   <provider-spec> \tis a string in the form <provider-type>:<data>\n\n"
          "   <provider-type> \tis KEY, or KULTRAIND\n\n"
          "   KEY:<data>      \tis a string form of a valid ULTRAINIO private key which maps to the provided public key\n\n"
          "   KULTRAIND:<data>    \tis the URL where kultraind is available and the approptiate wallet(s) are unlocked")
         ("kultraind-provider-timeout", boost::program_options::value<int32_t>()->default_value(5),
          "Limits the maximum time (in milliseconds) that is allowd for sending blocks to a kultraind provider for signing")
         ("my-sk-as-committee", boost::program_options::value<std::string>()->notifier([this](std::string g) { my->_my_sk_as_committee = g; }), "sk as committer member")
         ("my-sk-as-account", boost::program_options::value<std::string>()->notifier([this](std::string g) { my->_my_sk_as_account = g; }), "sk as created account")
         ("my-bls-sk", boost::program_options::value<std::string>()->notifier([this](std::string sk) { my->_my_bls_sk = sk; }), "bls sk")
         ("my-account-as-committee", boost::program_options::value<std::string>()->notifier([this](std::string g) { my->_my_account_as_committee = g; }), "account as committer member")
         ("genesis-startup-time", bpo::value<int32_t>()->default_value(Genesis::s_genesisStartupTime), "genesis startup time, set by test mode usually")
         ("genesis-pk", bpo::value<std::string>()->notifier([this](std::string g) { my->_genesis_pk = g; }), "genesis public key, set by test mode usually")
         ("max-round-seconds", bpo::value<int32_t>()->default_value(Config::s_maxRoundSeconds), "max round second, set by test mode usually")
         ("max-phase-seconds", bpo::value<int32_t>()->default_value(Config::s_maxPhaseSeconds), "max phase second, set by test mode usually")
         ("worldstates-dir", bpo::value<bfs::path>()->default_value("worldstate"),"the location of the worldstates directory (absolute path or relative to application data dir)")
         ("max-trxs-microseconds", bpo::value<int32_t>()->default_value(Config::s_maxTrxMicroSeconds), "max trxs microseconds in initpropose,set by test mode usually")
         ("allow-report-evil", bpo::value<bool>()->default_value(false), "whether report evil evidence")
         ;
   config_file_options.add(producer_options);
}

bool producer_rpos_plugin::is_producer_key(const chain::public_key_type& key) const
{
  auto private_key_itr = my->_signature_providers.find(key);
  if(private_key_itr != my->_signature_providers.end())
    return true;
  return false;
}

chain::signature_type producer_rpos_plugin::sign_compact(const chain::public_key_type& key, const fc::sha256& digest) const
{
  if(key != chain::public_key_type()) {
    auto private_key_itr = my->_signature_providers.find(key);
    FC_ASSERT(private_key_itr != my->_signature_providers.end(), "Local producer has no private key in config.ini corresponding to public key ${key}", ("key", key));

    return private_key_itr->second(digest);
  }
  else {
    return chain::signature_type();
  }
}

template<typename T>
T dejsonify(const string& s) {
   return fc::json::from_string(s).as<T>();
}

#define LOAD_VALUE_SET(options, name, container, type) \
if( options.count(name) ) { \
   const std::vector<std::string>& ops = options[name].as<std::vector<std::string>>(); \
   std::copy(ops.begin(), ops.end(), std::inserter(container, container.end())); \
}

static producer_rpos_plugin_impl::signature_provider_type
make_key_signature_provider(const private_key_type& key) {
   return [key]( const chain::digest_type& digest ) {
      return key.sign(digest);
   };
}

static producer_rpos_plugin_impl::signature_provider_type
make_kultraind_signature_provider(const std::shared_ptr<producer_rpos_plugin_impl>& impl, const string& url_str, const public_key_type pubkey) {
   auto kultraind_url = fc::url(url_str);
   std::weak_ptr<producer_rpos_plugin_impl> weak_impl = impl;

   return [weak_impl, kultraind_url, pubkey]( const chain::digest_type& digest ) {
      auto impl = weak_impl.lock();
      if (impl) {
         fc::variant params;
         fc::to_variant(std::make_pair(digest, pubkey), params);
         auto deadline = impl->_kultraind_provider_timeout_us.count() >= 0 ? fc::time_point::now() + impl->_kultraind_provider_timeout_us : fc::time_point::maximum();
         return app().get_plugin<http_client_plugin>().post_sync(kultraind_url, params, deadline).as<chain::signature_type>();
      } else {
         return signature_type();
      }
   };
}

void producer_rpos_plugin::plugin_initialize(const boost::program_options::variables_map& options)
{ try {
   my->_options = &options;
   LOAD_VALUE_SET(options, "producer-name", my->_producers, types::account_name)

   if( options.count("private-key") )
   {
      const std::vector<std::string> key_id_to_wif_pair_strings = options["private-key"].as<std::vector<std::string>>();
      for (const std::string& key_id_to_wif_pair_string : key_id_to_wif_pair_strings)
      {
         try {
            auto key_id_to_wif_pair = dejsonify<std::pair<public_key_type, private_key_type>>(key_id_to_wif_pair_string);
            my->_signature_providers[key_id_to_wif_pair.first] = make_key_signature_provider(key_id_to_wif_pair.second);
            auto blanked_privkey = std::string(std::string(key_id_to_wif_pair.second).size(), '*' );
            wlog("\"private-key\" is DEPRECATED, use \"signature-provider=${pub}=KEY:${priv}\"", ("pub",key_id_to_wif_pair.first)("priv", blanked_privkey));
         } catch ( fc::exception& e ) {
            elog("Malformed private key pair");
         }
      }
   }

   if( options.count("signature-provider") ) {
      const std::vector<std::string> key_spec_pairs = options["signature-provider"].as<std::vector<std::string>>();
      for (const auto& key_spec_pair : key_spec_pairs) {
         try {
            auto delim = key_spec_pair.find("=");
            FC_ASSERT(delim != std::string::npos);
            auto pub_key_str = key_spec_pair.substr(0, delim);
            auto spec_str = key_spec_pair.substr(delim + 1);

            auto spec_delim = spec_str.find(":");
            FC_ASSERT(spec_delim != std::string::npos);
            auto spec_type_str = spec_str.substr(0, spec_delim);
            auto spec_data = spec_str.substr(spec_delim + 1);

            auto pubkey = public_key_type(pub_key_str);

            ilog("spec_type_str = ${spec_type_str}, pubkey = ${pubkey}, spec_data = ${spec_data}", ("spec_type_str", spec_type_str)("pubkey", pubkey)("spec_data", spec_data));
            if (spec_type_str == "KEY") {
               my->_signature_providers[pubkey] = make_key_signature_provider(private_key_type(spec_data));
            } else if (spec_type_str == "KUTRD") {
               my->_signature_providers[pubkey] = make_kultraind_signature_provider(my, spec_data, pubkey);
            }

         } catch (...) {
            elog("Malformed signature provider: \"${val}\", ignoring!", ("val", key_spec_pair));
         }
      }
   }

   my->_kultraind_provider_timeout_us = fc::milliseconds(options.at("kultraind-provider-timeout").as<int32_t>());

   // TODO(yufenshen): temp hack.
   //   my->_max_transaction_time_ms = options.at("max-transaction-time").as<int32_t>();
   // This is only for pre-run deadline. Let's make it loose. But better get it from options.
   my->_max_transaction_time_ms = 500;
   //my->_genesis_delay = options.at("genesis-delay").as<int32_t>();
   my->_genesis_startup_time = options.at("genesis-startup-time").as<int32_t>();
   my->_max_round_seconds = options.at("max-round-seconds").as<int32_t>();
   my->_max_phase_seconds = options.at("max-phase-seconds").as<int32_t>();
   my->_allow_report_evil = options.at("allow-report-evil").as<bool>();
   my->_max_trxs_seconds = options.at("max-trxs-microseconds").as<int32_t>();
   my->_is_config_encrypt = options.at("encrypt-config").as<bool>();
   ultrainio::chain::config::block_interval_ms = my->_max_round_seconds * 1000;
   ultrainio::chain::config::block_interval_us =  my->_max_round_seconds * 1000000;
   if(options.count("genesis-time"))
   {
       my->_genesis_time = options.at("genesis-time").as<std::string>();
   }
   my->_max_irreversible_block_age_us = fc::seconds(options.at("max-irreversible-block-age").as<int32_t>());
   // if( options.count( "worldstates-dir" )) {
   //    auto wd = options.at( "worldstates-dir" ).as<bfs::path>();
   //    if( wd.is_relative()) {
   //       my->_worldstates_dir = app().data_dir() / wd;
   //       if (!fc::exists(my->_worldstates_dir)) {
   //          fc::create_directories(my->_worldstates_dir);
   //       }
   //   } else {
   //       my->_worldstates_dir = wd;
   //   }
   //   FC_ASSERT( fc::is_directory(my->_worldstates_dir) );
   // }

   my->_incoming_block_subscription = app().get_channel<incoming::channels::block>().subscribe([this](const signed_block_ptr& block){
      try {
         my->on_incoming_block(block);
      } FC_LOG_AND_DROP();
   });

   my->_incoming_transaction_subscription = app().get_channel<incoming::channels::transaction>().subscribe([this](const packed_transaction_ptr& trx){
      try {
         my->on_incoming_transaction_async(trx, false, false, [](const auto&){});
      } FC_LOG_AND_DROP();
   });

   my->_incoming_block_sync_provider = app().get_method<incoming::methods::block_sync>().register_provider([this](const signed_block_ptr& block){
      my->on_incoming_block(block);
   });

   my->_incoming_transaction_async_provider = app().get_method<incoming::methods::transaction_async>().register_provider([this](const packed_transaction_ptr& trx, bool from_network, bool persist_until_expired, next_function<transaction_trace_ptr> next) -> void {
      return my->on_incoming_transaction_async(trx, from_network, persist_until_expired, next );
   });

} FC_LOG_AND_RETHROW() }
int  producer_rpos_plugin::get_round_interval()
{
	    return  my->_max_round_seconds;
}
string producer_rpos_plugin::get_account_sk()
{
    return std::string(NodeInfo::getMainAccountTrxPriKey());
}
string producer_rpos_plugin::get_account_name()
{
    return std::string(NodeInfo::getMainAccount());
}
bool producer_rpos_plugin::handle_message(const EchoMsg& echo) {
   return Node::getInstance()->handleMessage(echo);
}

bool producer_rpos_plugin::handle_message(const ProposeMsg& propose) {
   return Node::getInstance()->handleMessage(propose);
}

bool producer_rpos_plugin::handle_message(const fc::sha256& node_id, const ReqSyncMsg& msg) {
   return Node::getInstance()->handleMessage(node_id, msg);
}

bool producer_rpos_plugin::handle_message(const SyncBlockMsg& msg, bool last_block, bool safe) {
   return Node::getInstance()->handleMessage(msg, last_block, safe);
}

bool producer_rpos_plugin::handle_message(const fc::sha256& node_id, const ReqBlockNumRangeMsg& msg) {
  return Node::getInstance()->handleMessage(node_id, msg);
}

bool producer_rpos_plugin::handle_message(const fc::sha256& node_id, const SyncStopMsg& msg) {
  return Node::getInstance()->handleMessage(node_id, msg);
}

bool producer_rpos_plugin::sync_fail(const ultrainio::ReqSyncMsg& sync_msg) {
  return Node::getInstance()->syncFail(sync_msg);
}

bool producer_rpos_plugin::sync_cancel() {
  return Node::getInstance()->syncCancel();
}

void producer_rpos_plugin::plugin_startup()
{ try {
   if(fc::get_logger_map().find(logger_name) != fc::get_logger_map().end()) {
      _log = fc::get_logger_map()[logger_name];
   }

   ilog("producer plugin:  plugin_startup() begin");

   std::shared_ptr<Node> nodePtr = Node::initAndGetInstance(app().get_io_service());
   // set before committee key
   nodePtr->setNonProducingNode(my->_is_non_producing_node);
   if (!my->_genesis_pk.empty()) {
       nodePtr->setGenesisPk(my->_genesis_pk);
   }
   if(my->_is_config_encrypt){
       std::string key("ultrain@#9Df2Y*c");
       std::string result = aes_decode(key, my->_my_sk_as_committee);
       my->_my_sk_as_committee = std::move(result);

       result = aes_decode(key, my->_my_sk_as_account);
       my->_my_sk_as_account = std::move(result);

       result = aes_decode(key, my->_my_bls_sk);
       my->_my_bls_sk = std::move(result);
   }
   std::vector<std::string> committeeV;
   StringUtils::tokenlize(my->_my_account_as_committee, ',', committeeV);
   std::vector<std::string> committeeSkV;
   StringUtils::tokenlize(my->_my_sk_as_committee, ',', committeeSkV);
   std::vector<std::string> blsSkV;
   StringUtils::tokenlize(my->_my_bls_sk, ',', blsSkV);
   std::vector<std::string> trxSkV;
   StringUtils::tokenlize(my->_my_sk_as_account, ',', trxSkV);
   NodeInfo::getInstance()->setCommitteeInfo(committeeV, committeeSkV, blsSkV, trxSkV);
   ULTRAIN_ASSERT( !my->_genesis_time.empty(),
           plugin_config_exception,
           "Genesis-time can not be empty,should be set in config.ini.");
   if (!my->_genesis_time.empty()) {
       fc::time_point tp = fc::time_point::from_iso_string(my->_genesis_time.data());
       nodePtr->setGenesisTime(tp);
   }
   nodePtr->setGenesisStartupTime(my->_genesis_startup_time);
   nodePtr->setRoundAndPhaseSecond(my->_max_round_seconds, my->_max_phase_seconds);
   nodePtr->setTrxsSecond(my->_max_trxs_seconds);
   nodePtr->setAllowReportEvil(my->_allow_report_evil);
   nodePtr->init();
   nodePtr->readyToJoin();
   ilog("producer plugin:  plugin_startup() end");
} FC_CAPTURE_AND_RETHROW() }

void producer_rpos_plugin::plugin_shutdown() {
   my->_accepted_block_connection.reset();
   my->_irreversible_block_connection.reset();
}

void producer_rpos_plugin::pause() {
   my->_pause_production = true;
}

void producer_rpos_plugin::resume() {
   ilog("producer_rpos_plugin::resume");
}

bool producer_rpos_plugin::paused() const {
   return my->_pause_production;
}

void producer_rpos_plugin::update_runtime_options(const runtime_options& options) {
   bool check_speculating = false;

   if (options.max_transaction_time) {
      my->_max_transaction_time_ms = *options.max_transaction_time;
   }

   if (options.max_irreversible_block_age) {
      my->_max_irreversible_block_age_us =  fc::seconds(*options.max_irreversible_block_age);
      check_speculating = true;
   }
}

producer_rpos_plugin::runtime_options producer_rpos_plugin::get_runtime_options() const {
   return {
      my->_max_transaction_time_ms,
      my->_max_irreversible_block_age_us.count() < 0 ? -1 : my->_max_irreversible_block_age_us.count() / 1'000'000
   };
}

} // namespace ultrainio
