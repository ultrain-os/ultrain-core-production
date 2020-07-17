/**
 *  @file
 *  @copyright defined in ultrain/LICENSE.txt
 */
#include <ultrainio/mongo_db_plugin/mongo_db_plugin.hpp>
#include <ultrainio/chain/ultrainio_contract.hpp>
#include <ultrainio/chain/config.hpp>
#include <ultrainio/chain/exceptions.hpp>
#include <ultrainio/chain/transaction.hpp>
#include <ultrainio/chain/types.hpp>

#include <fc/io/json.hpp>
#include <fc/utf8.hpp>
#include <fc/variant.hpp>

#include <boost/algorithm/string.hpp>
#include <boost/chrono.hpp>
#include <boost/signals2/connection.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>

#include <queue>

#include <bsoncxx/builder/basic/kvp.hpp>
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/exception/exception.hpp>
#include <bsoncxx/json.hpp>

#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/pool.hpp>
#include <mongocxx/exception/operation_exception.hpp>
#include <mongocxx/exception/logic_error.hpp>
#include <ultrainio/chain/plugin_interface.hpp>
using namespace ultrainio::chain::plugin_interface::compat;
using namespace ultrainio::chain;
namespace fc { class variant; }

namespace ultrainio {

using chain::account_name;
using chain::action_name;
using chain::block_id_type;
using chain::permission_name;
using chain::transaction;
using chain::signed_transaction;
using chain::signed_block;
using chain::transaction_id_type;
using chain::packed_transaction;
using chain::packed_generated_transaction;

static appbase::abstract_plugin& _mongo_db_plugin = app().register_plugin<mongo_db_plugin>();

struct filter_entry {
   account_name receiver;
   action_name action;
   account_name actor;

   friend bool operator<( const filter_entry& a, const filter_entry& b ) {
      return std::tie( a.receiver, a.action, a.actor ) < std::tie( b.receiver, b.action, b.actor );
   }

   //            receiver          action       actor
   bool match( const account_name& rr, const action_name& an, const account_name& ar ) const {
      return (receiver.value == 0 || receiver == rr) &&
             ((action.valueH == 0 && action.valueL == 0) || action == an) &&
             (actor.value == 0 || actor == ar);
   }
};

struct transaction_metadata_blocknum {
   chain::transaction_metadata_ptr trx_ptr;
   uint32_t block_num;
};

struct transaction_trace_blocknum {
   chain::transaction_trace_ptr trx_trace_ptr;
   uint32_t block_num;
};

class mongo_db_plugin_impl {
public:
   mongo_db_plugin_impl();
   ~mongo_db_plugin_impl();

   fc::optional<boost::signals2::scoped_connection> accepted_block_header_connection;
   fc::optional<boost::signals2::scoped_connection> accepted_block_connection;
   fc::optional<boost::signals2::scoped_connection> irreversible_block_connection;
   fc::optional<boost::signals2::scoped_connection> accepted_transaction_connection;
   fc::optional<boost::signals2::scoped_connection> applied_transaction_connection;

   void consume_blocks();

   void accepted_block( const chain::block_state_ptr& );
   void applied_irreversible_block(const chain::block_state_ptr&);
   void accepted_transaction(const chain::transaction_metadata_ptr&);
   void applied_transaction(const chain::transaction_trace_ptr&);
   void process_accepted_transaction(const transaction_metadata_blocknum&);
   void _process_accepted_transaction(const transaction_metadata_blocknum&);
   void process_applied_transaction(const transaction_trace_blocknum&);
   void _process_applied_transaction(const transaction_trace_blocknum&);
   void process_accepted_block( const chain::block_state_ptr& );
   void _process_accepted_block( const chain::block_state_ptr& );
   void process_irreversible_block(const chain::block_state_ptr&);
   void _process_irreversible_block(const chain::block_state_ptr&);
   void process_failed_transaction( const chain::packed_transaction_ptr&, const string& );
   void _process_failed_transaction( const chain::packed_transaction_ptr&, const string& );

   optional<abi_serializer> get_abi_serializer( account_name n );
   template<typename T> fc::variant to_variant_with_abi( const T& obj );

   void purge_abi_cache();

   bool add_action_trace( mongocxx::bulk_write& bulk_action_traces, const chain::action_trace& atrace,
                          const transaction_trace_blocknum& tb,
                          bool executed, const std::chrono::milliseconds& now,
                          bool& write_ttrace );

   void update_account(const chain::action& act);

   void add_pub_keys( const vector<chain::key_weight>& keys, const account_name& name,
                      const permission_name& permission, const std::chrono::milliseconds& now );
   void remove_pub_keys( const account_name& name, const permission_name& permission );
   void add_account_control( const vector<chain::permission_level_weight>& controlling_accounts,
                             const account_name& name, const permission_name& permission,
                             const std::chrono::milliseconds& now );
   void remove_account_control( const account_name& name, const permission_name& permission );

   /// @return true if act should be added to mongodb, false to skip it
   bool filter_include( const account_name& receiver, const action_name& act_name,
                        const vector<chain::permission_level>& authorization ) const;
   bool filter_include( const transaction& trx ) const;

   void init();
   void wipe_database();

   template<typename Queue, typename Entry> void queue(Queue& queue, const Entry& e);

   bool configured{false};
   bool wipe_database_on_startup{false};

   uint32_t current_block_num = 0;
   uint32_t start_block_num = 0;
   time_point block_time = fc::time_point::now();

   bool is_producer = false;
   bool filter_on_star = true;
   std::set<filter_entry> filter_on;
   std::set<filter_entry> filter_out;
   bool update_blocks_via_block_num = false;
   bool store_blocks = true;
   bool store_block_states = true;
   bool store_transactions = true;
   bool store_transaction_traces = true;
   bool store_actions = true;
   bool store_action_traces = true;

   std::string db_name;
   mongocxx::instance mongo_inst;
   fc::optional<mongocxx::pool> mongo_pool;

   // consum thread
   mongocxx::collection _accounts;
   mongocxx::collection _trans;
   mongocxx::collection _trans_traces;
   mongocxx::collection _actions;
   mongocxx::collection _action_traces;
   mongocxx::collection _block_states;
   mongocxx::collection _blocks;
   mongocxx::collection _pub_keys;
   mongocxx::collection _account_controls;
   mongocxx::collection _failed_trans;

   size_t max_queue_size = 0;
   int queue_sleep_time = 0;
   size_t abi_cache_size = 0;
   std::deque<transaction_metadata_blocknum> transaction_metadata_queue;
   std::deque<transaction_metadata_blocknum> transaction_metadata_process_queue;
   std::deque<transaction_trace_blocknum> transaction_trace_queue;
   std::deque<transaction_trace_blocknum> transaction_trace_process_queue;
   std::deque<chain::block_state_ptr> block_state_queue;
   std::deque<chain::block_state_ptr> block_state_process_queue;
   std::deque<chain::block_state_ptr> irreversible_block_state_queue;
   std::deque<chain::block_state_ptr> irreversible_block_state_process_queue;
   boost::mutex mtx;
   boost::condition_variable condition;
   boost::thread consume_thread;
   std::atomic_bool done{false};
   std::atomic_bool startup{true};
   fc::optional<chain::chain_id_type> chain_id;
   fc::microseconds abi_serializer_max_time;

   struct by_account;
   struct by_last_access;
   channels::transaction_ack::channel_type::handle  incoming_transaction_ack_subscription;
   void transaction_ack(const std::tuple<const fc::exception_ptr, const chain::transaction_trace_ptr, const chain::packed_transaction_ptr>&);
   struct abi_cache {
      account_name                     account;
      fc::time_point                   last_accessed;
      fc::optional<abi_serializer>     serializer;
   };

   typedef boost::multi_index_container<abi_cache,
         indexed_by<
               ordered_unique< tag<by_account>,  member<abi_cache,account_name,&abi_cache::account> >,
               ordered_non_unique< tag<by_last_access>,  member<abi_cache,fc::time_point,&abi_cache::last_accessed> >
         >
   > abi_cache_index_t;

   abi_cache_index_t abi_cache_index;

   static const action_name newaccount;
   static const action_name setabi;
   static const action_name updateauth;
   static const action_name deleteauth;
   static const permission_name owner;
   static const permission_name active;

