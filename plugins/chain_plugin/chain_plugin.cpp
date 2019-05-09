/**
 *  @file
 *  @copyright defined in ultrain/LICENSE.txt
 */
#include <ultrainio/chain_plugin/chain_plugin.hpp>
#include <ultrainio/chain/fork_database.hpp>
#include <ultrainio/chain/block_log.hpp>
#include <ultrainio/chain/exceptions.hpp>
#include <ultrainio/chain/authorization_manager.hpp>
#include <ultrainio/chain/producer_object.hpp>
#include <ultrainio/chain/config.hpp>
#include <ultrainio/chain/types.hpp>
#include <ultrainio/chain/wasm_interface.hpp>
#include <ultrainio/chain/resource_limits.hpp>
#include <ultrainio/chain/controller.hpp>
#include <ultrainio/chain/generated_transaction_object.hpp>
//#include <ultrainio/chain/ultrainio_object.hpp>

#include <ultrainio/chain/ultrainio_contract.hpp>

#include <ultrainio/utilities/key_conversion.hpp>
#include <ultrainio/utilities/common.hpp>
#include <ultrainio/chain/wast_to_wasm.hpp>
#include <ultrainio/chain/worldstate_file_manager.hpp>
#include <ultrainio/chain/worldstate.hpp>
#include <ultrainio/chain/callback.hpp>
#include <ultrainio/chain/callback_manager.hpp>

#include <boost/signals2/connection.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include <fc/io/json.hpp>
#include <fc/variant.hpp>
#include <signal.h>

#include <core/BlsVoterSet.h>
#include <core/types.h>
#include <lightclient/CommitteeSet.h>
#include <lightclient/EpochEndPoint.h>
#include <lightclient/LightClient.h>
#include <lightclient/LightClientMgr.h>
#include <lightclient/LightClientProducer.h>
#include <lightclient/StartPoint.h>

namespace ultrainio {

//declare operator<< and validate funciton for read_mode in the same namespace as read_mode itself
namespace chain {

std::ostream& operator<<(std::ostream& osm, ultrainio::chain::db_read_mode m) {
   if ( m == ultrainio::chain::db_read_mode::SPECULATIVE ) {
      osm << "speculative";
   } else if ( m == ultrainio::chain::db_read_mode::HEAD ) {
      osm << "head";
   } else if ( m == ultrainio::chain::db_read_mode::IRREVERSIBLE ) {
      osm << "irreversible";
   }

   return osm;
}

void validate(boost::any& v,
              std::vector<std::string> const& values,
              ultrainio::chain::db_read_mode* /* target_type */,
              int)
{
  using namespace boost::program_options;

  // Make sure no previous assignment to 'v' was made.
  validators::check_first_occurrence(v);

  // Extract the first string from 'values'. If there is more than
  // one string, it's an error, and exception will be thrown.
  std::string const& s = validators::get_single_string(values);

  if ( s == "speculative" ) {
     v = boost::any(ultrainio::chain::db_read_mode::SPECULATIVE);
  } else if ( s == "head" ) {
     v = boost::any(ultrainio::chain::db_read_mode::HEAD);
  } else if ( s == "irreversible" ) {
     v = boost::any(ultrainio::chain::db_read_mode::IRREVERSIBLE);
  } else {
     throw validation_error(validation_error::invalid_option_value);
  }
}

}

using namespace ultrainio;
using namespace ultrainio::chain;
using namespace ultrainio::chain::config;
using namespace ultrainio::chain::plugin_interface;
using vm_type = wasm_interface::vm_type;
using fc::flat_map;

using boost::signals2::scoped_connection;

//using txn_msg_rate_limits = controller::txn_msg_rate_limits;

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

using ultrainio::LightClient;
using ultrainio::CommitteeSet;
using ultrainio::LightClientMgr;

class light_client_callback : public ultrainio::chain::callback {
public:
    light_client_callback() {
    }

    bool on_accept_block_header(uint64_t chainName, const chain::signed_block_header &blockHeader, BlockIdType &id) {
        ilog("on_accept_block_header chain : ${chainName}, blockNum : ${blockNum}",
             ("chainName", name(chainName))("blockNum", blockHeader.block_num()));
        std::shared_ptr<LightClient> lightClient = LightClientMgr::getInstance()->getLightClient(chainName);
        if (std::string(blockHeader.proposer) == std::string("genesis")) {
            lightClient->accept(blockHeader, blockHeader.signature);
            id = lightClient->getLatestConfirmedBlockId();
            return lightClient->getStatus();
        } else {
            std::vector<signed_block_header> unconfirmedheaders;
            StartPoint startPoint;
            if (getUnconfirmedHeaderFromDb(name(chainName), unconfirmedheaders, startPoint)) {
                lightClient->setStartPoint(startPoint);
                for (auto e : unconfirmedheaders) {
                    lightClient->accept(e, e.signature);
                }
                lightClient->accept(blockHeader, blockHeader.signature);
                id = lightClient->getLatestConfirmedBlockId();
                return lightClient->getStatus();
            }
        }
        id = lightClient->getLatestConfirmedBlockId();
        return lightClient->getStatus();
    }

    bool on_replay_block(const chain::block_header& header) {
        if (!m_lightClientProducer) {
            chain::controller& chain = appbase::app().get_plugin<chain_plugin>().chain();
            m_lightClientProducer = std::make_shared<LightClientProducer>(chain.get_bls_votes_manager());
        }
        m_lightClientProducer->acceptNewHeader(header);
        const auto &ro_api = appbase::app().get_plugin<chain_plugin>().get_read_only_api();
        chain_apis::read_only::get_confirm_point_interval_result result = ro_api.get_confirm_point_interval(chain_apis::read_only::get_confirm_point_interval_params());
        LightClientProducer::setConfirmPointInterval(result.confirm_point_interval);
        return true;
    }

private:
    bool getUnconfirmedHeaderFromDb(const chain::name &chainName, std::vector<signed_block_header> &unconfirmedBlockHeader, StartPoint& startPoint) {
        try {
            const auto &ro_api = appbase::app().get_plugin<chain_plugin>().get_read_only_api();
            struct chain_apis::read_only::get_subchain_unconfirmed_header_params params;
            params.chain_name = chainName;
            auto result = ro_api.get_subchain_unconfirmed_header(params);
            unconfirmedBlockHeader = result.unconfirmed_headers;
            startPoint.lastConfirmedBlockId = result.confirmed_block_id;
            startPoint.committeeSet = CommitteeSet(result.committee_set);
            startPoint.nextCommitteeMroot = result.next_committee_mroot;
            //ilog("chainName name = ${name} committee : ${committee} size : ${size}", ("name", chainName.to_string())("committee", committeeSet.toString())("size", result.committee_set.size()));
            return true;
        } catch (fc::exception &e) {
            ilog("There may be no unconfirmed block header : ${e}", ("e", e.to_string()));
        }
        return false;
    };

    std::shared_ptr<LightClientProducer> m_lightClientProducer;
};

class chain_plugin_impl {
public:
   chain_plugin_impl()
   :pre_accepted_block_channel(app().get_channel<channels::pre_accepted_block>())
   ,accepted_block_header_channel(app().get_channel<channels::accepted_block_header>())
   ,accepted_block_channel(app().get_channel<channels::accepted_block>())
   ,irreversible_block_channel(app().get_channel<channels::irreversible_block>())
   ,accepted_transaction_channel(app().get_channel<channels::accepted_transaction>())
   ,applied_transaction_channel(app().get_channel<channels::applied_transaction>())
   ,incoming_block_channel(app().get_channel<incoming::channels::block>())
   ,incoming_block_sync_method(app().get_method<incoming::methods::block_sync>())
   ,incoming_transaction_async_method(app().get_method<incoming::methods::transaction_async>())
   {}

   bfs::path                        blocks_dir;
   bool                             readonly = false;
   flat_map<uint32_t,block_id_type> loaded_checkpoints;

   fc::optional<fork_database>      fork_db;
   fc::optional<block_log>          block_logger;
   fc::optional<controller::config> chain_config;
   fc::optional<controller>         chain;
   fc::optional<chain_id_type>      chain_id;
   //txn_msg_rate_limits              rate_limits;
   fc::optional<vm_type>            wasm_runtime;
   fc::microseconds                 abi_serializer_max_time_ms;
   fc::optional<bfs::path>          worldstate_path;
   std::string _genesis_time = std::string();

   // retained references to channels for easy publication
   channels::pre_accepted_block::channel_type&     pre_accepted_block_channel;
   channels::accepted_block_header::channel_type&  accepted_block_header_channel;
   channels::accepted_block::channel_type&         accepted_block_channel;
   channels::irreversible_block::channel_type&     irreversible_block_channel;
   channels::accepted_transaction::channel_type&   accepted_transaction_channel;
   channels::applied_transaction::channel_type&    applied_transaction_channel;
   incoming::channels::block::channel_type&        incoming_block_channel;

   // retained references to methods for easy calling
   incoming::methods::block_sync::method_type&        incoming_block_sync_method;
   incoming::methods::transaction_async::method_type& incoming_transaction_async_method;

   // method provider handles
   methods::get_block_by_number::method_type::handle                 get_block_by_number_provider;
   methods::get_block_by_id::method_type::handle                     get_block_by_id_provider;
   methods::get_head_block_id::method_type::handle                   get_head_block_id_provider;
   methods::get_last_irreversible_block_number::method_type::handle  get_last_irreversible_block_number_provider;

