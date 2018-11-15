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

#include <ultrainio/chain/ultrainio_contract.hpp>

#include <ultrainio/utilities/key_conversion.hpp>
#include <ultrainio/utilities/common.hpp>
#include <ultrainio/chain/wast_to_wasm.hpp>

#include <boost/signals2/connection.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include <fc/io/json.hpp>
#include <fc/variant.hpp>
#include <signal.h>

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
         ("checkpoint", bpo::value<vector<string>>()->composing(), "Pairs of [BLOCK_NUM,BLOCK_ID] that should be enforced as checkpoints.")
         ("wasm-runtime", bpo::value<ultrainio::chain::wasm_interface::vm_type>()->value_name("wavm/binaryen"), "Override default WASM runtime")
         ("abi-serializer-max-time-ms", bpo::value<uint32_t>()->default_value(config::default_abi_serializer_max_time_ms),
          "Override default maximum ABI serialization time allowed in ms")
         ("chain-state-db-size-mb", bpo::value<uint64_t>()->default_value(config::default_state_size / (1024  * 1024)), "Maximum size (in MiB) of the chain state database")
         ("chain-state-db-guard-size-mb", bpo::value<uint64_t>()->default_value(config::default_state_guard_size / (1024  * 1024)), "Safely shut down node when free space remaining in the chain state database drops below this size (in MiB).")
         ("contracts-console", bpo::bool_switch()->default_value(false),
          "print contract's output to console")
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

   try {
      try {
         genesis_state gs; // Check if ULTRAINIO_ROOT_KEY is bad
      } catch ( const fc::exception& ) {
         elog( "ULTRAINIO_ROOT_KEY ('${root_key}') is invalid. Recompile with a valid public key.",
               ("root_key", genesis_state::ultrainio_root_key));
         throw;
      }

      my->chain_config = controller::config();

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
      } else {
         wlog( "Starting up fresh blockchain with default genesis state." );
      }

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

   } FC_LOG_AND_RETHROW()

}