   static const std::string block_states_col;
   static const std::string blocks_col;
   static const std::string trans_col;
   static const std::string trans_traces_col;
   static const std::string actions_col;
   static const std::string action_traces_col;
   static const std::string accounts_col;
   static const std::string pub_keys_col;
   static const std::string account_controls_col;
   static const std::string failed_trans_col;
};

const action_name mongo_db_plugin_impl::newaccount = chain::newaccount::get_name();
const action_name mongo_db_plugin_impl::setabi = chain::setabi::get_name();
const action_name mongo_db_plugin_impl::updateauth = chain::updateauth::get_name();
const action_name mongo_db_plugin_impl::deleteauth = chain::deleteauth::get_name();
const permission_name mongo_db_plugin_impl::owner = chain::config::owner_name;
const permission_name mongo_db_plugin_impl::active = chain::config::active_name;

const std::string mongo_db_plugin_impl::block_states_col = "block_states";
const std::string mongo_db_plugin_impl::blocks_col = "blocks";
const std::string mongo_db_plugin_impl::trans_col = "transactions";
const std::string mongo_db_plugin_impl::trans_traces_col = "transaction_traces";
const std::string mongo_db_plugin_impl::actions_col = "actions";
const std::string mongo_db_plugin_impl::action_traces_col = "action_traces";
const std::string mongo_db_plugin_impl::accounts_col = "accounts";
const std::string mongo_db_plugin_impl::pub_keys_col = "pub_keys";
const std::string mongo_db_plugin_impl::account_controls_col = "account_controls";
const std::string mongo_db_plugin_impl::failed_trans_col = "failed_trxs";

bool mongo_db_plugin_impl::filter_include( const account_name& receiver, const action_name& act_name,
                                           const vector<chain::permission_level>& authorization ) const
{
   bool include = false;
   if( filter_on_star ) {
      include = true;
   } else {
      auto itr = std::find_if( filter_on.cbegin(), filter_on.cend(), [&receiver, &act_name]( const auto& filter ) {
         return filter.match( receiver, act_name, 0 );
      } );
      if( itr != filter_on.cend() ) {
         include = true;
      } else {
         for( const auto& a : authorization ) {
            auto itr = std::find_if( filter_on.cbegin(), filter_on.cend(), [&receiver, &act_name, &a]( const auto& filter ) {
               return filter.match( receiver, act_name, a.actor );
            } );
            if( itr != filter_on.cend() ) {
               include = true;
               break;
            }
         }
      }
   }

   if( !include ) { return false; }
   if( filter_out.empty() ) { return true; }

   auto itr = std::find_if( filter_out.cbegin(), filter_out.cend(), [&receiver, &act_name]( const auto& filter ) {
      return filter.match( receiver, act_name, 0 );
   } );
   if( itr != filter_out.cend() ) { return false; }

   for( const auto& a : authorization ) {
      auto itr = std::find_if( filter_out.cbegin(), filter_out.cend(), [&receiver, &act_name, &a]( const auto& filter ) {
         return filter.match( receiver, act_name, a.actor );
      } );
      if( itr != filter_out.cend() ) { return false; }
   }

   return true;
}

bool mongo_db_plugin_impl::filter_include( const transaction& trx ) const
{
   if( !filter_on_star || !filter_out.empty() ) {
      bool include = false;
      for( const auto& a : trx.actions ) {
         if( filter_include( a.account, a.name, a.authorization ) ) {
            include = true;
            break;
         }
      }
      if( !include ) {
         for( const auto& a : trx.context_free_actions ) {
            if( filter_include( a.account, a.name, a.authorization ) ) {
               include = true;
               break;
            }
         }
      }
      return include;
   }
   return true;
}


template<typename Queue, typename Entry>
void mongo_db_plugin_impl::queue( Queue& queue, const Entry& e ) {
   boost::mutex::scoped_lock lock( mtx );
   auto queue_size = queue.size();
   if( queue_size > max_queue_size ) {
      lock.unlock();
      condition.notify_one();
      queue_sleep_time += 10;
      if( queue_sleep_time > 1000 )
         wlog("queue size: ${q}", ("q", queue_size));
      boost::this_thread::sleep_for( boost::chrono::milliseconds( queue_sleep_time ));
      lock.lock();
   } else {
      queue_sleep_time -= 10;
      if( queue_sleep_time < 0 ) queue_sleep_time = 0;
   }
   queue.emplace_back( e );
   lock.unlock();
   condition.notify_one();
}

void mongo_db_plugin_impl::accepted_transaction( const chain::transaction_metadata_ptr& t ) {
   try {
      if( current_block_num >= start_block_num && store_transactions ) {
         const transaction_metadata_blocknum trx = {t, current_block_num};
         queue( transaction_metadata_queue, trx );
      }
   } catch (fc::exception& e) {
      elog("FC Exception while accepted_transaction ${e}", ("e", e.to_string()));
   } catch (std::exception& e) {
      elog("STD Exception while accepted_transaction ${e}", ("e", e.what()));
   } catch (...) {
      elog("Unknown exception while accepted_transaction");
   }
}

void mongo_db_plugin_impl::applied_transaction( const chain::transaction_trace_ptr& t ) {
   try {
     if ( current_block_num >= start_block_num ) {
        const transaction_trace_blocknum trace = {t, current_block_num};
        queue( transaction_trace_queue, trace );
     }
   } catch (fc::exception& e) {
      elog("FC Exception while applied_transaction ${e}", ("e", e.to_string()));
   } catch (std::exception& e) {
      elog("STD Exception while applied_transaction ${e}", ("e", e.what()));
   } catch (...) {
      elog("Unknown exception while applied_transaction");
   }
}

void mongo_db_plugin_impl::applied_irreversible_block( const chain::block_state_ptr& bs ) {
   try {
      if( bs->block_num >= start_block_num && ( store_blocks || store_block_states || store_transactions ) ) {
         queue( irreversible_block_state_queue, bs );
      }
   } catch (fc::exception& e) {
      elog("FC Exception while applied_irreversible_block ${e}", ("e", e.to_string()));
   } catch (std::exception& e) {
      elog("STD Exception while applied_irreversible_block ${e}", ("e", e.what()));
   } catch (...) {
      elog("Unknown exception while applied_irreversible_block");
   }
}

void mongo_db_plugin_impl::accepted_block( const chain::block_state_ptr& bs ) {
   try {
      if( bs->block_num >= start_block_num && ( store_blocks || store_block_states ) ) {
         queue( block_state_queue, bs );
      }
   } catch (fc::exception& e) {
      elog("FC Exception while accepted_block ${e}", ("e", e.to_string()));
   } catch (std::exception& e) {
      elog("STD Exception while accepted_block ${e}", ("e", e.what()));
   } catch (...) {
      elog("Unknown exception while accepted_block");
   }
}

void mongo_db_plugin_impl::consume_blocks() {
   try {
      auto mongo_client = mongo_pool->acquire();
      auto& mongo_conn = *mongo_client;

      _accounts = mongo_conn[db_name][accounts_col];
      _trans = mongo_conn[db_name][trans_col];
      _trans_traces = mongo_conn[db_name][trans_traces_col];
      _actions = mongo_conn[db_name][actions_col];
      _action_traces = mongo_conn[db_name][action_traces_col];
      _blocks = mongo_conn[db_name][blocks_col];
      _block_states = mongo_conn[db_name][block_states_col];
      _pub_keys = mongo_conn[db_name][pub_keys_col];
      _account_controls = mongo_conn[db_name][account_controls_col];
      _failed_trans = mongo_conn[db_name][failed_trans_col];

      while (true) {
         boost::mutex::scoped_lock lock(mtx);
         while ( transaction_metadata_queue.empty() &&
                 transaction_trace_queue.empty() &&
                 block_state_queue.empty() &&
                 irreversible_block_state_queue.empty() &&
                 !done ) {
            condition.wait(lock);
         }

         // capture for processing
         size_t transaction_metadata_size = transaction_metadata_queue.size();
         if (transaction_metadata_size > 0) {
            transaction_metadata_process_queue = move(transaction_metadata_queue);
            transaction_metadata_queue.clear();
         }
         size_t transaction_trace_size = transaction_trace_queue.size();
         if (transaction_trace_size > 0) {
            transaction_trace_process_queue = move(transaction_trace_queue);
            transaction_trace_queue.clear();
         }
         size_t block_state_size = block_state_queue.size();
         if (block_state_size > 0) {
            block_state_process_queue = move(block_state_queue);
            block_state_queue.clear();
         }
         size_t irreversible_block_size = irreversible_block_state_queue.size();
         if (irreversible_block_size > 0) {
            irreversible_block_state_process_queue = move(irreversible_block_state_queue);
            irreversible_block_state_queue.clear();
         }

         lock.unlock();

         if (done) {
            ilog("draining queue, size: ${q}", ("q", transaction_metadata_size + transaction_trace_size + block_state_size + irreversible_block_size));
         }

         // process transactions
         auto start_time = fc::time_point::now();
         auto size = transaction_trace_process_queue.size();
	 //ilog("start transaction_trace_process size: ${s}", ("s", size));
         while (!transaction_trace_process_queue.empty()) {
            const auto& t = transaction_trace_process_queue.front();
            process_applied_transaction(t);
            transaction_trace_process_queue.pop_front();
         }
         auto time = fc::time_point::now() - start_time;
         auto per = size > 0 ? time.count()/size : 0;
         if( time > fc::microseconds(500000) ) // reduce logging, .5 secs
            ilog( "process_applied_transaction,  time per: ${p}, size: ${s}, time: ${t}", ("s", size)("t", time)("p", per) );

         start_time = fc::time_point::now();
         size = transaction_metadata_process_queue.size();
	 //ilog("start transaction_metadata_process size: ${s}", ("s", size));
         while (!transaction_metadata_process_queue.empty()) {
            const auto& t = transaction_metadata_process_queue.front();
            process_accepted_transaction(t);
            transaction_metadata_process_queue.pop_front();
         }
         time = fc::time_point::now() - start_time;
         per = size > 0 ? time.count()/size : 0;
         if( time > fc::microseconds(500000) ) // reduce logging, .5 secs
            ilog( "process_accepted_transaction, time per: ${p}, size: ${s}, time: ${t}", ("s", size)( "t", time )( "p", per ));

         // process blocks
         start_time = fc::time_point::now();
         size = block_state_process_queue.size();
	 //ilog("start block_state_process size: ${s}", ("s", size));
         while (!block_state_process_queue.empty()) {
            const auto& bs = block_state_process_queue.front();
            process_accepted_block( bs );
            block_state_process_queue.pop_front();
         }
         time = fc::time_point::now() - start_time;
         per = size > 0 ? time.count()/size : 0;
         if( time > fc::microseconds(500000) ) // reduce logging, .5 secs
            ilog( "process_accepted_block,       time per: ${p}, size: ${s}, time: ${t}", ("s", size)("t", time)("p", per) );

         // process irreversible blocks
         start_time = fc::time_point::now();
         size = irreversible_block_state_process_queue.size();
	 //ilog("start irreversible_block_state size: ${s}", ("s", size));
         while (!irreversible_block_state_process_queue.empty()) {
            const auto& bs = irreversible_block_state_process_queue.front();
            process_irreversible_block(bs);
            irreversible_block_state_process_queue.pop_front();
         }
         time = fc::time_point::now() - start_time;
         per = size > 0 ? time.count()/size : 0;
         if( time > fc::microseconds(500000) ) // reduce logging, .5 secs
            ilog( "process_irreversible_block,   time per: ${p}, size: ${s}, time: ${t}", ("s", size)("t", time)("p", per) );

         //ilog("finish irreversible_block_state");
         if( transaction_metadata_size == 0 &&
             transaction_trace_size == 0 &&
             block_state_size == 0 &&
             irreversible_block_size == 0 &&
             done ) {
            break;
         }
      }
      ilog("mongo_db_plugin consume thread shutdown gracefully");
   } catch (fc::exception& e) {
      elog("FC Exception while consuming block ${e}", ("e", e.to_string()));
   } catch (std::exception& e) {
      elog("STD Exception while consuming block ${e}", ("e", e.what()));
   } catch (...) {
      elog("Unknown exception while consuming block");
   }
}

namespace {

auto find_account( mongocxx::collection& accounts, const account_name& name ) {
   using bsoncxx::builder::basic::make_document;
   using bsoncxx::builder::basic::kvp;
   return accounts.find_one( make_document( kvp( "name", name.to_string())));
}

auto find_transaction(mongocxx::collection& trans, const string& id) {
   using bsoncxx::builder::basic::make_document;
   using bsoncxx::builder::basic::kvp;
   return trans.find_one( make_document( kvp( "trx_id", id )));
}

auto find_block( mongocxx::collection& blocks, const string& id ) {
   using bsoncxx::builder::basic::make_document;
   using bsoncxx::builder::basic::kvp;

   mongocxx::options::find options;
   options.projection( make_document( kvp( "_id", 1 )) ); // only return _id
   return blocks.find_one( make_document( kvp( "block_id", id )), options);
}

void handle_mongo_exception( const std::string& desc, int line_num ) {
   bool shutdown = true;
   try {
      try {
         throw;
      } catch( mongocxx::logic_error& e) {
         // logic_error on invalid key, do not shutdown
         wlog( "mongo logic error, ${desc}, line ${line}, code ${code}, ${what}",
               ("desc", desc)( "line", line_num )( "code", e.code().value() )( "what", e.what() ));
         shutdown = false;
      } catch( mongocxx::operation_exception& e) {
         elog( "mongo exception, ${desc}, line ${line}, code ${code}, ${details}",
               ("desc", desc)( "line", line_num )( "code", e.code().value() )( "details", e.code().message() ));
         if (e.raw_server_error()) {
            elog( "  raw_server_error: ${e}", ( "e", bsoncxx::to_json(e.raw_server_error()->view())));
         }
      } catch( mongocxx::exception& e) {
         elog( "mongo exception, ${desc}, line ${line}, code ${code}, ${what}",
               ("desc", desc)( "line", line_num )( "code", e.code().value() )( "what", e.what() ));
      } catch( bsoncxx::exception& e) {
         elog( "bsoncxx exception, ${desc}, line ${line}, code ${code}, ${what}",
               ("desc", desc)( "line", line_num )( "code", e.code().value() )( "what", e.what() ));
      } catch( fc::exception& er ) {
         elog( "mongo fc exception, ${desc}, line ${line}, ${details}",
               ("desc", desc)( "line", line_num )( "details", er.to_detail_string()));
      } catch( const std::exception& e ) {
         elog( "mongo std exception, ${desc}, line ${line}, ${what}",
               ("desc", desc)( "line", line_num )( "what", e.what()));
      } catch( ... ) {
         elog( "mongo unknown exception, ${desc}, line ${line_nun}", ("desc", desc)( "line_num", line_num ));
      }
   } catch (...) {
      std::cerr << "Exception attempting to handle exception for " << desc << " " << line_num << std::endl;
   }

   if( shutdown ) {
      // shutdown if mongo failed to provide opportunity to fix issue and restart
      app().quit();
   }
}

void add_data( bsoncxx::builder::basic::document& act_doc, mongocxx::collection& accounts, const chain::action& act, const fc::microseconds& abi_serializer_max_time ) {
   using bsoncxx::builder::basic::kvp;
   using bsoncxx::builder::basic::make_document;
   try {
      if( act.account == chain::config::system_account_name ) {
         if( act.name == mongo_db_plugin_impl::setabi ) {
            auto setabi = act.data_as<chain::setabi>();
            try {
               const abi_def& abi_def = fc::raw::unpack<chain::abi_def>( setabi.abi );
               const string json_str = fc::json::to_string( abi_def );

               act_doc.append(
                     kvp( "data", make_document( kvp( "account", setabi.account.to_string()),
                                                 kvp( "abi_def", bsoncxx::from_json( json_str )))));
               return;
            } catch( bsoncxx::exception& ) {
               // better error handling below
            } catch( fc::exception& e ) {
               ilog( "Unable to convert action abi_def to json for ${n}", ("n", setabi.account.to_string()));
            }
         }
      }

      auto account = find_account( accounts, act.account );
      if( account ) {
         auto from_account = *account;
         abi_def abi;
         if( from_account.view().find( "abi" ) != from_account.view().end()) {
            try {
               abi = fc::json::from_string( bsoncxx::to_json( from_account.view()["abi"].get_document())).as<abi_def>();
            } catch( ... ) {
               ilog( "Unable to convert account abi to abi_def for ${s}::${n}", ("s", act.account)( "n", act.name ));
            }
         }
         string json;
         try {
            abi_serializer abis;
            abis.set_abi( abi, abi_serializer_max_time );
            auto v = abis.binary_to_variant( abis.get_action_type( act.name ), act.data, abi_serializer_max_time );
            json = fc::json::to_string( v );

            const auto& value = bsoncxx::from_json( json );
            act_doc.append( kvp( "data", value ));
            return;
         } catch( bsoncxx::exception& e ) {
            ilog( "Unable to convert UTR JSON to MongoDB JSON: ${e}", ("e", e.what()));
            ilog( "  UTR JSON: ${j}", ("j", json));
            ilog( "  Storing data has hex." );
         }
      }
   } catch( std::exception& e ) {
      ilog( "Unable to convert action.data to ABI: ${s}::${n}, std what: ${e}",
            ("s", act.account)( "n", act.name )( "e", e.what()));
   } catch (fc::exception& e) {
      if (act.name != "onblock") { // ultrainio::onblock not in original ultrainio.system abi
         ilog( "Unable to convert action.data to ABI: ${s}::${n}, fc exception: ${e}",
               ("s", act.account)( "n", act.name )( "e", e.to_detail_string()));
      }
   } catch( ... ) {
      ilog( "Unable to convert action.data to ABI: ${s}::${n}, unknown exception",
            ("s", act.account)( "n", act.name ));
   }
   // if anything went wrong just store raw hex_data
   act_doc.append( kvp( "hex_data", fc::variant( act.data ).as_string()));
}

} // anonymous namespace

void mongo_db_plugin_impl::purge_abi_cache() {
   if( abi_cache_index.size() < abi_cache_size ) return;

   // remove the oldest (smallest) last accessed
   auto& idx = abi_cache_index.get<by_last_access>();
   auto itr = idx.begin();
   if( itr != idx.end() ) {
      idx.erase( itr );
   }
}

optional<abi_serializer> mongo_db_plugin_impl::get_abi_serializer( account_name n ) {
   using bsoncxx::builder::basic::kvp;
   using bsoncxx::builder::basic::make_document;
   if( n.good()) {
      try {

         auto itr = abi_cache_index.find( n );
         if( itr != abi_cache_index.end() ) {
            abi_cache_index.modify( itr, []( auto& entry ) {
               entry.last_accessed = fc::time_point::now();
            });

            return itr->serializer;
         }

         auto account = _accounts.find_one( make_document( kvp("name", n.to_string())) );
         if(account) {
            auto view = account->view();
            abi_def abi;
            if( view.find( "abi" ) != view.end()) {
               try {
                  abi = fc::json::from_string( bsoncxx::to_json( view["abi"].get_document())).as<abi_def>();
               } catch (...) {
                  ilog( "Unable to convert account abi to abi_def for ${n}", ( "n", n ));
                  return optional<abi_serializer>();
               }

               purge_abi_cache(); // make room if necessary
               abi_cache entry;
               entry.account = n;
               entry.last_accessed = fc::time_point::now();
               abi_serializer abis;
               if( n == chain::config::system_account_name ) {
                  // redefine ultrainio setabi.abi from bytes to abi_def
                  // Done so that abi is stored as abi_def in mongo instead of as bytes
                  auto itr = std::find_if( abi.structs.begin(), abi.structs.end(),
                                           []( const auto& s ) { return s.name == "setabi"; } );
                  if( itr != abi.structs.end() ) {
                     auto itr2 = std::find_if( itr->fields.begin(), itr->fields.end(),
                                               []( const auto& f ) { return f.name == "abi"; } );
                     if( itr2 != itr->fields.end() ) {
                        if( itr2->type == "bytes" ) {
                           itr2->type = "abi_def";
                           // unpack setabi.abi as abi_def instead of as bytes
                           abis.add_specialized_unpack_pack( "abi_def",
                                 std::make_pair<abi_serializer::unpack_function, abi_serializer::pack_function>(
                                       []( fc::datastream<const char*>& stream, bool is_array, bool is_optional ) -> fc::variant {
                                          ULTRAIN_ASSERT( !is_array && !is_optional, chain::mongo_db_exception, "unexpected abi_def");
                                          chain::bytes temp;
                                          fc::raw::unpack( stream, temp );
                                          return fc::variant( fc::raw::unpack<abi_def>( temp ) );
                                       },
                                       []( const fc::variant& var, fc::datastream<char*>& ds, bool is_array, bool is_optional ) {
                                          ULTRAIN_ASSERT( false, chain::mongo_db_exception, "never called" );
                                       }
                                 ) );
                        }
                     }
                  }
               }
               abis.set_abi( abi, abi_serializer_max_time );
               entry.serializer.emplace( std::move( abis ) );
               abi_cache_index.insert( entry );
               return entry.serializer;
            }
         }
      } FC_CAPTURE_AND_LOG((n))
   }
   return optional<abi_serializer>();
}

template<typename T>
fc::variant mongo_db_plugin_impl::to_variant_with_abi( const T& obj ) {
   fc::variant pretty_output;
   abi_serializer::to_variant( obj, pretty_output,
                               [&]( account_name n ) { return get_abi_serializer( n ); },
                               abi_serializer_max_time );
   return pretty_output;
}

void mongo_db_plugin_impl::process_accepted_transaction( const transaction_metadata_blocknum& t ) {
   try {
      _process_accepted_transaction( t );
   } catch (fc::exception& e) {
      elog("FC Exception while processing accepted transaction metadata: ${e}", ("e", e.to_detail_string()));
   } catch (std::exception& e) {
      elog("STD Exception while processing accepted tranasction metadata: ${e}", ("e", e.what()));
   } catch (...) {
      elog("Unknown exception while processing accepted transaction metadata");
   }
}

void mongo_db_plugin_impl::process_applied_transaction( const transaction_trace_blocknum& t ) {
   try {
      // always call since we need to capture setabi on accounts even if not storing transaction traces
      _process_applied_transaction( t );
   } catch (fc::exception& e) {
      elog("FC Exception while processing applied transaction trace: ${e}", ("e", e.to_detail_string()));
   } catch (std::exception& e) {
      elog("STD Exception while processing applied transaction trace: ${e}", ("e", e.what()));
   } catch (...) {
      elog("Unknown exception while processing applied transaction trace");
   }
}

void mongo_db_plugin_impl::process_irreversible_block(const chain::block_state_ptr& bs) {
  try {
     _process_irreversible_block( bs );
  } catch (fc::exception& e) {
     elog("FC Exception while processing irreversible block: ${e}", ("e", e.to_detail_string()));
  } catch (std::exception& e) {
     elog("STD Exception while processing irreversible block: ${e}", ("e", e.what()));
  } catch (...) {
     elog("Unknown exception while processing irreversible block");
  }
}

void mongo_db_plugin_impl::process_accepted_block( const chain::block_state_ptr& bs ) {
   try {
      _process_accepted_block( bs );
   } catch (fc::exception& e) {
      elog("FC Exception while processing accepted block trace ${e}", ("e", e.to_string()));
   } catch (std::exception& e) {
      elog("STD Exception while processing accepted block trace ${e}", ("e", e.what()));
   } catch (...) {
      elog("Unknown exception while processing accepted block trace");
   }
}

/*void mongo_db_plugin_impl::_process_accepted_transaction( const transaction_metadata_blocknum& t ) {
   using namespace bsoncxx::types;
   using bsoncxx::builder::basic::kvp;
   using bsoncxx::builder::basic::make_document;
   using bsoncxx::builder::basic::make_array;
   namespace bbb = bsoncxx::builder::basic;

   const auto& trx = t->trx;

   if( !filter_include( trx ) ) return;
   
   auto trans_doc = bsoncxx::builder::basic::document{};

   auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
         std::chrono::microseconds{fc::time_point::now().time_since_epoch().count()} );

   const auto& trx_id = t->id;
   const auto trx_id_str = trx_id.str();

   trans_doc.append( kvp( "trx_id", trx_id_str ) );

   auto v = to_variant_with_abi( trx );
   string trx_json = fc::json::to_string( v );

   try {
      const auto& trx_value = bsoncxx::from_json( trx_json );
      trans_doc.append( bsoncxx::builder::concatenate_doc{trx_value.view()} );
   } catch( bsoncxx::exception& ) {
      try {
         trx_json = fc::prune_invalid_utf8( trx_json );
         const auto& trx_value = bsoncxx::from_json( trx_json );
         trans_doc.append( bsoncxx::builder::concatenate_doc{trx_value.view()} );
         trans_doc.append( kvp( "non-utf8-purged", b_bool{true} ) );
      } catch( bsoncxx::exception& e ) {
         elog( "Unable to convert transaction JSON to MongoDB JSON: ${e}", ("e", e.what()) );
         elog( "  JSON: ${j}", ("j", trx_json) );
      }
   }

   string signing_keys_json;
   if( t->signing_keys.valid() ) {
      signing_keys_json = fc::json::to_string( t->signing_keys->second );
   } else {
      auto signing_keys = trx.get_signature_keys( *chain_id, false, false );
      if( !signing_keys.empty() ) {
         signing_keys_json = fc::json::to_string( signing_keys );
      }
   }

   if( !signing_keys_json.empty() ) {
      try {
         const auto& keys_value = bsoncxx::from_json( signing_keys_json );
         trans_doc.append( kvp( "signing_keys", keys_value ) );
      } catch( bsoncxx::exception& e ) {
         // should never fail, so don't attempt to remove invalid utf8
         elog( "Unable to convert signing keys JSON to MongoDB JSON: ${e}", ("e", e.what()) );
         elog( "  JSON: ${j}", ("j", signing_keys_json) );
      }
   }

   trans_doc.append( kvp( "accepted", b_bool{t->accepted} ) );

   trans_doc.append( kvp( "createdAt", b_date{now} ) );

   try {
      mongocxx::options::update update_opts{};
      update_opts.upsert( true );
      if( !_trans.update_one( make_document( kvp( "trx_id", trx_id_str ) ),
                              make_document( kvp( "$set", trans_doc.view() ) ), update_opts ) ) {
         ULTRAIN_ASSERT( false, chain::mongo_db_insert_fail, "Failed to insert trans ${id}", ("id", trx_id) );
      }
   } catch( ... ) {
      handle_mongo_exception( "trans insert", __LINE__ );
   }

}*/

void mongo_db_plugin_impl::_process_failed_transaction( const chain::packed_transaction_ptr& pack_trx, const string& error_info ) {
    using namespace bsoncxx::types;
    using bsoncxx::builder::basic::kvp;
    using bsoncxx::builder::basic::make_document;
    using bsoncxx::builder::basic::make_array;
    transaction_id_type id = pack_trx->id();
    const auto t = std::make_shared<transaction_metadata>(*pack_trx);
    ilog( "mongodb process_failed_transaction trx-id = ${id} error_info: ${why}",("id", id)("why", error_info));
    const auto& trx = t->trx;
    auto trans_doc = bsoncxx::builder::basic::document{};
    auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::microseconds{block_time.time_since_epoch().count()});