   // scoped connections for chain controller
   fc::optional<scoped_connection>                                   pre_accepted_block_connection;
   fc::optional<scoped_connection>                                   accepted_block_header_connection;
   fc::optional<scoped_connection>                                   accepted_block_connection;
   fc::optional<scoped_connection>                                   irreversible_block_connection;
   fc::optional<scoped_connection>                                   accepted_transaction_connection;
   fc::optional<scoped_connection>                                   applied_transaction_connection;
   fc::optional<scoped_connection>                                   accepted_confirmation_connection;


};

chain_plugin::chain_plugin()
:my(new chain_plugin_impl()) {
}

chain_plugin::~chain_plugin(){}

void chain_plugin::set_program_options(options_description& cli, options_description& cfg)
{
   cfg.add_options()
         ("blocks-dir", bpo::value<bfs::path>()->default_value("blocks"),
          "the location of the blocks directory (absolute path or relative to application data dir)")
         ("worldstate", bpo::value<bfs::path>(), "File to read Worldstate State from")
         ("worldstate-control", bpo::bool_switch()->default_value(false), "Enable worldstate generation")
         ("checkpoint", bpo::value<vector<string>>()->composing(), "Pairs of [BLOCK_NUM,BLOCK_ID] that should be enforced as checkpoints.")
         ("wasm-runtime", bpo::value<ultrainio::chain::wasm_interface::vm_type>()->value_name("wavm/binaryen"), "Override default WASM runtime")
         ("abi-serializer-max-time-ms", bpo::value<uint32_t>()->default_value(config::default_abi_serializer_max_time_ms),
          "Override default maximum ABI serialization time allowed in ms")
         ("chain-state-db-size-mb", bpo::value<uint64_t>()->default_value(config::default_state_size / (1024  * 1024)), "Maximum size (in MiB) of the chain state database")
         ("chain-state-db-guard-size-mb", bpo::value<uint64_t>()->default_value(config::default_state_guard_size / (1024  * 1024)), "Safely shut down node when free space remaining in the chain state database drops below this size (in MiB).")
         ("contracts-console", bpo::bool_switch()->default_value(false),
          "print contract's output to console")
         ("masterchain", bpo::bool_switch()->default_value(false),
          "if the chain is running as main chain")
         ("actor-whitelist", boost::program_options::value<vector<string>>()->composing()->multitoken(),
          "Account added to actor whitelist (may specify multiple times)")
         ("actor-blacklist", boost::program_options::value<vector<string>>()->composing()->multitoken(),
          "Account added to actor blacklist (may specify multiple times)")
         ("contract-whitelist", boost::program_options::value<vector<string>>()->composing()->multitoken(),
          "Contract account added to contract whitelist (may specify multiple times)")
         ("contract-blacklist", boost::program_options::value<vector<string>>()->composing()->multitoken(),
          "Contract account added to contract blacklist (may specify multiple times)")
         ("action-blacklist", boost::program_options::value<vector<string>>()->composing()->multitoken(),
          "Action (in the form code::action) added to action blacklist (may specify multiple times)")
         ("key-blacklist", boost::program_options::value<vector<string>>()->composing()->multitoken(),
          "Public key added to blacklist of keys that should not be included in authorities (may specify multiple times)")
         ("read-mode", boost::program_options::value<ultrainio::chain::db_read_mode>()->default_value(ultrainio::chain::db_read_mode::SPECULATIVE),
          "Database read mode (\"speculative\" or \"head\").\n"// or \"irreversible\").\n"
          "In \"speculative\" mode database contains changes done up to the head block plus changes made by transactions not yet included to the blockchain.\n"
          "In \"head\" mode database contains changes done up to the current head block.\n")
          //"In \"irreversible\" mode database contains changes done up the current irreversible block.\n")
          #ifdef ULTRAIN_CONFIG_CONTRACT_PARAMS
         ("contract-return-string-length", bpo::value<uint64_t>()->default_value(config::default_contract_return_length),
          "Contract return string length limits the string length of Returns")
         ("contract-emit-string-length", bpo::value<uint64_t>()->default_value(config::default_contract_emit_length),
          "Contract emit string length limits the string length of serialized EventObject.")
          #endif
         ("max_block_cpu_usage", bpo::value<uint32_t>()->default_value(config::default_max_block_cpu_usage),
           "max_block_cpu_usage,used in resource ,in genesis param,etc")
         ("max_block_net_usage", bpo::value<uint32_t>()->default_value(config::default_max_block_net_usage),
                "max_block_net_usage,used in resource ,in genesis param,etc")
    	 ("genesis-time",bpo::value<string>(), "override the initial timestamp in the Genesis State file")
         ;

// TODO: rate limiting
         /*("per-authorized-account-transaction-msg-rate-limit-time-frame-sec", bpo::value<uint32_t>()->default_value(default_per_auth_account_time_frame_seconds),
          "The time frame, in seconds, that the per-authorized-account-transaction-msg-rate-limit is imposed over.")
         ("per-authorized-account-transaction-msg-rate-limit", bpo::value<uint32_t>()->default_value(default_per_auth_account),
          "Limits the maximum rate of transaction messages that an account is allowed each per-authorized-account-transaction-msg-rate-limit-time-frame-sec.")
          ("per-code-account-transaction-msg-rate-limit-time-frame-sec", bpo::value<uint32_t>()->default_value(default_per_code_account_time_frame_seconds),
           "The time frame, in seconds, that the per-code-account-transaction-msg-rate-limit is imposed over.")
          ("per-code-account-transaction-msg-rate-limit", bpo::value<uint32_t>()->default_value(default_per_code_account),
           "Limits the maximum rate of transaction messages that an account's code is allowed each per-code-account-transaction-msg-rate-limit-time-frame-sec.")*/

   cli.add_options()
         ("genesis-json", bpo::value<bfs::path>(), "File to read Genesis State from")
         ("genesis-timestamp", bpo::value<string>(), "override the initial timestamp in the Genesis State file")
         ("print-genesis-json", bpo::bool_switch()->default_value(false),
          "extract genesis_state from blocks.log as JSON, print to console, and exit")
         ("extract-genesis-json", bpo::value<bfs::path>(),
          "extract genesis_state from blocks.log as JSON, write into specified file, and exit")
         ("force-all-checks", bpo::bool_switch()->default_value(false),
          "do not skip any checks that can be skipped while replaying irreversible blocks")
         ("replay-blockchain", bpo::bool_switch()->default_value(false),
          "clear chain state database and replay all blocks")
         ("hard-replay-blockchain", bpo::bool_switch()->default_value(false),
          "clear chain state database, recover as many blocks as possible from the block log, and then replay those blocks")
         ("delete-all-blocks", bpo::bool_switch()->default_value(false),
          "clear chain state database and block log")
         ("truncate-at-block", bpo::value<uint32_t>()->default_value(0),
          "stop hard replay / block log recovery at this block number (if set to non-zero number)")
		;

}

#define LOAD_VALUE_SET(options, name, container) \
if( options.count(name) ) { \
   const std::vector<std::string>& ops = options[name].as<std::vector<std::string>>(); \
   std::copy(ops.begin(), ops.end(), std::inserter(container, container.end())); \
}

fc::time_point calculate_genesis_timestamp( string tstr ) {
   fc::time_point genesis_timestamp;
   if( strcasecmp (tstr.c_str(), "now") == 0 ) {
      genesis_timestamp = fc::time_point::now();
   } else {
      genesis_timestamp = time_point::from_iso_string( tstr );
   }

   auto epoch_us = genesis_timestamp.time_since_epoch().count();
   auto diff_us = epoch_us % config::block_interval_us;
   if (diff_us > 0) {
      auto delay_us = (config::block_interval_us - diff_us);
      genesis_timestamp += fc::microseconds(delay_us);
      dlog("pausing ${us} microseconds to the next interval",("us",delay_us));
   }

   ilog( "Adjusting genesis timestamp to ${timestamp}", ("timestamp", genesis_timestamp) );
   return genesis_timestamp;
}

void chain_plugin::plugin_initialize(const variables_map& options) {
   ilog("initializing chain plugin");

   ultrainio::chain::callback_manager::get_self()->register_callback(std::make_shared<light_client_callback>());
   try {
      try {
         genesis_state gs; // Check if ULTRAINIO_ROOT_KEY is bad
      } catch ( const fc::exception& ) {
         elog( "ULTRAINIO_ROOT_KEY ('${root_key}') is invalid. Recompile with a valid public key.",
               ("root_key", genesis_state::ultrainio_root_key));
         throw;
      }
     ultrainio::chain::config::default_max_block_cpu_usage = options.at("max_block_cpu_usage").as<uint32_t>();
     ultrainio::chain::config::default_max_transaction_cpu_usage = ultrainio::chain::config::default_max_block_cpu_usage / 2;
     ultrainio::chain::config::default_max_block_net_usage = options.at("max_block_net_usage").as<uint32_t>();
     ultrainio::chain::config::default_max_transaction_net_usage = ultrainio::chain::config::default_max_block_net_usage / 2;
     ilog("default ${default_max_block_cpu_usage}",("default_max_block_cpu_usage",ultrainio::chain::config::default_max_block_cpu_usage));
      my->chain_config = controller::config();
      if(options.count("genesis-time"))
      {
          my->_genesis_time = options.at( "genesis-time" ).as<string>();
      }
      ULTRAIN_ASSERT( !my->_genesis_time.empty(),
              plugin_config_exception,
              "Genesis-time can not be empty,should be set in config.ini.");
      fc::time_point genesis_timestamp =  calculate_genesis_timestamp(my->_genesis_time);
      LOAD_VALUE_SET( options, "actor-whitelist", my->chain_config->actor_whitelist );
      LOAD_VALUE_SET( options, "actor-blacklist", my->chain_config->actor_blacklist );
      LOAD_VALUE_SET( options, "contract-whitelist", my->chain_config->contract_whitelist );
      LOAD_VALUE_SET( options, "contract-blacklist", my->chain_config->contract_blacklist );

      #ifdef ULTRAIN_CONFIG_CONTRACT_PARAMS
      if( options.count( "contract-return-string-length" ))
         my->chain_config->contract_return_length = options.at( "contract-return-string-length" ).as<uint64_t>();

      if( options.count( "contract-emit-string-length" ))
         my->chain_config->contract_emit_length = options.at( "contract-emit-string-length" ).as<uint64_t>();
      #endif

      if( options.count( "action-blacklist" )) {
         const std::vector<std::string>& acts = options["action-blacklist"].as<std::vector<std::string>>();
         auto& list = my->chain_config->action_blacklist;
         for( const auto& a : acts ) {
            auto pos = a.find( "::" );
            ULTRAIN_ASSERT( pos != std::string::npos, plugin_config_exception, "Invalid entry in action-blacklist: '${a}'", ("a", a));
            account_name code( a.substr( 0, pos ));
            action_name act( a.substr( pos + 2 ));
            list.emplace( code.value, act );
         }
      }

      if( options.count( "key-blacklist" )) {
         const std::vector<std::string>& keys = options["key-blacklist"].as<std::vector<std::string>>();
         auto& list = my->chain_config->key_blacklist;
         for( const auto& key_str : keys ) {
            list.emplace( key_str );
         }
      }

      if( options.count( "blocks-dir" )) {
         auto bld = options.at( "blocks-dir" ).as<bfs::path>();
         if( bld.is_relative())
            my->blocks_dir = app().data_dir() / bld;
         else
            my->blocks_dir = bld;
      }

      if( options.count("checkpoint") ) {
         auto cps = options.at("checkpoint").as<vector<string>>();
         my->loaded_checkpoints.reserve(cps.size());
         for( const auto& cp : cps ) {
            auto item = fc::json::from_string(cp).as<std::pair<uint32_t,block_id_type>>();
            auto itr = my->loaded_checkpoints.find(item.first);
            if( itr != my->loaded_checkpoints.end() ) {
               ULTRAIN_ASSERT( itr->second == item.second,
                           plugin_config_exception,
                          "redefining existing checkpoint at block number ${num}: original: ${orig} new: ${new}",
                          ("num", item.first)("orig", itr->second)("new", item.second)
               );
            } else {
               my->loaded_checkpoints[item.first] = item.second;
            }
         }
      }

      if( options.count( "wasm-runtime" ))
         my->wasm_runtime = options.at( "wasm-runtime" ).as<vm_type>();

      if(options.count("abi-serializer-max-time-ms"))
         my->abi_serializer_max_time_ms = fc::microseconds(options.at("abi-serializer-max-time-ms").as<uint32_t>() * 1000);

      my->chain_config->blocks_dir = my->blocks_dir;
      my->chain_config->state_dir = app().data_dir() / config::default_state_dir_name;
      my->chain_config->read_only = my->readonly;

      if( options.count( "chain-state-db-size-mb" ))
         my->chain_config->state_size = options.at( "chain-state-db-size-mb" ).as<uint64_t>() * 1024 * 1024;

      if( options.count( "chain-state-db-guard-size-mb" ))
         my->chain_config->state_guard_size = options.at( "chain-state-db-guard-size-mb" ).as<uint64_t>() * 1024 * 1024;

      if( my->wasm_runtime )
         my->chain_config->wasm_runtime = *my->wasm_runtime;

      my->chain_config->worldstate_control = options.at( "worldstate-control" ).as<bool>();
      my->chain_config->force_all_checks = options.at( "force-all-checks" ).as<bool>();
      my->chain_config->contracts_console = options.at( "contracts-console" ).as<bool>();

      if( options.count( "extract-genesis-json" ) || options.at( "print-genesis-json" ).as<bool>()) {
         genesis_state gs;

         if( fc::exists( my->blocks_dir / "blocks.log" )) {
            gs = block_log::extract_genesis_state( my->blocks_dir );
         } else {
            wlog( "No blocks.log found at '${p}'. Using default genesis state.",
                  ("p", (my->blocks_dir / "blocks.log").generic_string()));
         }

         if( options.at( "print-genesis-json" ).as<bool>()) {
            ilog( "Genesis JSON:\n${genesis}", ("genesis", json::to_pretty_string( gs )));
         }

         if( options.count( "extract-genesis-json" )) {
            auto p = options.at( "extract-genesis-json" ).as<bfs::path>();

            if( p.is_relative()) {
               p = bfs::current_path() / p;
            }

            fc::json::save_to_file( gs, p, true );
            ilog( "Saved genesis JSON to '${path}'", ("path", p.generic_string()));
         }

         ULTRAIN_THROW( extract_genesis_state_exception, "extracted genesis state from blocks.log" );
      }

      if( options.at( "delete-all-blocks" ).as<bool>()) {
         ilog( "Deleting state database and blocks" );
         if( options.at( "truncate-at-block" ).as<uint32_t>() > 0 )
            wlog( "The --truncate-at-block option does not make sense when deleting all blocks." );
         fc::remove_all( my->chain_config->state_dir );
         fc::remove_all( my->blocks_dir );
      } else if( options.at( "hard-replay-blockchain" ).as<bool>()) {
         ilog( "Hard replay requested: deleting state database" );
         fc::remove_all( my->chain_config->state_dir );
         auto backup_dir = block_log::repair_log( my->blocks_dir, options.at( "truncate-at-block" ).as<uint32_t>());
      } else if( options.at( "replay-blockchain" ).as<bool>()) {
         ilog( "Replay requested: deleting state database" );
         if( options.at( "truncate-at-block" ).as<uint32_t>() > 0 )
            wlog( "The --truncate-at-block option does not work for a regular replay of the blockchain." );
         fc::remove_all( my->chain_config->state_dir );
      } else if( options.at( "truncate-at-block" ).as<uint32_t>() > 0 ) {
         wlog( "The --truncate-at-block option can only be used with --fix-reversible-blocks without a replay or with --hard-replay-blockchain." );
      }

      if (options.count( "worldstate" )) {
         my->worldstate_path = options.at( "worldstate" ).as<bfs::path>();
         ULTRAIN_ASSERT( fc::exists(*my->worldstate_path), plugin_config_exception,
                     "Cannot load worldstate, ${name} does not exist", ("name", my->worldstate_path->generic_string()) );

         // recover genesis information from the worldstate
         auto infile = std::ifstream(my->worldstate_path->generic_string(), (std::ios::in | std::ios::binary));
         auto reader = std::make_shared<istream_worldstate_reader>(infile);
         reader->validate();
         reader->read_section<genesis_state>([this]( auto &section ){
            section.read_row(my->chain_config->genesis);
         });
         infile.close();

         ULTRAIN_ASSERT( options.count( "genesis-json" ) == 0 &&  options.count( "genesis-timestamp" ) == 0,
                 plugin_config_exception,
                 "--worldstate is incompatible with --genesis-json and --genesis-timestamp as the worldstate contains genesis information");

         auto shared_mem_path = my->chain_config->state_dir / "shared_memory.bin";
         ULTRAIN_ASSERT( !fc::exists(shared_mem_path),
                 plugin_config_exception,
                 "worldstate can only be used to initialize an empty database." );

         if( fc::is_regular_file( my->blocks_dir / "blocks.log" )) {
            auto log_genesis = block_log::extract_genesis_state(my->blocks_dir);
            ULTRAIN_ASSERT( log_genesis.compute_chain_id() == my->chain_config->genesis.compute_chain_id(),
                    plugin_config_exception,
                    "Genesis information in blocks.log does not match genesis information in the worldstate");
         }

      } else
      {
         if( options.count( "genesis-json" )) {
            ULTRAIN_ASSERT( !fc::exists( my->blocks_dir / "blocks.log" ),
                        plugin_config_exception,
                     "Genesis state can only be set on a fresh blockchain." );

            auto genesis_file = options.at( "genesis-json" ).as<bfs::path>();
            if( genesis_file.is_relative()) {
               genesis_file = bfs::current_path() / genesis_file;
            }

            ULTRAIN_ASSERT( fc::is_regular_file( genesis_file ),
                        plugin_config_exception,
                     "Specified genesis file '${genesis}' does not exist.",
                     ("genesis", genesis_file.generic_string()));

            my->chain_config->genesis = fc::json::from_file( genesis_file ).as<genesis_state>();

            ilog( "Using genesis state provided in '${genesis}'", ("genesis", genesis_file.generic_string()));

            if( options.count( "genesis-timestamp" )) {
               my->chain_config->genesis.initial_timestamp = calculate_genesis_timestamp(
                     options.at( "genesis-timestamp" ).as<string>());
            }

            wlog( "Starting up fresh blockchain with provided genesis state." );
         } else if( options.count( "genesis-timestamp" )) {
             ULTRAIN_ASSERT( !fc::exists( my->blocks_dir / "blocks.log" ),
                     plugin_config_exception,
                     "Genesis state can only be set on a fresh blockchain." );

             my->chain_config->genesis.initial_timestamp = calculate_genesis_timestamp(
                     options.at( "genesis-timestamp" ).as<string>());

             wlog( "Starting up fresh blockchain with default genesis state but with adjusted genesis timestamp." );
         } else if( fc::is_regular_file( my->blocks_dir / "blocks.log" )) {
             my->chain_config->genesis = block_log::extract_genesis_state( my->blocks_dir );
             ULTRAIN_ASSERT( genesis_timestamp == my->chain_config->genesis.initial_timestamp,
                     plugin_config_exception,
                     "Genesis timestamp in data diff with which in config,ini" );
         } else {
             wlog( "Starting up fresh blockchain with default genesis state." );
             my->chain_config->genesis.initial_timestamp = genesis_timestamp;
         }
      }
      my->chain_config->genesis.initial_phase = options.at("max-phase-seconds").as<int32_t>();
      my->chain_config->genesis.initial_round = options.at("max-round-seconds").as<int32_t>();
      my->chain_config->genesis.initial_syncing_source_timeout = options.at( "max-waitblocknum-seconds" ).as<int>();
      my->chain_config->genesis.initial_syncing_block_timeout = options.at( "max-waitblock-seconds" ).as<int>();
      my->chain_config->genesis.initial_max_trxs_time = options.at("max-trxs-microseconds").as<int32_t>();
      ilog("genesis of chain in config: time ${genesis} phase ${initial_phase} round ${initial_round} syncing${source_timeout} ${block_timeout} trx ${trxs}",
              ("genesis",my->_genesis_time)
              ("initial_phase",my->chain_config->genesis.initial_phase)
              ("initial_round",my->chain_config->genesis.initial_round)
              ("source_timeout",my->chain_config->genesis.initial_syncing_source_timeout)
              ("block_timeout",my->chain_config->genesis.initial_syncing_block_timeout)
              ("trxs",my->chain_config->genesis.initial_max_trxs_time));
      if ( options.count("read-mode") ) {
         my->chain_config->read_mode = options.at("read-mode").as<db_read_mode>();
         ULTRAIN_ASSERT( my->chain_config->read_mode != db_read_mode::IRREVERSIBLE, plugin_config_exception, "irreversible mode not currently supported." );
      }

      my->chain.emplace( *my->chain_config );
      my->chain_id.emplace( my->chain->get_chain_id());

      // set up method providers
      my->get_block_by_number_provider = app().get_method<methods::get_block_by_number>().register_provider(
            [this]( uint32_t block_num ) -> signed_block_ptr {
               return my->chain->fetch_block_by_number( block_num );
            } );

      my->get_block_by_id_provider = app().get_method<methods::get_block_by_id>().register_provider(
            [this]( block_id_type id ) -> signed_block_ptr {
               return my->chain->fetch_block_by_id( id );
            } );

      my->get_head_block_id_provider = app().get_method<methods::get_head_block_id>().register_provider( [this]() {
         return my->chain->head_block_id();
      } );

      my->get_last_irreversible_block_number_provider = app().get_method<methods::get_last_irreversible_block_number>().register_provider(
            [this]() {
               return my->chain->last_irreversible_block_num();
            } );

      // relay signals to channels
      my->pre_accepted_block_connection = my->chain->pre_accepted_block.connect([this](const signed_block_ptr& blk) {
         auto itr = my->loaded_checkpoints.find( blk->block_num() );
         if( itr != my->loaded_checkpoints.end() ) {
            auto id = blk->id();
            ULTRAIN_ASSERT( itr->second == id, checkpoint_exception,
                        "Checkpoint does not match for block number ${num}: expected: ${expected} actual: ${actual}",
                        ("num", blk->block_num())("expected", itr->second)("actual", id)
            );
         }

         my->pre_accepted_block_channel.publish(blk);
      });

      my->accepted_block_header_connection = my->chain->accepted_block_header.connect(
            [this]( const block_state_ptr& blk ) {
               my->accepted_block_header_channel.publish( blk );
            } );

      my->accepted_block_connection = my->chain->accepted_block.connect( [this]( const block_state_ptr& blk ) {
         my->accepted_block_channel.publish( blk );
      } );

      my->irreversible_block_connection = my->chain->irreversible_block.connect( [this]( const block_state_ptr& blk ) {
         my->irreversible_block_channel.publish( blk );
      } );

      my->accepted_transaction_connection = my->chain->accepted_transaction.connect(
            [this]( const transaction_metadata_ptr& meta ) {
               my->accepted_transaction_channel.publish( meta );
            } );

      my->applied_transaction_connection = my->chain->applied_transaction.connect(
            [this]( const transaction_trace_ptr& trace ) {
               my->applied_transaction_channel.publish( trace );
            } );

      my->chain->add_indices();
   } FC_LOG_AND_RETHROW()

}

void chain_plugin::plugin_startup()
{ try {
   try {
      if (my->worldstate_path) {
         my->chain->startup(*(my->worldstate_path));
      } else {
         my->chain->startup();
      }
   } catch (const database_guard_exception& e) {
      log_guard_exception(e);
      // make sure to properly close the db
      my->chain.reset();
      throw;
   }

   if(!my->readonly) {
      ilog("starting chain in read/write mode");
   }

   ilog("Blockchain started; head block is #${num}, genesis timestamp is ${ts}",
        ("num", my->chain->head_block_num())("ts", (std::string)my->chain_config->genesis.initial_timestamp));

   my->chain_config.reset();
} FC_CAPTURE_AND_RETHROW() }

void chain_plugin::plugin_shutdown() {
   my->pre_accepted_block_connection.reset();
   my->accepted_block_header_connection.reset();
   my->accepted_block_connection.reset();
   my->irreversible_block_connection.reset();
   my->accepted_transaction_connection.reset();
   my->applied_transaction_connection.reset();
   my->accepted_confirmation_connection.reset();
   my->chain.reset();
}

chain_apis::read_write chain_plugin::get_read_write_api() {
   return chain_apis::read_write(chain(), get_abi_serializer_max_time());
}

void chain_plugin::accept_block(const signed_block_ptr& block ) {
   my->incoming_block_sync_method(block);
}

void chain_plugin::accept_transaction(const chain::packed_transaction& trx,
                                      bool from_network,
                                      next_function<chain::transaction_trace_ptr> next) {
    my->incoming_transaction_async_method(std::make_shared<packed_transaction>(trx), std::forward<bool>(from_network), false, std::forward<decltype(next)>(next));
}

bool chain_plugin::block_is_on_preferred_chain(const block_id_type& block_id) {
   auto b = chain().fetch_block_by_number( block_header::num_from_id(block_id) );
   return b && b->id() == block_id;
}

controller::config& chain_plugin::chain_config() {
   // will trigger optional assert if called before/after plugin_initialize()
   return *my->chain_config;
}

controller& chain_plugin::chain() { return *my->chain; }
const controller& chain_plugin::chain() const { return *my->chain; }

chain::chain_id_type chain_plugin::get_chain_id()const {
   ULTRAIN_ASSERT( my->chain_id.valid(), chain_id_type_exception, "chain ID has not been initialized yet" );
   return *my->chain_id;
}

fc::microseconds chain_plugin::get_abi_serializer_max_time() const {
   return my->abi_serializer_max_time_ms;
}

void chain_plugin::log_guard_exception(const chain::guard_exception&e ) const {
   if (e.code() == chain::database_guard_exception::code_value) {
      elog("Database has reached an unsafe level of usage, shutting down to avoid corrupting the database.  "
           "Please increase the value set for \"chain-state-db-size-mb\" and restart the process!");
   }
   dlog("Details: ${details}", ("details", e.to_detail_string()));
}

void chain_plugin::handle_guard_exception(const chain::guard_exception& e) const {
   log_guard_exception(e);

   // quit the app
   app().shutdown();
}

namespace chain_apis {

const string read_only::KEYi64 = "i64";

read_only::get_info_results read_only::get_chain_info(const read_only::get_chain_info_params&) const {
    const auto& rm = db.get_resource_limits_manager();
   return {
      ultrainio::utilities::common::itoh(static_cast<uint32_t>(app().version())),
      db.get_chain_id(),
      db.fork_db_head_block_num(),
      db.last_irreversible_block_num(),
      db.last_irreversible_block_id(),
      db.fork_db_head_block_id(),
      db.fork_db_head_block_time(),
      db.fork_db_head_block_proposer(),
      rm.get_virtual_block_cpu_limit(),
      rm.get_virtual_block_net_limit(),
      rm.get_block_cpu_limit(),
      rm.get_block_net_limit(),
      static_cast<uint32_t>(block_interval_ms)
      //std::bitset<64>(db.get_dynamic_global_properties().recent_slots_filled).to_string(),
      //__builtin_popcountll(db.get_dynamic_global_properties().recent_slots_filled) / 64.0
   };
}

uint64_t read_only::get_table_index_name(const read_only::get_table_records_params& p, bool& primary) {
   using boost::algorithm::starts_with;
   // see multi_index packing of index name
   const uint64_t table = p.table;
   uint64_t index = table & 0xFFFFFFFFFFFFFFF0ULL;
   ULTRAIN_ASSERT( index == table, chain::contract_table_query_exception, "Unsupported table name: ${n}", ("n", p.table) );

   primary = false;
   uint64_t pos = 0;
   if (p.index_position.empty() || p.index_position == "first" || p.index_position == "primary" || p.index_position == "one") {
      primary = true;
   } else if (starts_with(p.index_position, "sec") || p.index_position == "two") { // second, secondary
   } else if (starts_with(p.index_position , "ter") || starts_with(p.index_position, "th")) { // tertiary, ternary, third, three
      pos = 1;
   } else if (starts_with(p.index_position, "fou")) { // four, fourth
      pos = 2;
   } else if (starts_with(p.index_position, "fi")) { // five, fifth
      pos = 3;
   } else if (starts_with(p.index_position, "six")) { // six, sixth
      pos = 4;
   } else if (starts_with(p.index_position, "sev")) { // seven, seventh
      pos = 5;
   } else if (starts_with(p.index_position, "eig")) { // eight, eighth
      pos = 6;
   } else if (starts_with(p.index_position, "nin")) { // nine, ninth
      pos = 7;
   } else if (starts_with(p.index_position, "ten")) { // ten, tenth
      pos = 8;
   } else {
      try {
         pos = fc::to_uint64( p.index_position );
      } catch(...) {
         ULTRAIN_ASSERT( false, chain::contract_table_query_exception, "Invalid index_position: ${p}", ("p", p.index_position));
      }
      if (pos < 2) {
         primary = true;
         pos = 0;
      } else {
         pos -= 2;
      }
   }
   index |= (pos & 0x000000000000000FULL);
   return index;
}

template<>
uint64_t convert_to_type(const string& str, const string& desc) {
   uint64_t value = 0;
   try {
      value = boost::lexical_cast<uint64_t>(str.c_str(), str.size());
   } catch( ... ) {
      try {
         auto trimmed_str = str;
         boost::trim(trimmed_str);
         name s(trimmed_str);
         value = s.value;
      } catch( ... ) {
         try {
            auto symb = ultrainio::chain::symbol::from_string(str);
            value = symb.value();
         } catch( ... ) {
            try {
               value = ( ultrainio::chain::string_to_symbol( 0, str.c_str() ) >> 8 );
            } catch( ... ) {
               ULTRAIN_ASSERT( false, chain_type_exception, "Could not convert ${desc} string '${str}' to any of the following: "
                                 "uint64_t, valid name, or valid symbol (with or without the precision)",
                          ("desc", desc)("str", str));
            }
         }
      }
   }
   return value;
}

abi_def get_abi( const controller& db, const name& account ) {
   const auto &d = db.db();
   const account_object *code_accnt = d.find<account_object, by_name>(account);
   ULTRAIN_ASSERT(code_accnt != nullptr, chain::account_query_exception, "Fail to retrieve account for ${account}", ("account", account) );
   abi_def abi;
   abi_serializer::to_abi(code_accnt->abi, abi);
   return abi;
}

string get_table_type( const abi_def& abi, const name& table_name ) {
   for( const auto& t : abi.tables ) {
      if( t.name == table_name ){
         return t.index_type;
      }
   }
   ULTRAIN_ASSERT( false, chain::contract_table_query_exception, "Table ${table} is not specified in the ABI", ("table",table_name) );
}

read_only::get_table_records_result read_only::get_table_records( const read_only::get_table_records_params& p )const {
   const abi_def abi = ultrainio::chain_apis::get_abi( db, p.code );

   bool primary = false;
   auto table_with_index = get_table_index_name( p, primary );
   if( primary ) {
      ULTRAIN_ASSERT( p.table == table_with_index, chain::contract_table_query_exception, "Invalid table name ${t}", ( "t", p.table ));
      auto table_type = get_table_type( abi, p.table );
      if( table_type == KEYi64 || p.key_type == "i64" || p.key_type == "name" ) {
         return get_table_records_ex<key_value_index>(p,abi);
      }
      ULTRAIN_ASSERT( false, chain::contract_table_query_exception,  "Invalid table type ${type}", ("type",table_type)("abi",abi));
   } else {
      ULTRAIN_ASSERT( !p.key_type.empty(), chain::contract_table_query_exception, "key type required for non-primary index" );

      if (p.key_type == "i64" || p.key_type == "name") {
         return get_table_records_by_seckey<index64_index, uint64_t>(p, abi, [](uint64_t v)->uint64_t {
            return v;
         });
      }
      else if (p.key_type == "i128") {
         return get_table_records_by_seckey<index128_index, uint128_t>(p, abi, [](uint128_t v)->uint128_t {
            return v;
         });
      }
      else if (p.key_type == "i256") {
         return get_table_records_by_seckey<index256_index, uint256_t>(p, abi, [](uint256_t v)->key256_t {
            key256_t k;
            k[0] = ((uint128_t *)&v)[0];
            k[1] = ((uint128_t *)&v)[1];
            return k;
         });
      }
      else if (p.key_type == "float64") {
         return get_table_records_by_seckey<index_double_index, double>(p, abi, [](double v)->float64_t {
            float64_t f = *(float64_t *)&v;
            return f;
         });
      }
      else if (p.key_type == "float128") {
         return get_table_records_by_seckey<index_long_double_index, double>(p, abi, [](double v)->float128_t{
            float64_t f = *(float64_t *)&v;
            float128_t f128;
            f64_to_f128M(f, &f128);
            return f128;
         });
      }
      ULTRAIN_ASSERT(false, chain::contract_table_query_exception,  "Unsupported secondary index type: ${t}", ("t", p.key_type));
   }
}

read_only::get_table_by_scope_result read_only::get_table_by_scope( const read_only::get_table_by_scope_params& p )const {
   const auto& d = db.db();
   const auto& idx = d.get_index<chain::table_id_multi_index, chain::by_code_scope_table>();
   decltype(idx.lower_bound(boost::make_tuple(0, 0, 0))) lower;
   decltype(idx.upper_bound(boost::make_tuple(0, 0, 0))) upper;

   if (p.lower_bound.size()) {
      uint64_t scope = convert_to_type<uint64_t>(p.lower_bound, "lower_bound scope");
      lower = idx.lower_bound( boost::make_tuple(p.code, scope, p.table));
   } else {
      lower = idx.lower_bound(boost::make_tuple(p.code, 0, p.table));
   }
   if (p.upper_bound.size()) {
      uint64_t scope = convert_to_type<uint64_t>(p.upper_bound, "upper_bound scope");
      upper = idx.lower_bound( boost::make_tuple(p.code, scope, 0));
   } else {
      upper = idx.lower_bound(boost::make_tuple((uint64_t)p.code + 1, 0, 0));
   }
   auto end = fc::time_point::now() + fc::microseconds(1000 * 100); /// 100 ms max time
   unsigned int count = 0;
   auto itr = lower;
   read_only::get_table_by_scope_result result;
   for (; itr != upper; ++itr) {
      if (p.table && itr->table != p.table) {
         if (fc::time_point::now() > end) {
            break;
         }
         continue;
      }

      result.rows.push_back({itr->code, itr->scope, itr->table, itr->payer, itr->count});
      if (++count == p.limit || fc::time_point::now() > end) {
         ++itr;
         break;
      }
   }
   if (itr != upper) {
      result.more = itr->scope;
   } else {
      result.more = 0;
   }
   return result;
}

vector<asset> read_only::get_currency_balance( const read_only::get_currency_balance_params& p )const {

   const abi_def abi = ultrainio::chain_apis::get_abi( db, p.code );
   auto table_type = get_table_type( abi, "accounts" );

   vector<asset> results;
   walk_key_value_table(p.code, p.account, N(accounts), [&](const key_value_object& obj){
      ULTRAIN_ASSERT( obj.value.size() >= sizeof(asset), chain::asset_type_exception, "Invalid data on table");

      asset cursor;
      fc::datastream<const char *> ds(obj.value.data(), obj.value.size());
      fc::raw::unpack(ds, cursor);

      ULTRAIN_ASSERT( cursor.get_symbol().valid(), chain::asset_type_exception, "Invalid asset");

      if( !p.symbol || boost::iequals(cursor.symbol_name(), *p.symbol) ) {
        results.emplace_back(cursor);
      }

      // return false if we are looking for one and found it, true otherwise
      return !(p.symbol && boost::iequals(cursor.symbol_name(), *p.symbol));
   });

   return results;
}

fc::variant read_only::get_currency_stats( const read_only::get_currency_stats_params& p )const {
   fc::mutable_variant_object results;

   const abi_def abi = ultrainio::chain_apis::get_abi( db, p.code );
   auto table_type = get_table_type( abi, "stat" );

   uint64_t scope = ( ultrainio::chain::string_to_symbol( 0, boost::algorithm::to_upper_copy(p.symbol).c_str() ) >> 8 );

   walk_key_value_table(p.code, scope, N(stat), [&](const key_value_object& obj){
      ULTRAIN_ASSERT( obj.value.size() >= sizeof(read_only::get_currency_stats_result), chain::asset_type_exception, "Invalid data on table");

      fc::datastream<const char *> ds(obj.value.data(), obj.value.size());
      read_only::get_currency_stats_result result;

      fc::raw::unpack(ds, result.supply);
      fc::raw::unpack(ds, result.max_supply);
      fc::raw::unpack(ds, result.issuer);

      results[result.supply.symbol_name()] = result;
      return true;
   });

   return results;
}

vector<read_only::get_subchain_committee_result> read_only::get_subchain_committee( const read_only::get_subchain_committee_params& p )const {
   const abi_def abi = ultrainio::chain_apis::get_abi( db, N(ultrainio) );
   ULTRAIN_ASSERT(p.chain_name != master_chain_name, chain::contract_table_query_exception, "Could not query committee list of master chain.");

   name table = N(producers);
   auto index_type = get_table_type( abi, table );

   vector<get_subchain_committee_result> result;
   walk_key_value_table(N(ultrainio), p.chain_name, table, [&](const key_value_object& obj){
//       ULTRAIN_ASSERT( obj.value.size() >= sizeof(ultrainio::chain::subchain), chain::asset_type_exception, "Invalid subchain data on table");

       ultrainio::chain::producer_info producer_data;
       fc::datastream<const char *> ds(obj.value.data(), obj.value.size());
       fc::raw::unpack(ds, producer_data);

       read_only::get_subchain_committee_result tmp;
       tmp.owner = producer_data.owner.to_string();
       tmp.miner_pk = producer_data.producer_key;
       tmp.bls_pk = producer_data.bls_key;
       result.push_back(tmp);
       return true;
   });

   return result;
}

vector<ultrainio::chain::resources_lease> read_only::get_subchain_resource(const read_only::get_subchain_resource_params& p) const {
vector<ultrainio::chain::resources_lease> results;
    const abi_def abi = ultrainio::chain_apis::get_abi( db, N(ultrainio) );
    auto index_type = get_table_type( abi, N(reslease) );
    walk_key_value_table(N(ultrainio), p.chain_name, N(reslease), [&](const key_value_object& obj) {
        resources_lease resLease;
        fc::datastream<const char *> ds(obj.value.data(), obj.value.size());
        fc::raw::unpack(ds, resLease);
        auto headblock = db.head_block_num();
        auto blockinterval = (30 * 60 * 1000) /config::block_interval_ms;
        if(resLease.modify_block_height <= headblock && (headblock - resLease.modify_block_height) <= blockinterval) {
            results.push_back(resLease);
        }
        return true;
    } );
    return results;
}

read_only::get_subchain_block_num_result read_only::get_subchain_block_num(const read_only::get_subchain_block_num_params& p) const {
   const abi_def abi = ultrainio::chain_apis::get_abi( db, N(ultrainio) );
   ULTRAIN_ASSERT(p.chain_name != master_chain_name, chain::contract_table_query_exception, "Could not query committee list of master chain.");

   name table = N(chains);
   auto index_type = get_table_type( abi, table );

   read_only::get_subchain_block_num_result result;
   walk_key_value_table(N(ultrainio), N(ultrainio), table, [&](const key_value_object& obj){
   //    ULTRAIN_ASSERT( obj.value.size() >= sizeof(subchain), chain::asset_type_exception, "Invalid subchain data on table");

       chain_info subchain_data;
       fc::datastream<const char *> ds(obj.value.data(), obj.value.size());
       fc::raw::unpack(ds, subchain_data);
       if(p.chain_name == subchain_data.chain_name) {
           result.confirmed_block.number = subchain_data.confirmed_block_number;
           result.confirmed_block.block_id = subchain_data.confirmed_block_id;
           auto ite_block = subchain_data.unconfirmed_blocks.begin();
           for(; ite_block != subchain_data.unconfirmed_blocks.end(); ++ite_block) {
               if(ite_block->is_leaf) {
                   block_num_and_id temp_num_id;
                   temp_num_id.number   = ite_block->block_number;
                   temp_num_id.block_id = ite_block->block_id;
                   result.forks.push_back(temp_num_id);
               }
           }
           return false;
       }
       else {
           return true;
       }
   });
   return result;
}

read_only::get_subchain_unconfirmed_header_result read_only::get_subchain_unconfirmed_header(const read_only::get_subchain_unconfirmed_header_params& p) const {
    const abi_def abi = ultrainio::chain_apis::get_abi( db, N(ultrainio) );

    name table = N(chains);
    auto index_type = get_table_type( abi, table );

    read_only::get_subchain_unconfirmed_header_result result;
    walk_key_value_table(N(ultrainio), N(ultrainio), table, [&](const key_value_object& obj) {
        chain_info chain_data;
        fc::datastream<const char *> ds(obj.value.data(), obj.value.size());
        fc::raw::unpack(ds, chain_data);
        if(p.chain_name == chain_data.chain_name) {
            result.committee_set = chain_data.committee_set;
            result.confirmed_block_id = chain_data.confirmed_block_id;
            uint32_t confirmBlockNum = chain_data.confirmed_block_number;
            for (auto e : chain_data.unconfirmed_blocks) {
                if (chain_data.confirmed_block_id == e.block_id) {
                    result.next_committee_mroot = e.next_committee_mroot;
                } else {
                    if (e.block_number > confirmBlockNum) {
                        result.unconfirmed_headers.push_back(e);
                    }
                }
            }
            return false;
        }
        else {
            return true;
        }
    });
    return result;
}

read_only::get_master_block_num_result read_only::get_master_block_num(const read_only::get_master_block_num_params& p) const {
   const abi_def abi = ultrainio::chain_apis::get_abi( db, N(ultrainio) );

   name table = N(chains);
   auto index_type = get_table_type( abi, table );

   read_only::get_master_block_num_result result;
   walk_key_value_table(N(ultrainio), N(ultrainio), table, [&](const key_value_object& obj){
   //    ULTRAIN_ASSERT( obj.value.size() >= sizeof(chain_info), chain::asset_type_exception, "Invalid master data on table");

       chain_info master_data;
       fc::datastream<const char *> ds(obj.value.data(), obj.value.size());
       fc::raw::unpack(ds, master_data);
       if("master" == master_data.chain_name) {
           result.confirmed_block.number = master_data.confirmed_block_number;
           auto ite_block = master_data.unconfirmed_blocks.begin();
           for(; ite_block != master_data.unconfirmed_blocks.end(); ++ite_block) {
               if(ite_block->is_leaf) {
                   block_num_and_id temp_num_id;
                   temp_num_id.number   = ite_block->block_number;
                   temp_num_id.block_id = ite_block->block_id;
                   result.forks.push_back(temp_num_id);
               }
               if(ite_block->block_number == result.confirmed_block.number) {
                   result.confirmed_block.block_id = ite_block->block_id;
               }
           }
           return false;
       }
       else {
           return true;
       }
   });
   return result;
}

std::string read_only::get_subchain_ws_hash(const read_only::get_subchain_ws_hash_params& p) const {
        return "hash";
}
read_only::get_producer_info_result read_only::get_producer_info(const read_only::get_producer_info_params& p) const {
    const abi_def abi = ultrainio::chain_apis::get_abi( db, N(ultrainio) );

    name table = N(briefprod);
    auto index_type = get_table_type( abi, table );
    read_only::get_producer_info_result result;
    result.location = std::numeric_limits<uint64_t>::max();
    walk_key_value_table(N(ultrainio), N(ultrainio), table, [&](const key_value_object& obj){
       producer_brief producer_data;
       fc::datastream<const char *> ds(obj.value.data(), obj.value.size());
       fc::raw::unpack(ds, producer_data);
       if(p.owner == producer_data.owner) {
           result.location = producer_data.location;
           return false;
       }
       else {
           return true;
       }
    });

    if(result.location == chain::master_chain_name) {
        result.chain_id = db.get_chain_id();
        result.genesis_time = block_timestamp(); //TODO, modify as master genesis time
    }
    else if (result.location != std::numeric_limits<uint64_t>::max()) {
        table = N(chains);
        auto index_type = get_table_type( abi, table );
        walk_key_value_table(N(ultrainio), N(ultrainio), table, [&](const key_value_object& obj){
            chain_info subchain_data;
            fc::datastream<const char *> ds(obj.value.data(), obj.value.size());
            fc::raw::unpack(ds, subchain_data);
            if(result.location == subchain_data.chain_name) {
                result.chain_id = string(subchain_data.chain_id);
                result.genesis_time = subchain_data.genesis_time;
                return false;
            }
            else {
                return true;
            }
        });
    }
    result.quit_before_num = 0; // todo, quey it from table
    return result;
}

std::vector<read_only::get_user_bulletin_result> read_only::get_user_bulletin(const read_only::get_user_bulletin_params& p) const {
    const abi_def abi = ultrainio::chain_apis::get_abi( db, N(ultrainio) );

    name table = N(chains);
    auto index_type = get_table_type( abi, table );
    const auto& d = db.db();
    std::vector<read_only::get_user_bulletin_result> result;
    walk_key_value_table(N(ultrainio), N(ultrainio), table, [&](const key_value_object& obj){
       chain_info subchain_data;
       fc::datastream<const char *> ds(obj.value.data(), obj.value.size());
       fc::raw::unpack(ds, subchain_data);
       if(p.chain_name == subchain_data.chain_name) {
           for(int32_t index = subchain_data.recent_users.size() - 1; index >= 0; --index) {
               read_only::get_user_bulletin_result tmpuser;
               tmpuser.owner = subchain_data.recent_users[index].user_name.to_string();
               tmpuser.issue_date = fc::time_point_sec(subchain_data.recent_users[index].emp_time);
               if(subchain_data.recent_users[index].is_producer || subchain_data.recent_users[index].owner_key == "") {
                   //producer will always use the same pk as in master
                   const auto& permission_o = d.get<permission_object,by_owner>(boost::make_tuple(subchain_data.recent_users[index].user_name, N(owner)));
                   tmpuser.owner_pk = string(permission_o.auth.keys[0].key);
               }
               else{
                   tmpuser.owner_pk = subchain_data.recent_users[index].owner_key;
               }

               if(subchain_data.recent_users[index].is_producer ||subchain_data.recent_users[index].active_key == "") {
                   const auto& permission_a = d.get<permission_object,by_owner>(boost::make_tuple(subchain_data.recent_users[index].user_name, N(active)));
                   tmpuser.active_pk = string(permission_a.auth.keys[0].key);
               }
               else {
                   tmpuser.active_pk = subchain_data.recent_users[index].active_key;
               }
               tmpuser.block_height = subchain_data.recent_users[index].block_height;
               tmpuser.updateable = subchain_data.recent_users[index].updateable;
               result.push_back(tmpuser);
           }
           return false;
       }
       else {
           return true;
       }
    });
    return result;
}

static fc::variant get_global_row( const database& db, const abi_def& abi, const fc::microseconds& abi_serializer_max_time_ms ) {
   const auto table_type = get_table_type(abi, N(global));
   const abi_serializer abis{ abi, abi_serializer_max_time_ms };
   ULTRAIN_ASSERT(table_type == read_only::KEYi64, chain::contract_table_query_exception, "Invalid table type ${type} for table global", ("type",table_type));

   const auto* const table_id = db.find<chain::table_id_object, chain::by_code_scope_table>(boost::make_tuple(N(ultrainio), N(ultrainio), N(global)));
   ULTRAIN_ASSERT(table_id, chain::contract_table_query_exception, "Missing table global");

   const auto& kv_index = db.get_index<key_value_index, by_scope_primary>();
   const auto it = kv_index.find(boost::make_tuple(table_id->id, N(global)));
   ULTRAIN_ASSERT(it != kv_index.end(), chain::contract_table_query_exception, "Missing row in table global");

   vector<char> data;
   read_only::copy_inline_row(*it, data);
   return abis.binary_to_variant(abis.get_table_type(N(global)), data, abi_serializer_max_time_ms);
}

read_only::get_producers_result read_only::get_producers( const read_only::get_producers_params& p ) const {
   const abi_def abi = ultrainio::chain_apis::get_abi(db, N(ultrainio));
   const auto table_type = get_table_type(abi, N(producers));
   const auto subchian_table_type = get_table_type( abi, N(chains) );
   const abi_serializer abis{ abi, abi_serializer_max_time };
   ULTRAIN_ASSERT(table_type == KEYi64, chain::contract_table_query_exception, "Invalid table type ${type} for table producers", ("type",table_type));

   const auto& d = db.db();

   static const uint8_t secondary_index_num = 0;
   std::vector<name> scopes;
   if(p.all_chain) {
       scopes.emplace_back(master_chain_name); //add master
       //add side chains
       walk_key_value_table(N(ultrainio), N(ultrainio), N(chains), [&](const key_value_object& obj){
           ultrainio::chain::chain_info subchain_data;
           fc::datastream<const char *> ds(obj.value.data(), obj.value.size());
           fc::raw::unpack(ds, subchain_data);
           scopes.emplace_back(subchain_data.chain_name);
           return true;
       });
   }
   else {
       scopes.emplace_back(p.chain_name);
   }
   read_only::get_producers_result result;
   for(auto i = 0; i < scopes.size(); ++i) {
       if(scopes[i] == N(master))
           continue;
       const auto* const table_id = d.find<chain::table_id_object, chain::by_code_scope_table>(boost::make_tuple(N(ultrainio), scopes[i], N(producers)));
       const auto* const secondary_table_id = d.find<chain::table_id_object, chain::by_code_scope_table>(boost::make_tuple(N(ultrainio), scopes[i], N(producers) | secondary_index_num));
       if(table_id == nullptr || secondary_table_id == nullptr) {
           ULTRAIN_ASSERT(scopes[i] != master_chain_name, chain::contract_table_query_exception, "Missing master producers table");
           continue;
       }

       const auto& kv_index = d.get_index<key_value_index, by_scope_primary>();
       decltype(table_id->id) next_tid(table_id->id._id + 1);
       auto lower = kv_index.lower_bound(boost::make_tuple(table_id->id));
       auto upper = kv_index.lower_bound(boost::make_tuple(next_tid));

       const auto stopTime = fc::time_point::now() + fc::microseconds(1000 * 10); // 10ms
       vector<char> data;

       std::for_each(lower,upper, [&](const key_value_object& obj) {
           copy_inline_row(obj, data);
           auto producer = abis.binary_to_variant(abis.get_table_type(N(producers)), data, abi_serializer_max_time);
           if (p.json)
               result.rows.emplace_back(scopes[i], abis.binary_to_variant(abis.get_table_type(N(producers)), data, abi_serializer_max_time));
           else
               result.rows.emplace_back(scopes[i], fc::variant(data));
       });
   }
   return result;
}

read_only::get_confirm_point_interval_result read_only::get_confirm_point_interval(const read_only::get_confirm_point_interval_params& p) const {
    const abi_def abi = ultrainio::chain_apis::get_abi(db, N(ultrainio));
    const auto& d = db.db();
    read_only::get_confirm_point_interval_result result;
    try {
        auto gstate = get_global_row(d, abi, abi_serializer_max_time);
        exten_types ext = gstate["table_extension"].as<exten_types>();
        for (auto v : ext) {
            if (v.key == global_state_exten_type_key::confirm_point_interval) {
                result.confirm_point_interval = std::stoi(v.value);
                ilog("read confirm_point_interval from global state : ${value}", ("value", result.confirm_point_interval));
                break;
            }
        }
    } catch (fc::exception& e) {
        elog("get_confirm_point_interval exception : ${e}", ("e", e.to_string()));
    }
    return result;
}

read_only::get_global_exten_result read_only::get_global_exten_data(const read_only::get_global_exten_params& p) const {
    const abi_def abi = ultrainio::chain_apis::get_abi(db, N(ultrainio));
    const auto& d = db.db();
    read_only::get_global_exten_result result;
    try {
        auto gstate = get_global_row(d, abi, abi_serializer_max_time);
        exten_types ext = gstate["table_extension"].as<exten_types>();
        for ( auto v : ext ) {
            if( v.key == p.index ) {
                result.global_exten_data = v.value;
                ilog("read get_global_exten_data from global state : ${value}", ("value", result.global_exten_data));
                break;
            }
        }
    } catch (fc::exception& e) {
        elog("get_global_exten_data exception : ${e}", ("e", e.to_string()));
    } catch (...) {
        elog("get_global_exten_data error thrown from get_global_row ");
    }
    return result;
}
bool read_only::is_exec_patch_code( int patch_version_number ) const{
   try {
      read_only::get_global_exten_params p;
      p.index = read_only::global_state_exten_type_key::version_number;
      read_only::get_global_exten_result result = get_global_exten_data( p );
      if( result.global_exten_data.empty() )
         return false;
      else
         return std::stoi(result.global_exten_data.c_str()) >= patch_version_number ? true : false;
   } catch (...) {
      elog("is_exec_patch_code exception");
   }
   return false;
}
static fc::variant get_row( const database& db, const abi_def& abi, const abi_serializer& abis, const fc::microseconds& abi_serializer_max_time_ms, const name& key) {
    const auto table_type = get_table_type(abi, N(rand));
    ULTRAIN_ASSERT(table_type == read_only::KEYi64, chain::contract_table_query_exception, "Invalid table type ${type} for table rand", ("type",table_type));

    const auto* const table_id = db.find<chain::table_id_object, chain::by_code_scope_table>(boost::make_tuple(N(utrio.rand), N(rand), N(rand)));
    if (table_id == nullptr) {
        return fc::variants();
    }

    const auto& kv_index = db.get_index<key_value_index, by_scope_primary>();
    const auto it = kv_index.find(boost::make_tuple(table_id->id, key));
    if (it == kv_index.end()) {
        return fc::variants();
    }

    vector<char> data;
    read_only::copy_inline_row(*it, data);
    return abis.binary_to_variant(abis.get_table_type(N(rand)), data, abi_serializer_max_time_ms);
}

read_only::get_random_result read_only::get_random(const read_only::get_random_params& p) const {
    const abi_def abi = ultrainio::chain_apis::get_abi(db, N(utrio.rand));
    const auto table_type = get_table_type(abi, N(rand));
    const abi_serializer serializer { abi, abi_serializer_max_time };
    ULTRAIN_ASSERT(table_type == KEYi64, chain::contract_table_query_exception, "Invalid table type ${type} for table rand", ("type", table_type));

    get_random_result result;
    const auto& d = db.db();

    const auto max_bck_num_key = 1;
    auto max_bck_num_entry = get_row(d, abi, serializer, abi_serializer_max_time, max_bck_num_key);
    if (max_bck_num_entry.is_null()) {
        return result;
    }
    const auto rand_key = "val";
    uint64_t max_bck_num = max_bck_num_entry[rand_key].as_uint64();
    auto latest_rand = get_row(d, abi, serializer, abi_serializer_max_time, max_bck_num);
    if (max_bck_num_entry.is_null()) {
        return result;
    }
    ilog("latest block random number: ${entry}", ("entry", latest_rand));
    result.random = latest_rand[rand_key].as_string();
    return result;
}

bool read_only::is_genesis_finished() const{
    const abi_def abi = ultrainio::chain_apis::get_abi(db, N(ultrainio));
    const auto& d = db.db();
    try {
        auto gstate = get_global_row(d, abi, abi_serializer_max_time);
        auto committee_num = gstate["cur_committee_number"].as_uint64();
        auto min_committee_num = gstate["min_committee_member_number"].as_uint64();
        return committee_num >= min_committee_num;
    }
    catch (fc::exception& e) {
        ilog("is_genesis_finished there may be no producer registered: ${e}", ("e", e.to_string()));
        return false;
    } catch (...) {
        elog("is_genesis_finished error thrown from get_global_row ");
        return false;
    }
}

uint64_t read_only::get_worldstate_interval() const{
    const abi_def abi = ultrainio::chain_apis::get_abi(db, N(ultrainio));
    const auto& d = db.db();
    try {
        auto gstate = get_global_row(d, abi, abi_serializer_max_time);
        auto interval = gstate["worldstate_interval"].as_uint64();
        return interval;
    } catch ( const fc::exception& e ){
        elog( "get_worldstate_interval: ${e}", ("e",e.to_detail_string()));
        return 0;
    } catch (...) {
        elog("error thrown from get_worldstate_interval");
        return 0;
    }
}

template<typename Api>
struct resolver_factory {
   static auto make(const Api* api, const fc::microseconds& max_serialization_time) {
      return [api, max_serialization_time](const account_name &name) -> optional<abi_serializer> {
         const auto* accnt = api->db.db().template find<account_object, by_name>(name);
         if (accnt != nullptr) {
            abi_def abi;
            if (abi_serializer::to_abi(accnt->abi, abi)) {
               return abi_serializer(abi, max_serialization_time);
            }
         }

         return optional<abi_serializer>();
      };
   }
};

template<typename Api>
auto make_resolver(const Api* api, const fc::microseconds& max_serialization_time) {
   return resolver_factory<Api>::make(api, max_serialization_time);
}


read_only::get_scheduled_transactions_result
read_only::get_scheduled_transactions( const read_only::get_scheduled_transactions_params& p ) const {
   const auto& d = db.db();

   const auto& idx_by_delay = d.get_index<generated_transaction_multi_index,by_delay>();
   auto itr = ([&](){
      if (!p.lower_bound.empty()) {
         try {
            auto when = time_point::from_iso_string( p.lower_bound );
            return idx_by_delay.lower_bound(boost::make_tuple(when));
         } catch (...) {
            try {
               auto txid = transaction_id_type(p.lower_bound);
               const auto& by_txid = d.get_index<generated_transaction_multi_index,by_trx_id>();
               auto itr = by_txid.find( txid );
               if (itr == by_txid.end()) {
                  ULTRAIN_THROW(transaction_exception, "Unknown Transaction ID: ${txid}", ("txid", txid));
               }

               return d.get_index<generated_transaction_multi_index>().indices().project<by_delay>(itr);

            } catch (...) {
               return idx_by_delay.end();
            }
         }
      } else {
         return idx_by_delay.begin();
      }
   })();

   read_only::get_scheduled_transactions_result result;

   auto resolver = make_resolver(this, abi_serializer_max_time);

   uint32_t remaining = p.limit;
   auto time_limit = fc::time_point::now() + fc::microseconds(1000 * 10); /// 10ms max time
   while (itr != idx_by_delay.end() && remaining > 0 && time_limit > fc::time_point::now()) {
      auto row = fc::mutable_variant_object()
              ("trx_id", itr->trx_id)
              ("sender", itr->sender)
              ("sender_id", itr->sender_id)
              ("payer", itr->payer)
              ("delay_until", itr->delay_until)
              ("expiration", itr->expiration)
              ("published", itr->published)
      ;

      if (p.json) {
         fc::variant pretty_transaction;

         transaction trx;
         fc::datastream<const char*> ds( itr->packed_trx.data(), itr->packed_trx.size() );
         fc::raw::unpack(ds,trx);

         abi_serializer::to_variant(trx, pretty_transaction, resolver, abi_serializer_max_time);
         row("transaction", pretty_transaction);
      } else {
         auto packed_transaction = bytes(itr->packed_trx.begin(), itr->packed_trx.end());
         row("transaction", packed_transaction);
      }

      result.transactions.emplace_back(std::move(row));
      ++itr;
      remaining--;
   }

   if (itr != idx_by_delay.end()) {
      result.more = string(itr->trx_id);
   }

   return result;
}

fc::variant read_only::get_block_info(const read_only::get_block_info_params& params) const {
   signed_block_ptr block;
   ULTRAIN_ASSERT(!params.block_num_or_id.empty() && params.block_num_or_id.size() <= 64, chain::block_id_type_exception, "Invalid Block number or ID, must be greater than 0 and less than 64 characters" );
   try {
      block = db.fetch_block_by_id(fc::variant(params.block_num_or_id).as<block_id_type>());
      if (!block) {
         block = db.fetch_block_by_number(fc::to_uint64(params.block_num_or_id));
      }

   } ULTRAIN_RETHROW_EXCEPTIONS(chain::block_id_type_exception, "Invalid block ID: ${block_num_or_id}", ("block_num_or_id", params.block_num_or_id))

   ULTRAIN_ASSERT( block, unknown_block_exception, "Could not find block: ${block}", ("block", params.block_num_or_id));

   fc::variant pretty_output;
   abi_serializer::to_variant(*block, pretty_output, make_resolver(this, abi_serializer_max_time), abi_serializer_max_time);

   uint32_t ref_block_prefix = block->id()._hash[1];

   return fc::mutable_variant_object(pretty_output.get_object())
           ("id", block->id())
           ("block_num",block->block_num())
           ("ref_block_prefix", ref_block_prefix)
           ("timevalue", block->timestamp.abstime);
}

read_only::get_merkle_proof_result read_only::get_merkle_proof(const read_only::get_merkle_proof_params& params) {
   read_only::get_merkle_proof_result result;

   vector<char> trx_receipt_bytes;
   result.merkle_proof =  db.merkle_proof_of(params.block_number, params.trx_id, trx_receipt_bytes);
   result.trx_receipt_bytes = trx_receipt_bytes;

   return result;
}

read_only::verify_merkle_proof_result read_only::verify_merkle_proof(const read_only::verify_merkle_proof_params& params) {
   read_only::verify_merkle_proof_result result;
   result.is_matched = db.verify_merkle_proof(params.merkle_proof, params.transaction_mroot, params.trx_receipt_bytes);
   return result;
}

fc::variant read_only::get_block_header_state(const get_block_header_state_params& params) const {
   block_state_ptr b;
   optional<uint64_t> block_num;
   std::exception_ptr e;
   try {
      block_num = fc::to_uint64(params.block_num_or_id);
   } catch( ... ) {}

   if( block_num.valid() ) {
      b = db.fetch_block_state_by_number(*block_num);
   } else {
      try {
         b = db.fetch_block_state_by_id(fc::variant(params.block_num_or_id).as<block_id_type>());
      } ULTRAIN_RETHROW_EXCEPTIONS(chain::block_id_type_exception, "Invalid block ID: ${block_num_or_id}", ("block_num_or_id", params.block_num_or_id))
   }

   ULTRAIN_ASSERT( b, unknown_block_exception, "Could not find block: ${block}", ("block", params.block_num_or_id));

   fc::variant vo;
   fc::to_variant( static_cast<const block_header_state&>(*b), vo );
   return vo;
}

void read_write::push_block(const read_write::push_block_params& params, next_function<read_write::push_block_results> next) {
   try {
      app().get_method<incoming::methods::block_sync>()(std::make_shared<signed_block>(params));
      next(read_write::push_block_results{});
   } catch ( boost::interprocess::bad_alloc& ) {
      raise(SIGUSR1);
   } CATCH_AND_CALL(next);
}

void read_write::push_tx(const read_write::push_tx_params& params, next_function<read_write::push_tx_results> next) {

   try {
      auto pretty_input = std::make_shared<packed_transaction>();
      auto resolver = make_resolver(this, abi_serializer_max_time);
      try {
         abi_serializer::from_variant(params, *pretty_input, resolver, abi_serializer_max_time);
      } ULTRAIN_RETHROW_EXCEPTIONS(chain::packed_transaction_type_exception, "Invalid packed transaction")

         app().get_method<incoming::methods::transaction_async>()(pretty_input, false, true, [this, next](const fc::static_variant<fc::exception_ptr, transaction_trace_ptr>& result) -> void{
         if (result.contains<fc::exception_ptr>()) {
            next(result.get<fc::exception_ptr>());
         } else {
            auto trx_trace_ptr = result.get<transaction_trace_ptr>();

            try {
               fc::variant pretty_output;
               pretty_output = db.to_variant_with_abi(*trx_trace_ptr, abi_serializer_max_time);

               chain::transaction_id_type id = trx_trace_ptr->id;
               next(read_write::push_tx_results{id, pretty_output});
            } CATCH_AND_CALL(next);
         }
      });


   } catch ( boost::interprocess::bad_alloc& ) {
      raise(SIGUSR1);
   } CATCH_AND_CALL(next);
}

static void push_recurse(read_write* rw, int index, const std::shared_ptr<read_write::push_txs_params>& params, const std::shared_ptr<read_write::push_txs_results>& results, const next_function<read_write::push_txs_results>& next) {
   auto wrapped_next = [=](const fc::static_variant<fc::exception_ptr, read_write::push_tx_results>& result) {
      if (result.contains<fc::exception_ptr>()) {
         const auto& e = result.get<fc::exception_ptr>();
         results->emplace_back( read_write::push_tx_results{ transaction_id_type(), fc::mutable_variant_object( "error", e->to_detail_string() ) } );
      } else {
         const auto& r = result.get<read_write::push_tx_results>();
         results->emplace_back( r );
      }

      int next_index = index + 1;
      if (next_index < params->size()) {
         push_recurse(rw, next_index, params, results, next );
      } else {
         next(*results);
      }
   };

   rw->push_tx(params->at(index), wrapped_next);
}

void read_write::push_txs(const read_write::push_txs_params& params, next_function<read_write::push_txs_results> next) {
   try {
      ULTRAIN_ASSERT( params.size() <= 1000, too_many_tx_at_once, "Attempt to push too many transactions at once" );
      auto params_copy = std::make_shared<read_write::push_txs_params>(params.begin(), params.end());
      auto result = std::make_shared<read_write::push_txs_results>();
      result->reserve(params.size());

      push_recurse(this, 0, params_copy, result, next);

   } CATCH_AND_CALL(next);
}

read_write::register_result read_write::register_event(const register_event_params& params) {
   ilog("register_event for ${account}, post_url is ${post_url}", ("account", params.account) ("post_url", params.post_url));
   read_write::register_result res;
   res.result = "Success";
   try {
      db.register_event(params.account, params.post_url);
   } catch (chain_exception& e) {
      res.result = e.what();
   } catch (fc::exception& e) {
      res.result = e.what();
   } catch (...) {
      res.result = "Unknown error.";
   }

   return res;
}

read_write::register_result read_write::unregister_event(const unregister_event_params& params) {
   ilog("unregister_event for ${account} post_url:${url}", ("account", params.account)("url", params.post_url));

   read_write::register_result res;
   res.result = "Success";
   try {
      db.unregister_event(params.account, params.post_url);
   } catch (chain_exception& e) {
      res.result = e.what();
   } catch (fc::exception& e) {
      res.result = e.what();
   } catch (...) {
      res.result = "Unknown error.";
   }

   return res;
}

read_only::get_abi_results read_only::get_abi( const get_abi_params& params )const {
   get_abi_results result;
   result.account_name = params.account_name;
   const auto& d = db.db();
   const auto& accnt  = d.get<account_object,by_name>( params.account_name );

   abi_def abi;
   if( abi_serializer::to_abi(accnt.abi, abi) ) {
      result.abi = std::move(abi);
   }

   return result;
}

read_only::get_contract_results read_only::get_contract( const get_contract_params& params )const {
   get_contract_results result;
   result.account_name = params.account_name;
   const auto& d = db.db();
   const auto& accnt  = d.get<account_object,by_name>( params.account_name );

   if( accnt.code.size() ) {
      if (params.code_as_wasm) {
         result.wasm = string(accnt.code.begin(), accnt.code.end());
      } else {
         result.wast = wasm_to_wast( (const uint8_t*)accnt.code.data(), accnt.code.size(), true );
      }
      result.code_hash = fc::sha256::hash( accnt.code.data(), accnt.code.size() );
   }

   abi_def abi;
   if( abi_serializer::to_abi(accnt.abi, abi) ) {

      result.abi = std::move(abi);
   }

   return result;
}

read_only::get_raw_code_and_abi_results read_only::get_raw_code_and_abi( const get_raw_code_and_abi_params& params)const {
   get_raw_code_and_abi_results result;
   result.account_name = params.account_name;

   const auto& d = db.db();
   const auto& accnt = d.get<account_object,by_name>(params.account_name);
   result.wasm = blob{{accnt.code.begin(), accnt.code.end()}};
   result.abi = blob{{accnt.abi.begin(), accnt.abi.end()}};

   return result;
}

read_only::get_account_results read_only::get_account_info( const get_account_info_params& params )const {
   get_account_results result;
   result.account_name = params.account_name;

   const auto& d = db.db();
   const auto& rm = db.get_resource_limits_manager();

   result.head_block_num  = db.head_block_num();
   result.head_block_time = db.head_block_time();

   rm.get_account_limits( result.account_name, result.ram_quota, result.net_weight, result.cpu_weight );

   const auto& a = db.get_account(result.account_name);

   result.privileged       = a.privileged;
   result.last_code_update = a.last_code_update;
   result.created          = a.creation_date;
   result.updateable       = a.updateable;
   bool grelisted = db.is_resource_greylisted(result.account_name);
   result.net_limit = rm.get_account_net_limit_ex( result.account_name, !grelisted);
   result.cpu_limit = rm.get_account_cpu_limit_ex( result.account_name, !grelisted);
   result.ram_usage = rm.get_account_ram_usage( result.account_name );


   const auto& permissions = d.get_index<permission_index,by_owner>();
   auto perm = permissions.lower_bound( boost::make_tuple( params.account_name ) );
   while( perm != permissions.end() && perm->owner == params.account_name ) {
      /// TODO: lookup perm->parent name
      name parent;

      // Don't lookup parent if null
      if( perm->parent._id ) {
         const auto* p = d.find<permission_object,by_id>( perm->parent );
         if( p ) {
            ULTRAIN_ASSERT(perm->owner == p->owner, invalid_parent_permission, "Invalid parent permission");
            parent = p->name;
         }
      }

      result.permissions.push_back( permission{ perm->name, parent, perm->auth.to_authority() } );
      ++perm;
   }

   const auto& code_account = db.db().get<account_object,by_name>( N(ultrainio) );

   abi_def abi;
   if( abi_serializer::to_abi(code_account.abi, abi) ) {
      abi_serializer abis( abi, abi_serializer_max_time );

      const auto token_code = N(utrio.token);

      const auto* t_id = d.find<chain::table_id_object, chain::by_code_scope_table>(boost::make_tuple( token_code, params.account_name, N(accounts) ));
      if( t_id != nullptr ) {
         const auto &idx = d.get_index<key_value_index, by_scope_primary>();
         auto it = idx.find(boost::make_tuple( t_id->id, symbol().to_symbol_code() ));
         if( it != idx.end() && it->value.size() >= sizeof(asset) ) {
            asset bal;
            fc::datastream<const char *> ds(it->value.data(), it->value.size());
            fc::raw::unpack(ds, bal);

            if( bal.get_symbol().valid() && bal.get_symbol() == symbol() ) {
               result.core_liquid_balance = bal;
            }
         }
      }

      t_id = d.find<chain::table_id_object, chain::by_code_scope_table>(boost::make_tuple( config::system_account_name, params.account_name, N(delcons) ));
      if (t_id != nullptr) {
         const auto &idx = d.get_index<key_value_index, by_scope_primary>();
         auto it = idx.find(boost::make_tuple( t_id->id, params.account_name ));
         if ( it != idx.end() ) {
            vector<char> data;
            copy_inline_row(*it, data);
            result.self_delegated_consensus = abis.binary_to_variant( "delegated_consensus", data, abi_serializer_max_time );
         }
      }

      t_id = d.find<chain::table_id_object, chain::by_code_scope_table>(boost::make_tuple( config::system_account_name, params.account_name, N(refundscons) ));
      if (t_id != nullptr) {
         const auto &idx = d.get_index<key_value_index, by_scope_primary>();
         auto it = idx.find(boost::make_tuple( t_id->id, params.account_name ));
         if ( it != idx.end() ) {
            vector<char> data;
            copy_inline_row(*it, data);
            result.refund_cons = abis.binary_to_variant( "refund_cons", data, abi_serializer_max_time );
         }
      }
      auto const get_producer_delegate_func = [&]( bool in_disable, const name& location){
         name cur_serch_scope = location;
         uint64_t cur_serch_table = N(producers);
         std::string prods_struct_name = "producer_info";
         if(in_disable){
            cur_serch_scope = config::system_account_name;
            cur_serch_table = N(disableprods);
            prods_struct_name = "disabled_producer";
         }
         auto prods_id = d.find<chain::table_id_object, chain::by_code_scope_table>(boost::make_tuple( config::system_account_name, cur_serch_scope, cur_serch_table ));
         if (prods_id != nullptr) {
            const auto &idx = d.get_index<key_value_index, by_scope_primary>();
            auto it = idx.find(boost::make_tuple( prods_id->id, params.account_name ));
            if ( it != idx.end() ) {
               vector<char> data;
               copy_inline_row(*it, data);
               result.producer_info = abis.binary_to_variant( prods_struct_name, data, abi_serializer_max_time );
            }
         }
      };
      t_id = d.find<chain::table_id_object, chain::by_code_scope_table>(boost::make_tuple( config::system_account_name, config::system_account_name, N(briefprod) ));
      if (t_id != nullptr) {
         const auto &idx = d.get_index<key_value_index, by_scope_primary>();
         auto it = idx.find(boost::make_tuple( t_id->id, params.account_name ));
         if ( it != idx.end() ) {
            vector<char> data;
            copy_inline_row(*it, data);
            auto producer_brief = abis.binary_to_variant( "producer_brief", data, abi_serializer_max_time );
            bool in_disable = producer_brief["in_disable"].as_bool();
            name location = producer_brief["location"].as_string();
            get_producer_delegate_func(in_disable, location);
         }
      }
      auto chain_resource_func = [&](const vector<char>& data, name chain_name){
         auto resleaseinfo = abis.binary_to_variant( "resources_lease", data, abi_serializer_max_time );
         read_only::get_block_info_params blockinfoparams{resleaseinfo["start_block_height"].as_string()};
         auto blockinfo = get_block_info(blockinfoparams);
         string str_start_time = blockinfo["timestamp"].as_string();
         fc::time_point start_timestamp = time_point::from_iso_string( str_start_time );
         fc::time_point end_timestamp(microseconds((resleaseinfo["end_block_height"].as_uint64() - resleaseinfo["start_block_height"].as_uint64())*config::block_interval_us + start_timestamp.time_since_epoch().count()));
         result.chain_resource.push_back( resourcelease{ chain_name, resleaseinfo["lease_num"].as_uint64(), string(start_timestamp), string(end_timestamp) } );
      };
      t_id = d.find<chain::table_id_object, chain::by_code_scope_table>(boost::make_tuple( config::system_account_name, config::system_account_name, N(reslease) ));
      if (t_id != nullptr) {
         const auto &idx = d.get_index<key_value_index, by_scope_primary>();
         auto it = idx.find(boost::make_tuple( t_id->id, params.account_name ));
         if ( it != idx.end() ) {
            vector<char> data;
            copy_inline_row(*it, data);
            chain_resource_func( data, config::system_account_name );
         }
      }

      auto table_id = d.find<chain::table_id_object, chain::by_code_scope_table>(boost::make_tuple( config::system_account_name, config::system_account_name, N(chains) ));
      if (table_id != nullptr) {
         const auto& kv_index = d.get_index<key_value_index, by_scope_primary>();
         decltype(table_id->id) next_tid(table_id->id._id + 1);
         auto lower = kv_index.lower_bound(boost::make_tuple(table_id->id));
         auto upper = kv_index.lower_bound(boost::make_tuple(next_tid));
         std::for_each(lower,upper, [&](const key_value_object& obj) {
            vector<char> data;
            copy_inline_row(obj, data);
            auto subchain = abis.binary_to_variant(abis.get_table_type(N(chains)), data, abi_serializer_max_time);
            name chain_name = subchain["chain_name"].as_string();
            const auto* res_id = d.find<chain::table_id_object, chain::by_code_scope_table>(boost::make_tuple( config::system_account_name, chain_name, N(reslease) ));
            if (res_id != nullptr) {
               const auto &idx = d.get_index<key_value_index, by_scope_primary>();
               auto itres = idx.find(boost::make_tuple( res_id->id, params.account_name ));
               if ( itres != idx.end() ) {
                  vector<char> data;
                  copy_inline_row(*itres, data);
                  chain_resource_func(data, chain_name);
               }
            }
         });
      }
   }
   return result;
}

read_only::account_exist_result read_only::get_account_exist( const get_account_exist_params& params )const {
    account_exist_result result;
    result.is_exist = nullptr != db.db().find<account_object,by_name>( params.account_name );
    return result;
}

static variant action_abi_to_variant( const abi_def& abi, type_name action_type ) {
   variant v;
   auto it = std::find_if(abi.structs.begin(), abi.structs.end(), [&](auto& x){return x.name == action_type;});
   if( it != abi.structs.end() )
      to_variant( it->fields,  v );
   return v;
};

read_only::abi_json2bin_result read_only::abi_json2bin( const read_only::abi_json2bin_params& params )const try {
   abi_json2bin_result result;
   const auto code_account = db.db().find<account_object,by_name>( params.code );
   ULTRAIN_ASSERT(code_account != nullptr, contract_query_exception, "Contract can't be found ${contract}", ("contract", params.code));

   abi_def abi;
   if( abi_serializer::to_abi(code_account->abi, abi) ) {
      abi_serializer abis( abi, abi_serializer_max_time );
      auto action_type = abis.get_action_type(params.action);
      ULTRAIN_ASSERT(!action_type.empty(), action_validate_exception, "Unknown action ${action} in contract ${contract}", ("action", params.action)("contract", params.code));
      try {
         result.binargs = abis.variant_to_binary(action_type, params.args, abi_serializer_max_time);
      } ULTRAIN_RETHROW_EXCEPTIONS(chain::invalid_action_args_exception,
                                "'${args}' is invalid args for action '${action}' code '${code}'. expected '${proto}'",
                                ("args", params.args)("action", params.action)("code", params.code)("proto", action_abi_to_variant(abi, action_type)))
   } else {
      ULTRAIN_ASSERT(false, abi_not_found_exception, "No ABI found for ${contract}", ("contract", params.code));
   }
   return result;
} FC_RETHROW_EXCEPTIONS( warn, "code: ${code}, action: ${action}, args: ${args}",
                         ("code", params.code)( "action", params.action )( "args", params.args ))

read_only::abi_bin2json_result read_only::abi_bin2json( const read_only::abi_bin2json_params& params )const {
   abi_bin2json_result result;
   const auto& code_account = db.db().get<account_object,by_name>( params.code );
   abi_def abi;
   if( abi_serializer::to_abi(code_account.abi, abi) ) {
      abi_serializer abis( abi, abi_serializer_max_time );
      result.args = abis.binary_to_variant( abis.get_action_type( params.action ), params.binargs, abi_serializer_max_time );
   } else {
      ULTRAIN_ASSERT(false, abi_not_found_exception, "No ABI found for ${contract}", ("contract", params.code));
   }
   return result;
}

read_only::get_required_keys_result read_only::get_required_keys( const get_required_keys_params& params )const {
   transaction pretty_input;
   auto resolver = make_resolver(this, abi_serializer_max_time);
   try {
      abi_serializer::from_variant(params.transaction, pretty_input, resolver, abi_serializer_max_time);
   } ULTRAIN_RETHROW_EXCEPTIONS(chain::transaction_type_exception, "Invalid transaction")

   auto required_keys_set = db.get_authorization_manager().get_required_keys(pretty_input, params.available_keys);
   get_required_keys_result result;
   result.required_keys = required_keys_set;
   return result;
}


} // namespace chain_apis
} // namespace ultrainio