void chain_plugin::plugin_startup()
{ try {
   try {
      my->chain->startup();
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

void chain_plugin::accept_transaction(const chain::packed_transaction& trx, next_function<chain::transaction_trace_ptr> next) {
   my->incoming_transaction_async_method(std::make_shared<packed_transaction>(trx), false, std::forward<decltype(next)>(next));
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
   app().quit();
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
      rm.get_block_net_limit()
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

// TODO: move this and similar functions to a header. Copied from wasm_interface.cpp.
// TODO: fix strict aliasing violation
static float64_t to_softfloat64( double d ) {
   return *reinterpret_cast<float64_t*>(&d);
}

static fc::variant get_global_row( const database& db, const abi_def& abi, const abi_serializer& abis, const fc::microseconds& abi_serializer_max_time_ms ) {
   const auto table_type = get_table_type(abi, N(global));
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

read_only::get_producers_result read_only::get_producers( const read_only::get_producers_params& p , bool filter_enabled) const {
   const abi_def abi = ultrainio::chain_apis::get_abi(db, N(ultrainio));
   const auto table_type = get_table_type(abi, N(producers));
   const abi_serializer abis{ abi, abi_serializer_max_time };
   ULTRAIN_ASSERT(table_type == KEYi64, chain::contract_table_query_exception, "Invalid table type ${type} for table producers", ("type",table_type));

   const auto& d = db.db();
   const auto lower = name{p.lower_bound};

   static const uint8_t secondary_index_num = 0;
   const auto* const table_id = d.find<chain::table_id_object, chain::by_code_scope_table>(boost::make_tuple(N(ultrainio), N(ultrainio), N(producers)));
   const auto* const secondary_table_id = d.find<chain::table_id_object, chain::by_code_scope_table>(boost::make_tuple(N(ultrainio), N(ultrainio), N(producers) | secondary_index_num));
   ULTRAIN_ASSERT(table_id && secondary_table_id, chain::contract_table_query_exception, "Missing producers table");

   const auto& kv_index = d.get_index<key_value_index, by_scope_primary>();
   const auto& secondary_index = d.get_index<index_double_index>().indices();
   const auto& secondary_index_by_primary = secondary_index.get<by_primary>();
   const auto& secondary_index_by_secondary = secondary_index.get<by_secondary>();

   read_only::get_producers_result result;
   const auto stopTime = fc::time_point::now() + fc::microseconds(1000 * 10); // 10ms
   vector<char> data;

   auto it = [&]{
      if(lower.value == 0)
         return secondary_index_by_secondary.lower_bound(
            boost::make_tuple(secondary_table_id->id, to_softfloat64(std::numeric_limits<double>::lowest()), 0));
      else
         return secondary_index.project<by_secondary>(
            secondary_index_by_primary.lower_bound(
               boost::make_tuple(secondary_table_id->id, lower.value)));
   }();
   it = secondary_index_by_secondary.begin();

   for( ; it != secondary_index_by_secondary.end() && it->t_id == secondary_table_id->id; ++it ) {
      //TODO add an option to make the stop time and size limit valid according to requirement
#if 0
      if (result.rows.size() >= p.limit || fc::time_point::now() > stopTime) {
         result.more = name{it->primary_key}.to_string();
         break;
      }
#endif
      copy_inline_row(*kv_index.find(boost::make_tuple(table_id->id, it->primary_key)), data);
      auto producer = abis.binary_to_variant(abis.get_table_type(N(producers)), data, abi_serializer_max_time);
      if(filter_enabled && !(producer["is_enabled"].as_bool())) {
          continue;
      }
      if (p.json)
         result.rows.emplace_back(abis.binary_to_variant(abis.get_table_type(N(producers)), data, abi_serializer_max_time));
      else
         result.rows.emplace_back(fc::variant(data));
   }
   auto gstate = get_global_row(d, abi, abis, abi_serializer_max_time);
   ilog("global ${gl}", ("gl", gstate));

   result.thresh_activated_stake_time = gstate["thresh_activated_stake_time"].as_double();
   result.min_stake_thresh = gstate["min_activated_stake"].as_double()/gstate["min_committee_member"].as_double();
   result.min_committee_member_number = gstate["min_committee_member_number"].as_double();
   return result;
}

bool read_only::is_genesis_finished() const{
    static bool genesis_finished {};
    if (!genesis_finished){
        try {
            get_producers_params params;
            params.json=true;
            params.lower_bound="";
            auto result = get_producers(params,true);
            genesis_finished = result.thresh_activated_stake_time && result.rows.size()>result.min_committee_member_number? true: false;
        }
        catch (fc::exception& e) {
            ilog("there may be no producer registered: ${e}", ("e", e.to_string()));
        }
    }
    return genesis_finished;
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
           ("ref_block_prefix", ref_block_prefix);
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

      app().get_method<incoming::methods::transaction_async>()(pretty_input, true, [this, next](const fc::static_variant<fc::exception_ptr, transaction_trace_ptr>& result) -> void{
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

      t_id = d.find<chain::table_id_object, chain::by_code_scope_table>(boost::make_tuple( config::system_account_name, params.account_name, N(userres) ));
      if (t_id != nullptr) {
         const auto &idx = d.get_index<key_value_index, by_scope_primary>();
         auto it = idx.find(boost::make_tuple( t_id->id, params.account_name ));
         if ( it != idx.end() ) {
            vector<char> data;
            copy_inline_row(*it, data);
            result.total_resources = abis.binary_to_variant( "user_resources", data, abi_serializer_max_time );
         }
      }

      t_id = d.find<chain::table_id_object, chain::by_code_scope_table>(boost::make_tuple( config::system_account_name, params.account_name, N(delband) ));
      if (t_id != nullptr) {
         const auto &idx = d.get_index<key_value_index, by_scope_primary>();
         auto it = idx.find(boost::make_tuple( t_id->id, params.account_name ));
         if ( it != idx.end() ) {
            vector<char> data;
            copy_inline_row(*it, data);
            result.self_delegated_bandwidth = abis.binary_to_variant( "delegated_bandwidth", data, abi_serializer_max_time );
         }
      }

      t_id = d.find<chain::table_id_object, chain::by_code_scope_table>(boost::make_tuple( config::system_account_name, params.account_name, N(refunds) ));
      if (t_id != nullptr) {
         const auto &idx = d.get_index<key_value_index, by_scope_primary>();
         auto it = idx.find(boost::make_tuple( t_id->id, params.account_name ));
         if ( it != idx.end() ) {
            vector<char> data;
            copy_inline_row(*it, data);
            result.refund_request = abis.binary_to_variant( "refund_request", data, abi_serializer_max_time );
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

      t_id = d.find<chain::table_id_object, chain::by_code_scope_table>(boost::make_tuple( config::system_account_name, config::system_account_name, N(producers) ));
      if (t_id != nullptr) {
         const auto &idx = d.get_index<key_value_index, by_scope_primary>();
         auto it = idx.find(boost::make_tuple( t_id->id, params.account_name ));
         if ( it != idx.end() ) {
            vector<char> data;
            copy_inline_row(*it, data);
            result.producer_info = abis.binary_to_variant( "producer_info", data, abi_serializer_max_time );
         }
      }
   }
   return result;
}
read_only::get_sourcerate_results read_only::get_sourcerate( const get_sourcerate_params& params)const {
      get_sourcerate_results result;
      bool grelisted = db.is_resource_greylisted(params.account_name);

      const auto& rm = db.get_resource_limits_manager();
      result.cpu_rate = rm.get_cpu_rate(!grelisted);
      result.net_rate = rm.get_net_rate(!grelisted);
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