    const auto trx_id = t->id;
    const auto trx_id_str = trx_id.str();
    const chain::transaction_header& trx_header = trx;

    int32_t act_num = 0;
    auto process_action = [&](const std::string& trx_id_str, const chain::action& act, bsoncxx::builder::basic::array& act_array, bool cfa) -> auto {
        auto act_doc = bsoncxx::builder::basic::document();
        act_doc.append( kvp( "action_num", b_int32{act_num} ),
                        kvp( "trx_id", trx_id_str ));
        act_doc.append( kvp( "cfa", b_bool{cfa} ));
        act_doc.append( kvp( "account", act.account.to_string()));
        act_doc.append( kvp( "name", act.name.to_string()));
        act_doc.append( kvp( "authorization", [&act]( bsoncxx::builder::basic::sub_array subarr ) {
            for( const auto& auth : act.authorization ) {
                subarr.append( [&auth]( bsoncxx::builder::basic::sub_document subdoc ) {
                subdoc.append( kvp( "actor", auth.actor.to_string()),
                                kvp( "permission", auth.permission.to_string()));
                } );
            }
        } ));

        try {
            update_account( act );
        } catch (...) {
            ilog( "Unable to update account for ${s}::${n}", ("s", act.account)( "n", act.name ));
        }

        add_data( act_doc, _accounts, act, abi_serializer_max_time );
        act_array.append( act_doc );
        ++act_num;
        return act_num;
    };

    trans_doc.append( kvp( "trx_id", trx_id_str ),
                        kvp( "action_count", b_int32{static_cast<int32_t>(trx.total_actions())} ),
                        kvp( "error_info", error_info ) );

    string signing_keys_json;
    if( t->signing_keys.valid()) {
        signing_keys_json = fc::json::to_string( t->signing_keys->second );
    } else {
#ifdef ULTRAIN_TRX_SUPPORT_GM
        // auto signing_keys = trx.get_signature_keys( *chain_id, false, false );
        flat_set<public_key_type> signing_keys;
        elog("ULTRAIN_TRX_SUPPORT_GM do not support get_signature_keys");
#else
        auto signing_keys = trx.get_signature_keys( *chain_id, false, false );
#endif
        if( !signing_keys.empty()) {
            signing_keys_json = fc::json::to_string( signing_keys );
        }
    }
    string trx_header_json = fc::json::to_string( trx_header );
    try {
        const auto& trx_header_value = bsoncxx::from_json( trx_header_json );
        trans_doc.append( kvp( "transaction_header", trx_header_value ));
    } catch( bsoncxx::exception& ) {
        try {
            trx_header_json = fc::prune_invalid_utf8( trx_header_json );
            const auto& trx_header_value = bsoncxx::from_json( trx_header_json );
            trans_doc.append( kvp( "transaction_header", trx_header_value ));
            trans_doc.append( kvp( "non-utf8-purged", b_bool{true}));
        } catch( bsoncxx::exception& e ) {
            elog( "Unable to convert transaction header JSON to MongoDB JSON: ${e}", ("e", e.what()));
            elog( "  JSON: ${j}", ("j", trx_header_json));
        }
    }
    if( !signing_keys_json.empty()) {
        try {
            const auto& keys_value = bsoncxx::from_json( signing_keys_json );
            trans_doc.append( kvp( "signing_keys", keys_value ));
        } catch( bsoncxx::exception& e ) {
            // should never fail, so don't attempt to remove invalid utf8
            elog( "Unable to convert signing keys JSON to MongoDB JSON: ${e}", ("e", e.what()));
            elog( "  JSON: ${j}", ("j", signing_keys_json));
        }
    }

    if( !trx.actions.empty() && store_actions) {
        bsoncxx::builder::basic::array action_array;
        for( const auto& act : trx.actions ) {
            process_action( trx_id_str, act, action_array, false );
        }
        trans_doc.append( kvp( "actions", action_array ));
    }

    if( !trx.context_free_actions.empty() && store_actions) {
        bsoncxx::builder::basic::array action_array;
        for( const auto& cfa : trx.context_free_actions ) {
            process_action( trx_id_str, cfa, action_array, true );
        }
        trans_doc.append( kvp( "context_free_actions", action_array ));
    }

    string trx_extensions_json = fc::json::to_string( trx.transaction_extensions );
    string trx_signatures_json = fc::json::to_string( trx.signatures );
    string trx_context_free_data_json = fc::json::to_string( trx.context_free_data );

    try {
        if( !trx_extensions_json.empty()) {
            try {
                const auto& trx_extensions_value = bsoncxx::from_json( trx_extensions_json );
                trans_doc.append( kvp( "transaction_extensions", trx_extensions_value ));
            } catch( bsoncxx::exception& ) {
                static_assert( sizeof(std::remove_pointer<decltype(b_binary::bytes)>::type) == sizeof(std::string::value_type), "string type not storable as b_binary" );
                trans_doc.append( kvp( "transaction_extensions",
                                    b_binary{bsoncxx::binary_sub_type::k_binary,
                                                static_cast<uint32_t>(trx_extensions_json.size()),
                                                reinterpret_cast<const u_int8_t*>(trx_extensions_json.data())} ));
            }
        } else {
            trans_doc.append( kvp( "transaction_extensions", make_array()));
        }

        if( !trx_signatures_json.empty()) {
            // signatures contain only utf8
            const auto& trx_signatures_value = bsoncxx::from_json( trx_signatures_json );
            trans_doc.append( kvp( "signatures", trx_signatures_value ));
        } else {
            trans_doc.append( kvp( "signatures", make_array()));
        }

        if( !trx_context_free_data_json.empty()) {
            try {
                const auto& trx_context_free_data_value = bsoncxx::from_json( trx_context_free_data_json );
                trans_doc.append( kvp( "context_free_data", trx_context_free_data_value ));
            } catch( bsoncxx::exception& ) {
                static_assert( sizeof(std::remove_pointer<decltype(b_binary::bytes)>::type) ==
                            sizeof(std::remove_pointer<decltype(trx.context_free_data[0].data())>::type), "context_free_data not storable as b_binary" );
                bsoncxx::builder::basic::array data_array;
                for (auto& cfd : trx.context_free_data) {
                data_array.append(
                        b_binary{bsoncxx::binary_sub_type::k_binary,
                                static_cast<uint32_t>(cfd.size()),
                                reinterpret_cast<const u_int8_t*>(cfd.data())} );
                }
                trans_doc.append( kvp( "context_free_data", data_array.view() ));
            }
        } else {
            trans_doc.append( kvp( "context_free_data", make_array()));
        }
    } catch( std::exception& e ) {
        elog( "Unable to convert transaction JSON to MongoDB JSON: ${e}", ("e", e.what()));
        elog( "  JSON: ${j}", ("j", trx_extensions_json));
        elog( "  JSON: ${j}", ("j", trx_signatures_json));
        elog( "  JSON: ${j}", ("j", trx_context_free_data_json));
    }

    trans_doc.append( kvp( "createdAt", b_date{now} ));

    try {
        if( !_failed_trans.insert_one( trans_doc.view())) {
            ULTRAIN_ASSERT( false, chain::mongo_db_insert_fail, "Failed to insert trans ${id}", ("id", trx_id));
        }
    } catch(...) {
        handle_mongo_exception("_failed_trans insert", __LINE__);
    }
}

void mongo_db_plugin_impl::process_failed_transaction( const chain::packed_transaction_ptr& pack_trx, const string& error_info ) {
   try {
      _process_failed_transaction( pack_trx, error_info );
   } catch (fc::exception& e) {
      elog("FC Exception while processing failed transaction metadata: ${e}", ("e", e.to_detail_string()));
   } catch (std::exception& e) {
      elog("STD Exception while processing failed tranasction metadata: ${e}", ("e", e.what()));
   } catch (...) {
      elog("Unknown exception while processing failed transaction metadata");
   }
}

void mongo_db_plugin_impl::transaction_ack(const std::tuple<const fc::exception_ptr, const chain::transaction_trace_ptr, const chain::packed_transaction_ptr>& results) {
    const auto exception_ptr = std::get<0>(results);
    if ( exception_ptr ) {
        const auto error_code = exception_ptr->code();
        if ( error_code == node_is_syncing::code_value
            || error_code == expired_tx_exception::code_value
            || error_code == tx_duplicate::code_value) {
                return;
        }
        process_failed_transaction( std::get<2>(results), exception_ptr->to_detail_string() );
    }
}

void mongo_db_plugin_impl::_process_accepted_transaction( const transaction_metadata_blocknum& tb ) {
   using namespace bsoncxx::types;
   using bsoncxx::builder::basic::kvp;
   using bsoncxx::builder::basic::make_document;
   using bsoncxx::builder::basic::make_array;
   namespace bbb = bsoncxx::builder::basic;

   const auto  t   = tb.trx_ptr;
   const auto& trx = t->trx;

   if( !filter_include( trx ) ) return;

   auto trans_doc = bsoncxx::builder::basic::document{};

   auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
         std::chrono::microseconds{block_time.time_since_epoch().count()});

   const auto trx_id = t->id;
   const auto trx_id_str = trx_id.str();
   const chain::transaction_header& trx_header = trx;

   bool actions_to_write = false;
   mongocxx::options::bulk_write bulk_opts;
   bulk_opts.ordered(false);
   mongocxx::bulk_write bulk_actions = _actions.create_bulk_write(bulk_opts);

   int32_t act_num = 0;
   auto process_action = [&](const std::string& trx_id_str, const chain::action& act, /*bbb::array& act_array,*/ bool cfa) -> auto {
      auto act_doc = bsoncxx::builder::basic::document();
      act_doc.append( kvp( "action_num", b_int32{act_num} ),
                      kvp( "block_num", b_int32{static_cast<int32_t>(tb.block_num)} ),
                      kvp( "trx_id", trx_id_str ));
      act_doc.append( kvp( "cfa", b_bool{cfa} ));
      act_doc.append( kvp( "account", act.account.to_string()));
      act_doc.append( kvp( "name", act.name.to_string()));
      act_doc.append( kvp( "createdAt", b_date{now} ));
      act_doc.append( kvp( "authorization", [&act]( bsoncxx::builder::basic::sub_array subarr ) {
         for( const auto& auth : act.authorization ) {
            subarr.append( [&auth]( bsoncxx::builder::basic::sub_document subdoc ) {
               subdoc.append( kvp( "actor", auth.actor.to_string()),
                              kvp( "permission", auth.permission.to_string()));
            } );
         }
      } ));

      try {
         update_account( act );
      } catch (...) {
         ilog( "Unable to update account for ${s}::${n}", ("s", act.account)( "n", act.name ));
      }

      add_data( act_doc, _accounts, act, abi_serializer_max_time );
      //act_array.append( act_doc );
      mongocxx::model::insert_one insert_op{act_doc.view()};
      bulk_actions.append( insert_op );
      actions_to_write = true;

      ++act_num;
      return act_num;
   };

   trans_doc.append( kvp( "trx_id", trx_id_str ),
                     kvp( "irreversible", b_bool{false} ),
                     kvp( "action_count", b_int32{static_cast<int32_t>(trx.total_actions())} ) );

   string signing_keys_json;
   if( t->signing_keys.valid()) {
      signing_keys_json = fc::json::to_string( t->signing_keys->second );
   } else {
#ifdef ULTRAIN_TRX_SUPPORT_GM
      // auto signing_keys = trx.get_signature_keys( *chain_id, false, false );
      flat_set<public_key_type> signing_keys;
      elog("ULTRAIN_TRX_SUPPORT_GM do not support get_signature_keys");
#else
      auto signing_keys = trx.get_signature_keys( *chain_id, false, false );
#endif
      if( !signing_keys.empty()) {
         signing_keys_json = fc::json::to_string( signing_keys );
      }
   }
   string trx_header_json = fc::json::to_string( trx_header );
   try {
      const auto& trx_header_value = bsoncxx::from_json( trx_header_json );
      trans_doc.append( kvp( "transaction_header", trx_header_value ));
   } catch( bsoncxx::exception& ) {
      try {
         trx_header_json = fc::prune_invalid_utf8( trx_header_json );
         const auto& trx_header_value = bsoncxx::from_json( trx_header_json );
         trans_doc.append( kvp( "transaction_header", trx_header_value ));
         trans_doc.append( kvp( "non-utf8-purged", b_bool{true}));
      } catch( bsoncxx::exception& e ) {
         elog( "Unable to convert transaction header JSON to MongoDB JSON: ${e}", ("e", e.what()));
         elog( "  JSON: ${j}", ("j", trx_header_json));
      }
   }
   if( !signing_keys_json.empty()) {
      try {
         const auto& keys_value = bsoncxx::from_json( signing_keys_json );
         trans_doc.append( kvp( "signing_keys", keys_value ));
      } catch( bsoncxx::exception& e ) {
         // should never fail, so don't attempt to remove invalid utf8
         elog( "Unable to convert signing keys JSON to MongoDB JSON: ${e}", ("e", e.what()));
         elog( "  JSON: ${j}", ("j", signing_keys_json));
      }
   }

   if( !trx.actions.empty() && store_actions) {
      //bsoncxx::builder::basic::array action_array;
      for( const auto& act : trx.actions ) {
         process_action( trx_id_str, act, false );
      }
      //trans_doc.append( kvp( "actions", action_array ));
   }

   act_num = 0;
   if( !trx.context_free_actions.empty() && store_actions) {
      //bsoncxx::builder::basic::array action_array;
      for( const auto& cfa : trx.context_free_actions ) {
         process_action( trx_id_str, cfa, true );
      }
      //trans_doc.append( kvp( "context_free_actions", action_array ));
   }

   string trx_extensions_json = fc::json::to_string( trx.transaction_extensions );
   string trx_signatures_json = fc::json::to_string( trx.signatures );
   string trx_context_free_data_json = fc::json::to_string( trx.context_free_data );

   try {
      if( !trx_extensions_json.empty()) {
         try {
            const auto& trx_extensions_value = bsoncxx::from_json( trx_extensions_json );
            trans_doc.append( kvp( "transaction_extensions", trx_extensions_value ));
         } catch( bsoncxx::exception& ) {
            static_assert( sizeof(std::remove_pointer<decltype(b_binary::bytes)>::type) == sizeof(std::string::value_type), "string type not storable as b_binary" );
            trans_doc.append( kvp( "transaction_extensions",
                                   b_binary{bsoncxx::binary_sub_type::k_binary,
                                            static_cast<uint32_t>(trx_extensions_json.size()),
                                            reinterpret_cast<const u_int8_t*>(trx_extensions_json.data())} ));
         }
      } else {
         trans_doc.append( kvp( "transaction_extensions", make_array()));
      }


      if( !trx_signatures_json.empty()) {
         // signatures contain only utf8
         const auto& trx_signatures_value = bsoncxx::from_json( trx_signatures_json );
         trans_doc.append( kvp( "signatures", trx_signatures_value ));
      } else {
         trans_doc.append( kvp( "signatures", make_array()));
      }

      if( !trx_context_free_data_json.empty()) {
         try {
            const auto& trx_context_free_data_value = bsoncxx::from_json( trx_context_free_data_json );
            trans_doc.append( kvp( "context_free_data", trx_context_free_data_value ));
         } catch( bsoncxx::exception& ) {
            static_assert( sizeof(std::remove_pointer<decltype(b_binary::bytes)>::type) ==
                           sizeof(std::remove_pointer<decltype(trx.context_free_data[0].data())>::type), "context_free_data not storable as b_binary" );
            bsoncxx::builder::basic::array data_array;
            for (auto& cfd : trx.context_free_data) {
               data_array.append(
                     b_binary{bsoncxx::binary_sub_type::k_binary,
                              static_cast<uint32_t>(cfd.size()),
                              reinterpret_cast<const u_int8_t*>(cfd.data())} );
            }
            trans_doc.append( kvp( "context_free_data", data_array.view() ));
         }
      } else {
         trans_doc.append( kvp( "context_free_data", make_array()));
      }
   } catch( std::exception& e ) {
      elog( "Unable to convert transaction JSON to MongoDB JSON: ${e}", ("e", e.what()));
      elog( "  JSON: ${j}", ("j", trx_extensions_json));
      elog( "  JSON: ${j}", ("j", trx_signatures_json));
      elog( "  JSON: ${j}", ("j", trx_context_free_data_json));
   }

   trans_doc.append( kvp( "block_num", b_int32{static_cast<int32_t>(tb.block_num )}));
   trans_doc.append( kvp( "createdAt", b_date{now} ));

   try {
      if( !_trans.insert_one( trans_doc.view())) {
         ULTRAIN_ASSERT( false, chain::mongo_db_insert_fail, "Failed to insert trans ${id}", ("id", trx_id));
      }
   } catch(...) {
      handle_mongo_exception("trans insert", __LINE__);
   }

   if (actions_to_write) {
      try {
         if( !bulk_actions.execute() ) {
            ULTRAIN_ASSERT( false, chain::mongo_db_insert_fail, "Bulk actions insert failed for transaction: ${id}", ("id", trx_id_str));
         }
      } catch(...) {
         handle_mongo_exception("actions insert", __LINE__);
      }
   }
}

bool
mongo_db_plugin_impl::add_action_trace( mongocxx::bulk_write& bulk_action_traces, const chain::action_trace& atrace,
                                        const transaction_trace_blocknum& tb,
                                        bool executed, const std::chrono::milliseconds& now,
                                        bool& write_ttrace )
{
   using namespace bsoncxx::types;
   using bsoncxx::builder::basic::kvp;

   auto t = tb.trx_trace_ptr;

   if( executed && atrace.receipt.receiver == chain::config::system_account_name ) {
      update_account( atrace.act );
   }

   bool added = false;
   const bool in_filter = (store_action_traces || store_transaction_traces) &&
                    filter_include( atrace.receipt.receiver, atrace.act.name, atrace.act.authorization );
   write_ttrace |= in_filter;
   if( store_action_traces && in_filter ) {
      auto action_traces_doc = bsoncxx::builder::basic::document{};
      const chain::base_action_trace& base = atrace; // without inline action traces

      auto v = to_variant_with_abi( base );
      string json = fc::json::to_string( v );
      try {
         const auto& value = bsoncxx::from_json( json );
         action_traces_doc.append( bsoncxx::builder::concatenate_doc{value.view()} );
      } catch( bsoncxx::exception& ) {
         try {
            json = fc::prune_invalid_utf8( json );
            const auto& value = bsoncxx::from_json( json );
            action_traces_doc.append( bsoncxx::builder::concatenate_doc{value.view()} );
            action_traces_doc.append( kvp( "non-utf8-purged", b_bool{true} ) );
         } catch( bsoncxx::exception& e ) {
            elog( "Unable to convert action trace JSON to MongoDB JSON: ${e}", ("e", e.what()) );
            elog( "  JSON: ${j}", ("j", json) );
         }
      }
      if( t->receipt.valid() ) {
         action_traces_doc.append( kvp( "trx_status", std::string( t->receipt->status ) ) );
      }
      action_traces_doc.append( kvp( "block_num", b_int32{static_cast<int32_t>(tb.block_num)} ) );
      action_traces_doc.append( kvp( "createdAt", b_date{now} ) );

      mongocxx::model::insert_one insert_op{action_traces_doc.view()};
      bulk_action_traces.append( insert_op );
      added = true;
   }

   for( const auto& iline_atrace : atrace.inline_traces ) {
      added |= add_action_trace( bulk_action_traces, iline_atrace, tb, executed, now, write_ttrace );
   }

   return added;
}


void mongo_db_plugin_impl::_process_applied_transaction( const transaction_trace_blocknum& tb ) {
   using namespace bsoncxx::types;
   using bsoncxx::builder::basic::kvp;

   auto t = tb.trx_trace_ptr;
   auto trans_traces_doc = bsoncxx::builder::basic::document{};

   auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
         std::chrono::microseconds{block_time.time_since_epoch().count()});

   mongocxx::options::bulk_write bulk_opts;
   bulk_opts.ordered(false);
   mongocxx::bulk_write bulk_action_traces = _action_traces.create_bulk_write(bulk_opts);
   bool write_atraces = false;
   bool write_ttrace = false; // filters apply to transaction_traces as well
   bool executed = t->receipt.valid() && t->receipt->status == chain::transaction_receipt_header::executed;

   for( const auto& atrace : t->action_traces ) {
      try {
         write_atraces |= add_action_trace( bulk_action_traces, atrace, tb, executed, now, write_ttrace );
      } catch(...) {
         handle_mongo_exception("add action traces", __LINE__);
      }
   }

   // transaction trace insert

   if( store_transaction_traces && write_ttrace ) {
      try {
         auto v = to_variant_with_abi( *t );
         string json = fc::json::to_string( v );
         try {
            const auto& value = bsoncxx::from_json( json );
            trans_traces_doc.append( bsoncxx::builder::concatenate_doc{value.view()} );
         } catch( bsoncxx::exception& ) {
            try {
               json = fc::prune_invalid_utf8( json );
               const auto& value = bsoncxx::from_json( json );
               trans_traces_doc.append( bsoncxx::builder::concatenate_doc{value.view()} );
               trans_traces_doc.append( kvp( "non-utf8-purged", b_bool{true} ) );
            } catch( bsoncxx::exception& e ) {
               elog( "Unable to convert transaction JSON to MongoDB JSON: ${e}", ("e", e.what()) );
               elog( "  JSON: ${j}", ("j", json) );
            }
         }
         trans_traces_doc.append( kvp( "block_num", b_int32{static_cast<int32_t>(tb.block_num)} ) );
         trans_traces_doc.append( kvp( "createdAt", b_date{now} ) );

         try {
            if( !_trans_traces.insert_one( trans_traces_doc.view() ) ) {
               ULTRAIN_ASSERT( false, chain::mongo_db_insert_fail, "Failed to insert trans ${id}", ("id", t->id) );
            }
         } catch( ... ) {
            handle_mongo_exception( "trans_traces insert: " + json, __LINE__ );
         }
      } catch( ... ) {
         handle_mongo_exception( "trans_traces serialization: " + t->id.str(), __LINE__ );
      }
   }

   // insert action_traces
   if( write_atraces ) {
      try {
         if( !bulk_action_traces.execute() ) {
            ULTRAIN_ASSERT( false, chain::mongo_db_insert_fail,
                        "Bulk action traces insert failed for transaction trace: ${id}", ("id", t->id) );
         }
      } catch( ... ) {
         handle_mongo_exception( "action traces insert", __LINE__ );
      }
   }

}

void mongo_db_plugin_impl::_process_accepted_block( const chain::block_state_ptr& bs ) {
   using namespace bsoncxx::types;
   using namespace bsoncxx::builder;
   using bsoncxx::builder::basic::kvp;
   using bsoncxx::builder::basic::make_document;

   block_time = bs->block->timestamp.to_time_point();

   mongocxx::options::update update_opts{};
   update_opts.upsert( true );

   auto block_num = bs->block_num;
   if( block_num % 1000 == 0 )
      ilog( "block_num: ${b}", ("b", block_num) );
   const auto& block_id = bs->id;
   const auto block_id_str = block_id.str();

   auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
         std::chrono::microseconds{block_time.time_since_epoch().count()});

   if( store_block_states ) {
      auto block_state_doc = bsoncxx::builder::basic::document{};
      block_state_doc.append( kvp( "block_num", b_int32{static_cast<int32_t>(block_num)} ),
                              kvp( "block_id", block_id_str ),
                              kvp( "validated", b_bool{bs->validated} ) );

      const chain::block_header_state& bhs = *bs;

      auto json = fc::json::to_string( bhs );
      try {
         const auto& value = bsoncxx::from_json( json );
         block_state_doc.append( kvp( "block_header_state", value ) );
      } catch( bsoncxx::exception& ) {
         try {
            json = fc::prune_invalid_utf8( json );
            const auto& value = bsoncxx::from_json( json );
            block_state_doc.append( kvp( "block_header_state", value ) );
            block_state_doc.append( kvp( "non-utf8-purged", b_bool{true} ) );
         } catch( bsoncxx::exception& e ) {
            elog( "Unable to convert block_header_state JSON to MongoDB JSON: ${e}", ("e", e.what()) );
            elog( "  JSON: ${j}", ("j", json) );
         }
      }
      block_state_doc.append( kvp( "createdAt", b_date{now} ) );

      try {
         if( update_blocks_via_block_num ) {
            if( !_block_states.update_one( make_document( kvp( "block_num", b_int32{static_cast<int32_t>(block_num)} ) ),
                                           make_document( kvp( "$set", block_state_doc.view() ) ), update_opts ) ) {
               ULTRAIN_ASSERT( false, chain::mongo_db_insert_fail, "Failed to insert block_state ${num}", ("num", block_num) );
            }
         } else {
            if( !_block_states.update_one( make_document( kvp( "block_id", block_id_str ) ),
                                           make_document( kvp( "$set", block_state_doc.view() ) ), update_opts ) ) {
               ULTRAIN_ASSERT( false, chain::mongo_db_insert_fail, "Failed to insert block_state ${bid}", ("bid", block_id) );
            }
         }
      } catch( ... ) {
         handle_mongo_exception( "block_states insert: " + json, __LINE__ );
      }
   }

   if( store_blocks ) {
      auto block_doc = bsoncxx::builder::basic::document{};
      block_doc.append( kvp( "block_num", b_int32{static_cast<int32_t>(block_num)} ),
                        kvp( "block_id", block_id_str ) );

      chain::block_db_record bdr(*bs->block);
      ilog("save block, num: ${n}", ("n", block_num));
      auto v = to_variant_with_abi( bdr );

      //auto v = to_variant_with_abi( *bs->block );
      auto json = fc::json::to_string( v );
      try {
         const auto& value = bsoncxx::from_json( json );
         block_doc.append( kvp( "block", value ) );
      } catch( bsoncxx::exception& ) {
         try {
            json = fc::prune_invalid_utf8( json );
            const auto& value = bsoncxx::from_json( json );
            block_doc.append( kvp( "block", value ) );
            block_doc.append( kvp( "non-utf8-purged", b_bool{true} ) );
         } catch( bsoncxx::exception& e ) {
            elog( "Unable to convert block JSON to MongoDB JSON: ${e}", ("e", e.what()) );
            elog( "  JSON: ${j}", ("j", json) );
         }
      }
      block_doc.append( kvp( "createdAt", b_date{now} ) );

      try {
         if( update_blocks_via_block_num ) {
            if( !_blocks.update_one( make_document( kvp( "block_num", b_int32{static_cast<int32_t>(block_num)} ) ),
                                     make_document( kvp( "$set", block_doc.view() ) ), update_opts ) ) {
               ULTRAIN_ASSERT( false, chain::mongo_db_insert_fail, "Failed to insert block ${num}", ("num", block_num) );
            }
         } else {
            if( !_blocks.update_one( make_document( kvp( "block_id", block_id_str ) ),
                                     make_document( kvp( "$set", block_doc.view() ) ), update_opts ) ) {
               ULTRAIN_ASSERT( false, chain::mongo_db_insert_fail, "Failed to insert block ${bid}", ("bid", block_id) );
            }
         }
      } catch( ... ) {
         handle_mongo_exception( "blocks insert: " + json, __LINE__ );
      }
   }
}

void mongo_db_plugin_impl::_process_irreversible_block(const chain::block_state_ptr& bs)
{
   using namespace bsoncxx::types;
   using namespace bsoncxx::builder;
   using bsoncxx::builder::basic::make_document;
   using bsoncxx::builder::basic::kvp;


   const auto block_id = bs->block->id();
   const auto block_id_str = block_id.str();
   ilog("block id: ${b}", ("b", block_id_str));
   auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
         std::chrono::microseconds{block_time.time_since_epoch().count()});

   if( store_blocks ) {
      auto ir_block = find_block( _blocks, block_id_str );
      if( !ir_block ) {
         _process_accepted_block( bs );
         ir_block = find_block( _blocks, block_id_str );
         if( !ir_block ) return; // should never happen
      }

      auto update_doc = make_document( kvp( "$set", make_document( kvp( "irreversible", b_bool{true} ),
                                                                   kvp( "validated", b_bool{bs->validated} ),
                                                                   kvp( "updatedAt", b_date{now} ) ) ) );

      _blocks.update_one( make_document( kvp( "_id", ir_block->view()["_id"].get_oid() ) ), update_doc.view() );
   }

   if( store_block_states ) {
      auto ir_block = find_block( _block_states, block_id_str );
      if( !ir_block ) {
         _process_accepted_block( bs );
         ir_block = find_block( _block_states, block_id_str );
         if( !ir_block ) return; // should never happen
      }

      auto update_doc = make_document( kvp( "$set", make_document( kvp( "irreversible", b_bool{true} ),
                                                                   kvp( "validated", b_bool{bs->validated} ),
                                                                   kvp( "updatedAt", b_date{now} ) ) ) );

      _block_states.update_one( make_document( kvp( "_id", ir_block->view()["_id"].get_oid() ) ), update_doc.view() );
   }

   if( store_transactions ) {
      const auto block_num = bs->block->block_num();
      bool transactions_in_block = false;
      mongocxx::options::bulk_write bulk_opts;
      bulk_opts.ordered( false );
      auto bulk = _trans.create_bulk_write( bulk_opts );

      for( const auto& receipt : bs->block->transactions ) {
         string trx_id_str;
         if( receipt.trx.contains<packed_transaction>() ) {
            const auto& pt = receipt.trx.get<packed_transaction>();
            // get id via get_raw_transaction() as packed_transaction.id() mutates internal transaction state
            const auto& raw = pt.get_raw_transaction();
            const auto& trx = fc::raw::unpack<transaction>( raw );
            if( !filter_include( trx ) ) continue;
            const auto& id = trx.id();
            trx_id_str = id.str();
         } else if(receipt.trx.contains<packed_generated_transaction>()) {
            const auto& pgt = receipt.trx.get<packed_generated_transaction>();
            const auto& trx = pgt.get_transaction();
            if(!filter_include(trx)) continue;
            const auto& id = trx.id();
            trx_id_str = id.str();
         } else {
            const auto& id = receipt.trx.get<transaction_id_type>();
            trx_id_str = id.str();
         }

         auto ir_trans = find_transaction(_trans, trx_id_str);

         if (ir_trans) {
            auto update_doc = make_document( kvp( "$set", make_document( kvp( "irreversible", b_bool{true} ),
                                                                         kvp( "block_id", block_id_str),
                                                                         kvp( "updatedAt", b_date{now}))));

            mongocxx::model::update_one update_op{ make_document(kvp("_id", ir_trans->view()["_id"].get_oid())), update_doc.view()};
            update_op.upsert( false );
            bulk.append(update_op);
            transactions_in_block = true;
         }
      }

      if( transactions_in_block ) {
         try {
            if( !bulk.execute() ) {
               ULTRAIN_ASSERT( false, chain::mongo_db_insert_fail, "Bulk transaction insert failed for block: ${bid}", ("bid", block_id) );
            }
         } catch( ... ) {
            handle_mongo_exception( "bulk transaction insert", __LINE__ );
         }
      }
   }
}

void mongo_db_plugin_impl::add_pub_keys( const vector<chain::key_weight>& keys, const account_name& name,
                                         const permission_name& permission, const std::chrono::milliseconds& now )
{
   using bsoncxx::builder::basic::kvp;
   using bsoncxx::builder::basic::make_document;
   using namespace bsoncxx::types;

   if( keys.empty()) return;

   mongocxx::bulk_write bulk = _pub_keys.create_bulk_write();

   for( const auto& pub_key_weight : keys ) {
      auto find_doc = bsoncxx::builder::basic::document();

      find_doc.append( kvp( "account", name.to_string()),
                       kvp( "public_key", pub_key_weight.key.operator string()),
                       kvp( "permission", permission.to_string()) );

      auto update_doc = make_document( kvp( "$set", make_document( bsoncxx::builder::concatenate_doc{find_doc.view()},
                                                                   kvp( "createdAt", b_date{now} ))));

      mongocxx::model::update_one insert_op{find_doc.view(), update_doc.view()};
      insert_op.upsert(true);
      bulk.append( insert_op );
   }

   try {
      if( !bulk.execute()) {
         ULTRAIN_ASSERT( false, chain::mongo_db_insert_fail,
                     "Bulk pub_keys insert failed for account: ${a}, permission: ${p}",
                     ("a", name)( "p", permission ));
      }
   } catch (...) {
      handle_mongo_exception( "pub_keys insert", __LINE__ );
   }
}

void mongo_db_plugin_impl::remove_pub_keys( const account_name& name, const permission_name& permission )
{
   using bsoncxx::builder::basic::kvp;
   using bsoncxx::builder::basic::make_document;

   try {
      auto result = _pub_keys.delete_many( make_document( kvp( "account", name.to_string()),
                                                         kvp( "permission", permission.to_string())));
      if( !result ) {
         ULTRAIN_ASSERT( false, chain::mongo_db_update_fail,
                     "pub_keys delete failed for account: ${a}, permission: ${p}",
                     ("a", name)( "p", permission ));
      }
   } catch (...) {
      handle_mongo_exception( "pub_keys delete", __LINE__ );
   }
}

void mongo_db_plugin_impl::add_account_control( const vector<chain::permission_level_weight>& controlling_accounts,
                                                const account_name& name, const permission_name& permission,
                                                const std::chrono::milliseconds& now )
{
   using bsoncxx::builder::basic::kvp;
   using bsoncxx::builder::basic::make_document;
   using namespace bsoncxx::types;

   if( controlling_accounts.empty()) return;

   mongocxx::bulk_write bulk = _account_controls.create_bulk_write();

   for( const auto& controlling_account : controlling_accounts ) {
      auto find_doc = bsoncxx::builder::basic::document();

      find_doc.append( kvp( "controlled_account", name.to_string()),
                       kvp( "controlled_permission", permission.to_string()),
                       kvp( "controlling_account", controlling_account.permission.actor.to_string()) );

      auto update_doc = make_document( kvp( "$set", make_document( bsoncxx::builder::concatenate_doc{find_doc.view()},
                                                                   kvp( "createdAt", b_date{now} ))));


      mongocxx::model::update_one insert_op{find_doc.view(), update_doc.view()};
      insert_op.upsert(true);
      bulk.append( insert_op );
   }

   try {
      if( !bulk.execute()) {
         ULTRAIN_ASSERT( false, chain::mongo_db_insert_fail,
                     "Bulk account_controls insert failed for account: ${a}, permission: ${p}",
                     ("a", name)( "p", permission ));
      }
   } catch (...) {
      handle_mongo_exception( "account_controls insert", __LINE__ );
   }
}

void mongo_db_plugin_impl::remove_account_control( const account_name& name, const permission_name& permission )
{
   using bsoncxx::builder::basic::kvp;
   using bsoncxx::builder::basic::make_document;

   try {
      auto result = _account_controls.delete_many( make_document( kvp( "controlled_account", name.to_string()),
                                                                  kvp( "controlled_permission", permission.to_string())));
      if( !result ) {
         ULTRAIN_ASSERT( false, chain::mongo_db_update_fail,
                     "account_controls delete failed for account: ${a}, permission: ${p}",
                     ("a", name)( "p", permission ));
      }
   } catch (...) {
      handle_mongo_exception( "account_controls delete", __LINE__ );
   }
}

namespace {

void create_account( mongocxx::collection& accounts, const name& name, std::chrono::milliseconds& now ) {
   using namespace bsoncxx::types;
   using bsoncxx::builder::basic::kvp;
   using bsoncxx::builder::basic::make_document;

   mongocxx::options::update update_opts{};
   update_opts.upsert( true );

   const string name_str = name.to_string();
   auto update = make_document(
         kvp( "$set", make_document( kvp( "name", name_str),
                                     kvp( "createdAt", b_date{now} ))));
   try {
      if( !accounts.update_one( make_document( kvp( "name", name_str )), update.view(), update_opts )) {
         ULTRAIN_ASSERT( false, chain::mongo_db_update_fail, "Failed to insert account ${n}", ("n", name));
      }
   } catch (...) {
      handle_mongo_exception( "create_account", __LINE__ );
   }
}

}

void mongo_db_plugin_impl::update_account(const chain::action& act)
{
   using bsoncxx::builder::basic::kvp;
   using bsoncxx::builder::basic::make_document;
   using namespace bsoncxx::types;

   if (act.account != chain::config::system_account_name)
      return;

   try {
      if( act.name == newaccount ) {
         std::chrono::milliseconds now = std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::microseconds{block_time.time_since_epoch().count()} );
         auto newacc = act.data_as<chain::newaccount>();

         create_account( _accounts, newacc.name, now );

         add_pub_keys( newacc.owner.keys, newacc.name, owner, now );
         add_account_control( newacc.owner.accounts, newacc.name, owner, now );
         add_pub_keys( newacc.active.keys, newacc.name, active, now );
         add_account_control( newacc.active.accounts, newacc.name, active, now );

      } else if( act.name == updateauth ) {
         auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::microseconds{block_time.time_since_epoch().count()} );
         const auto update = act.data_as<chain::updateauth>();
         remove_pub_keys(update.account, update.permission);
         remove_account_control(update.account, update.permission);
         add_pub_keys(update.auth.keys, update.account, update.permission, now);
         add_account_control(update.auth.accounts, update.account, update.permission, now);

      } else if( act.name == deleteauth ) {
         const auto del = act.data_as<chain::deleteauth>();
         remove_pub_keys( del.account, del.permission );
         remove_account_control(del.account, del.permission);

      } else if( act.name == setabi ) {
         auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::microseconds{block_time.time_since_epoch().count()} );
         auto setabi = act.data_as<chain::setabi>();

         abi_cache_index.erase( setabi.account );

         auto account = find_account( _accounts, setabi.account );
         if( !account ) {
            create_account( _accounts, setabi.account, now );
            account = find_account( _accounts, setabi.account );
         }
         if( account ) {
            abi_def abi_def = fc::raw::unpack<chain::abi_def>( setabi.abi );
            const string json_str = fc::json::to_string( abi_def );

            try{
               auto update_from = make_document(
                     kvp( "$set", make_document( kvp( "abi", bsoncxx::from_json( json_str )),
                                                 kvp( "updatedAt", b_date{now} ))));

               try {
                  if( !_accounts.update_one( make_document( kvp( "_id", account->view()["_id"].get_oid())),
                                             update_from.view())) {
                     ULTRAIN_ASSERT( false, chain::mongo_db_update_fail, "Failed to udpdate account ${n}", ("n", setabi.account));
                  }
               } catch( ... ) {
                  handle_mongo_exception( "account update", __LINE__ );
               }
            } catch( bsoncxx::exception& e ) {
               elog( "Unable to convert abi JSON to MongoDB JSON: ${e}", ("e", e.what()));
               elog( "  JSON: ${j}", ("j", json_str));
            }
         }
      }
   } catch( fc::exception& e ) {
      // if unable to unpack native type, skip account creation
   }
}

mongo_db_plugin_impl::mongo_db_plugin_impl()
{
}

mongo_db_plugin_impl::~mongo_db_plugin_impl() {
   if (!startup) {
      try {
         ilog( "mongo_db_plugin shutdown in process please be patient this can take a few minutes" );
         done = true;
         condition.notify_one();

         consume_thread.join();

         mongo_pool.reset();
      } catch( std::exception& e ) {
         elog( "Exception on mongo_db_plugin shutdown of consume thread: ${e}", ("e", e.what()));
      }
   }
}

void mongo_db_plugin_impl::wipe_database() {
   ilog("mongo db wipe_database");

   auto client = mongo_pool->acquire();
   auto& mongo_conn = *client;

   auto block_states = mongo_conn[db_name][block_states_col];
   auto blocks = mongo_conn[db_name][blocks_col];
   auto trans = mongo_conn[db_name][trans_col];
   auto trans_traces = mongo_conn[db_name][trans_traces_col];
   auto actions = mongo_conn[db_name][actions_col];
   auto action_traces = mongo_conn[db_name][action_traces_col];
   auto accounts = mongo_conn[db_name][accounts_col];
   auto pub_keys = mongo_conn[db_name][pub_keys_col];
   auto account_controls = mongo_conn[db_name][account_controls_col];

   block_states.drop();
   blocks.drop();
   trans.drop();
   trans_traces.drop();
   actions.drop();
   action_traces.drop();
   accounts.drop();
   pub_keys.drop();
   account_controls.drop();
   ilog("done wipe_database");
}

void mongo_db_plugin_impl::init() {
   using namespace bsoncxx::types;
   using bsoncxx::builder::basic::make_document;
   using bsoncxx::builder::basic::kvp;
   // Create the native contract accounts manually; sadly, we can't run their contracts to make them create themselves
   // See native_contract_chain_initializer::prepare_database()

   ilog("init mongo");
   try {
      auto client = mongo_pool->acquire();
      auto& mongo_conn = *client;

      auto accounts = mongo_conn[db_name][accounts_col];
      if( accounts.count( make_document()) == 0 ) {
         auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::microseconds{fc::time_point::now().time_since_epoch().count()} );

         auto doc = make_document( kvp( "name", name( chain::config::system_account_name ).to_string()),
                                   kvp( "createdAt", b_date{now} ));

         try {
            if( !accounts.insert_one( doc.view())) {
               ULTRAIN_ASSERT( false, chain::mongo_db_insert_fail, "Failed to insert account ${n}",
                           ("n", name( chain::config::system_account_name ).to_string()));
            }
         } catch (...) {
            handle_mongo_exception( "account insert", __LINE__ );
         }

         try {
            // blocks indexes
            auto blocks = mongo_conn[db_name][blocks_col];
            blocks.create_index( bsoncxx::from_json( R"xxx({ "block_num" : 1 })xxx" ));
            blocks.create_index( bsoncxx::from_json( R"xxx({ "block_id" : 1 })xxx" ));

            auto block_states = mongo_conn[db_name][block_states_col];
            block_states.create_index( bsoncxx::from_json( R"xxx({ "block_num" : 1 })xxx" ));
            block_states.create_index( bsoncxx::from_json( R"xxx({ "block_id" : 1 })xxx" ));

            // accounts indexes
            accounts.create_index( bsoncxx::from_json( R"xxx({ "name" : 1 })xxx" ));

            // transactions indexes
            auto trans = mongo_conn[db_name][trans_col];
            trans.create_index( bsoncxx::from_json( R"xxx({ "trx_id" : 1 })xxx" ));
            trans.create_index( bsoncxx::from_json( R"xxx({ "block_num" : 1 })xxx" ));

            auto trans_trace = mongo_conn[db_name][trans_traces_col];
            trans_trace.create_index( bsoncxx::from_json( R"xxx({ "id" : 1 })xxx" ));
            trans_trace.create_index( bsoncxx::from_json( R"xxx({ "block_num" : 1 })xxx" ));

            // actions indexes
            auto actions = mongo_conn[db_name][actions_col];
            actions.create_index( bsoncxx::from_json( R"xxx({ "block_num" : 1 })xxx" ));

            // action traces indexes
            auto action_traces = mongo_conn[db_name][action_traces_col];
            action_traces.create_index( bsoncxx::from_json( R"xxx({ "trx_id" : 1 })xxx" ));
            action_traces.create_index( bsoncxx::from_json( R"xxx({ "block_num" : 1 })xxx" ));

            // pub_keys indexes
            auto pub_keys = mongo_conn[db_name][pub_keys_col];
            pub_keys.create_index( bsoncxx::from_json( R"xxx({ "account" : 1, "permission" : 1 })xxx" ));
            pub_keys.create_index( bsoncxx::from_json( R"xxx({ "public_key" : 1 })xxx" ));

            // account_controls indexes
            auto account_controls = mongo_conn[db_name][account_controls_col];
            account_controls.create_index(
                  bsoncxx::from_json( R"xxx({ "controlled_account" : 1, "controlled_permission" : 1 })xxx" ));
            account_controls.create_index( bsoncxx::from_json( R"xxx({ "controlling_account" : 1 })xxx" ));

         } catch (...) {
            handle_mongo_exception( "create indexes", __LINE__ );
         }
      }
   } catch (...) {
      handle_mongo_exception( "mongo init", __LINE__ );
   }

   ilog("starting db plugin thread");

   consume_thread = boost::thread([this] { consume_blocks(); });

   startup = false;
}

////////////
// mongo_db_plugin
////////////

mongo_db_plugin::mongo_db_plugin()
:my(new mongo_db_plugin_impl)
{
}

mongo_db_plugin::~mongo_db_plugin()
{
}

void mongo_db_plugin::set_program_options(options_description& cli, options_description& cfg)
{
   cfg.add_options()
         ("mongodb-queue-size,q", bpo::value<uint32_t>()->default_value(1024),
         "The target queue size between nodultrain and MongoDB plugin thread.")
         ("mongodb-abi-cache-size", bpo::value<uint32_t>()->default_value(2048),
          "The maximum size of the abi cache for serializing data.")
         ("mongodb-wipe", bpo::bool_switch()->default_value(false),
         "Required with --replay-blockchain, --hard-replay-blockchain, or --delete-all-blocks to wipe mongo db."
         "This option required to prevent accidental wipe of mongo db.")
         ("mongodb-block-start", bpo::value<uint32_t>()->default_value(0),
         "If specified then only abi data pushed to mongodb until specified block is reached.")
         ("mongodb-uri,m", bpo::value<std::string>(),
         "MongoDB URI connection string, see: https://docs.mongodb.com/master/reference/connection-string/."
               " If not specified then plugin is disabled. Default database 'ULTRAIN' is used if not specified in URI."
               " Example: mongodb://127.0.0.1:27017/UTR")
         ("mongodb-update-via-block-num", bpo::value<bool>()->default_value(false),
          "Update blocks/block_state with latest via block number so that duplicates are overwritten.")
         ("mongodb-store-blocks", bpo::value<bool>()->default_value(true),
          "Enables storing blocks in mongodb.")
         ("mongodb-store-block-states", bpo::value<bool>()->default_value(true),
          "Enables storing block state in mongodb.")
         ("mongodb-store-transactions", bpo::value<bool>()->default_value(true),
          "Enables storing transactions in mongodb.")
         ("mongodb-store-transaction-traces", bpo::value<bool>()->default_value(true),
          "Enables storing transaction traces in mongodb.")
         ("mongodb-store-actions", bpo::value<bool>()->default_value(true),
          "Enables storing actions in mongodb.")
         ("mongodb-store-action-traces", bpo::value<bool>()->default_value(true),
          "Enables storing action traces in mongodb.")
         ("mongodb-filter-on", bpo::value<vector<string>>()->composing(),
          "Track actions which match receiver:action:actor. Receiver, Action, & Actor may be blank to include all. i.e. ultrainio:: or :transfer:  Use * or leave unspecified to include all.")
         ("mongodb-filter-out", bpo::value<vector<string>>()->composing(),
          "Do not track actions which match receiver:action:actor. Receiver, Action, & Actor may be blank to exclude all.")
         ;
}

void mongo_db_plugin::plugin_initialize(const variables_map& options)
{
   try {
      if( options.count( "mongodb-uri" )) {
         ilog( "initializing mongo_db_plugin" );
         my->configured = true;

         if( options.at( "replay-blockchain" ).as<bool>() || options.at( "hard-replay-blockchain" ).as<bool>() || options.at( "delete-all-blocks" ).as<bool>() ) {
            if( options.at( "mongodb-wipe" ).as<bool>()) {
               ilog( "Wiping mongo database on startup" );
               my->wipe_database_on_startup = true;
            } else if( options.count( "mongodb-block-start" ) == 0 ) {
               ULTRAIN_ASSERT( false, chain::plugin_config_exception, "--mongodb-wipe required with --replay-blockchain, --hard-replay-blockchain, or --delete-all-blocks"
                                 " --mongodb-wipe will remove all ULTRAIN collections from mongodb." );
            }
         }

         if( options.count( "abi-serializer-max-time-ms") == 0 ) {
            ULTRAIN_ASSERT(false, chain::plugin_config_exception, "--abi-serializer-max-time-ms required as default value not appropriate for parsing full blocks");
         }
         my->abi_serializer_max_time = app().get_plugin<chain_plugin>().get_abi_serializer_max_time();

         if( options.count( "mongodb-queue-size" )) {
            my->max_queue_size = options.at( "mongodb-queue-size" ).as<uint32_t>();
         }
         if( options.count( "mongodb-abi-cache-size" )) {
            my->abi_cache_size = options.at( "mongodb-abi-cache-size" ).as<uint32_t>();
            ULTRAIN_ASSERT( my->abi_cache_size > 0, chain::plugin_config_exception, "mongodb-abi-cache-size > 0 required" );
         }
         if( options.count( "mongodb-block-start" )) {
            my->start_block_num = options.at( "mongodb-block-start" ).as<uint32_t>();
         }
         if( options.count( "mongodb-update-via-block-num" )) {
            my->update_blocks_via_block_num = options.at( "mongodb-update-via-block-num" ).as<bool>();
         }
         if( options.count( "mongodb-store-blocks" )) {
            my->store_blocks = options.at( "mongodb-store-blocks" ).as<bool>();
         }
         if( options.count( "mongodb-store-block-states" )) {
            my->store_block_states = options.at( "mongodb-store-block-states" ).as<bool>();
         }
         if( options.count( "mongodb-store-transactions" )) {
            my->store_transactions = options.at( "mongodb-store-transactions" ).as<bool>();
         }
         if( options.count( "mongodb-store-transaction-traces" )) {
            my->store_transaction_traces = options.at( "mongodb-store-transaction-traces" ).as<bool>();
         }
	 if( options.count( "mongodb-store-actions" )) {
            my->store_actions = options.at( "mongodb-store-actions" ).as<bool>();
         }
         if( options.count( "mongodb-store-action-traces" )) {
            my->store_action_traces = options.at( "mongodb-store-action-traces" ).as<bool>();
         }
         if( options.count( "mongodb-filter-on" )) {
            auto fo = options.at( "mongodb-filter-on" ).as<vector<string>>();
            my->filter_on_star = false;
            for( auto& s : fo ) {
               if( s == "*" ) {
                  my->filter_on_star = true;
                  break;
               }
               std::vector<std::string> v;
               boost::split( v, s, boost::is_any_of( ":" ));
               ULTRAIN_ASSERT( v.size() == 3, fc::invalid_arg_exception, "Invalid value ${s} for --mongodb-filter-on", ("s", s));
               filter_entry fe{v[0], v[1], v[2]};
               my->filter_on.insert( fe );
            }
         } else {
            my->filter_on_star = true;
         }
         if( options.count( "mongodb-filter-out" )) {
            auto fo = options.at( "mongodb-filter-out" ).as<vector<string>>();
            for( auto& s : fo ) {
               std::vector<std::string> v;
               boost::split( v, s, boost::is_any_of( ":" ));
               ULTRAIN_ASSERT( v.size() == 3, fc::invalid_arg_exception, "Invalid value ${s} for --mongodb-filter-out", ("s", s));
               filter_entry fe{v[0], v[1], v[2]};
               my->filter_out.insert( fe );
            }
         }
         if( options.count( "producer-name") ) {
            wlog( "mongodb plugin not recommended on producer node" );
            my->is_producer = true;
         }

         std::string uri_str = options.at( "mongodb-uri" ).as<std::string>();
         ilog( "connecting to ${u}", ("u", uri_str));
         mongocxx::uri uri = mongocxx::uri{uri_str};
         my->db_name = uri.database();
         if( my->db_name.empty())
            my->db_name = "UTR";
         my->mongo_pool.emplace(uri);

         // hook up to signals on controller
         chain_plugin* chain_plug = app().find_plugin<chain_plugin>();
         ULTRAIN_ASSERT( chain_plug, chain::missing_chain_plugin_exception, ""  );
         auto& chain = chain_plug->chain();
         my->chain_id.emplace( chain.get_chain_id());

         my->accepted_block_header_connection.emplace(
               chain.accepted_block_header.connect( [&]( const chain::block_state_ptr& bs ) {
                  my->current_block_num = bs->block_num;
               } ));
         my->accepted_block_connection.emplace(
               chain.accepted_block.connect( [&]( const chain::block_state_ptr& bs ) {
                  my->accepted_block( bs );
               } ));
         my->irreversible_block_connection.emplace(
               chain.irreversible_block.connect( [&]( const chain::block_state_ptr& bs ) {
                  my->applied_irreversible_block( bs );
               } ));
         my->accepted_transaction_connection.emplace(
               chain.accepted_transaction.connect( [&]( const chain::transaction_metadata_ptr& t ) {
                  my->accepted_transaction( t );
               } ));
         my->applied_transaction_connection.emplace(
               chain.applied_transaction.connect( [&]( const chain::transaction_trace_ptr& t ) {
                  my->applied_transaction( t );
               } ));

         if( my->wipe_database_on_startup ) {
            my->wipe_database();
         }
         my->init();
      } else {
         wlog( "ultrainio::mongo_db_plugin configured, but no --mongodb-uri specified." );
         wlog( "mongo_db_plugin disabled." );
      }
   } FC_LOG_AND_RETHROW()
}

void mongo_db_plugin::plugin_startup()
{
   //my->incoming_transaction_ack_subscription = app().get_channel<channels::transaction_ack>().subscribe(boost::bind(&mongo_db_plugin_impl::transaction_ack, my.get(), _1));

}

void mongo_db_plugin::plugin_shutdown()
{
   my->accepted_block_header_connection.reset();
   my->accepted_block_connection.reset();
   my->irreversible_block_connection.reset();
   my->accepted_transaction_connection.reset();
   my->applied_transaction_connection.reset();

   my.reset();
}

} // namespace ultrainio
