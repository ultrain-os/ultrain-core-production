/**
 *  @file
 *  @copyright defined in ultrain/LICENSE.txt
 *  @defgroup ultrainclienttool ULTRAINIO Command Line Client Reference
 *  @brief Tool for sending transactions and querying state from @ref nodultrain
 *  @ingroup ultrainclienttool
 */

/**
  @defgroup ultrainclienttool

  @section intro Introduction to clultrain

  `clultrain` is a command line tool that interfaces with the REST api exposed by @ref nodultrain. In order to use `clultrain` you will need to
  have a local copy of `nodultrain` running and configured to load the 'ultrainio::chain_api_plugin'.

   clultrain contains documentation for all of its commands. For a list of all commands known to clultrain, simply run it with no arguments:
```
$ ./clultrain
Command Line Interface to ULTRAINIO Client
Usage: programs/clultrain/clultrain [OPTIONS] SUBCOMMAND

Options:
  -h,--help                   Print this help message and exit
  -u,--url TEXT=http://localhost:8888/
                              the http/https URL where nodultrain is running
  --wallet-url TEXT=http://localhost:8888/
                              the http/https URL where kultraind is running
  -r,--header                 pass specific HTTP header, repeat this option to pass multiple headers
  -n,--no-verify              don't verify peer certificate when using HTTPS
  -v,--verbose                output verbose actions on error

Subcommands:
  version                     Retrieve version information
  create                      Create various items, on and off the blockchain
  get                         Retrieve various items and information from the blockchain
  set                         Set or update blockchain state
  transfer                    Transfer UGAS from account to account
  net                         Interact with local p2p network connections
  wallet                      Interact with local wallet
  sign                        Sign a transaction
  push                        Push arbitrary transactions to the blockchain
  multisig                    Multisig contract commands

```
To get help with any particular subcommand, run it with no arguments as well:
```
$ ./clultrain create
Create various items, on and off the blockchain
Usage: ./clultrain create SUBCOMMAND

Subcommands:
  key                         Create a new keypair and print the public and private keys
  account                     Create a new account on the blockchain

$ ./clultrain create account
Create a new account on the blockchain
Usage: ./clultrain create account [OPTIONS] creator name OwnerKey ActiveKey

Positionals:
  creator TEXT                The name of the account creating the new account
  name TEXT                   The name of the new account
  OwnerKey TEXT               The owner public key for the new account
  ActiveKey TEXT              The active public key for the new account

Options:
  -x,--expiration             set the time in seconds before a transaction expires, defaults to 30s
  -f,--force-unique           force the transaction to be unique. this will consume extra bandwidth and remove any protections against accidently issuing the same transaction multiple times
  -s,--skip-sign              Specify if unlocked wallet keys should be used to sign transaction
  -d,--dont-broadcast         don't broadcast transaction to the network (just print to stdout)
  -p,--permission TEXT ...    An account and permission level to authorize, as in 'account@permission' (defaults to 'creator@active')
```
*/

#include <string>
#include <vector>
#include <regex>
#include <iostream>
#include <fc/crypto/hex.hpp>
#include <fc/variant.hpp>
#include <fc/io/datastream.hpp>
#include <fc/io/json.hpp>
#include <fc/io/console.hpp>
#include <fc/exception/exception.hpp>
#include <fc/variant_object.hpp>
#include <ultrainio/utilities/key_conversion.hpp>

#include <ultrainio/chain/name.hpp>
#include <ultrainio/chain/config.hpp>
#include <ultrainio/chain/wast_to_wasm.hpp>
#include <ultrainio/chain/trace.hpp>
#include <ultrainio/chain_plugin/chain_plugin.hpp>
#include <ultrainio/chain/contract_types.hpp>

#pragma push_macro("N")
#undef N

#include <boost/asio.hpp>
#include <boost/format.hpp>
#include <boost/dll/runtime_symbol_info.hpp>
#include <boost/filesystem.hpp>
#include <boost/process.hpp>
#include <boost/process/spawn.hpp>
#include <boost/range/algorithm/find_if.hpp>
#include <boost/range/algorithm/sort.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/range/algorithm/copy.hpp>
#include <boost/algorithm/string/classification.hpp>

#pragma pop_macro("N")

#include <Inline/BasicTypes.h>
#include <IR/Module.h>
#include <IR/Validate.h>
#include <WAST/WAST.h>
#include <WASM/WASM.h>
#include <Runtime/Runtime.h>

#include <fc/io/fstream.hpp>

#include "CLI11.hpp"
#include "help_text.hpp"
#include "localize.hpp"
#include "config.hpp"
#include "httpc.hpp"

using namespace std;
using namespace ultrainio;
using namespace ultrainio::chain;
using namespace ultrainio::utilities;
using namespace ultrainio::client::help;
using namespace ultrainio::client::http;
using namespace ultrainio::client::localize;
using namespace ultrainio::client::config;
using namespace boost::filesystem;

FC_DECLARE_EXCEPTION( explained_exception, 9000000, "explained exception, see error log" );
FC_DECLARE_EXCEPTION( localized_exception, 10000000, "an error occured" );
#define ULTRAINC_ASSERT( TEST, ... ) \
  FC_EXPAND_MACRO( \
    FC_MULTILINE_MACRO_BEGIN \
      if( UNLIKELY(!(TEST)) ) \
      {                                                   \
        std::cerr << localized( __VA_ARGS__ ) << std::endl;  \
        FC_THROW_EXCEPTION( explained_exception, #TEST ); \
      }                                                   \
    FC_MULTILINE_MACRO_END \
  )

string url = "http://127.0.0.1:8888/";
string wallet_url = "http://127.0.0.1:8900/";
bool no_verify = false;
vector<string> headers;
vector<string> addl_wallet_urls;

auto   tx_expiration = fc::seconds(60);
string tx_ref_block_num_or_id;
bool   tx_force_unique = false;
bool   tx_dont_broadcast = false;
bool   tx_skip_sign = false;
bool   tx_print_json = false;
bool   tx_multi_urls = false;
bool   print_request = false;
bool   print_response = false;

uint8_t  tx_max_cpu_usage = 0;
uint32_t tx_max_net_usage = 0;
uint32_t delaysec = 0;
static constexpr uint32_t default_cons_delaysec = 3*60;//24*3600;//抵押共识权重生效默认延迟一天  测试先使用三分钟
vector<string> tx_permission;

ultrainio::client::http::http_context context;

void add_standard_transaction_options(CLI::App* cmd, string default_permission = "") {
   CLI::callback_t parse_expiration = [](CLI::results_t res) -> bool {
      double value_s;
      if (res.size() == 0 || !CLI::detail::lexical_cast(res[0], value_s)) {
         return false;
      }

      tx_expiration = fc::seconds(static_cast<uint64_t>(value_s));
      return true;
   };
   delaysec = 0;
   cmd->add_option("-x,--expiration", parse_expiration, localized("set the time in seconds before a transaction expires, defaults to 60s"));
   cmd->add_flag("-f,--force-unique", tx_force_unique, localized("force the transaction to be unique. this will consume extra bandwidth and remove any protections against accidently issuing the same transaction multiple times"));
   cmd->add_flag("-s,--skip-sign", tx_skip_sign, localized("Specify if unlocked wallet keys should be used to sign transaction"));
   cmd->add_flag("-m,--multi-urls", tx_multi_urls, localized("Specify if multiple wallets urls should be used to sign transaction"));
   cmd->add_flag("-j,--json", tx_print_json, localized("print result as json"));
   cmd->add_flag("-d,--dont-broadcast", tx_dont_broadcast, localized("don't broadcast transaction to the network (just print to stdout)"));
   cmd->add_option("-r,--ref-block", tx_ref_block_num_or_id, (localized("set the reference block num or block id used for TAPOS (Transaction as Proof-of-Stake)")));

   string msg = "An account and permission level to authorize, as in 'account@permission'";
   if(!default_permission.empty())
      msg += " (defaults to '" + default_permission + "')";
   cmd->add_option("-p,--permission", tx_permission, localized(msg.c_str()));

   cmd->add_option("--max-cpu-usage-ms", tx_max_cpu_usage, localized("set an upper limit on the milliseconds of cpu usage budget, for the execution of the transaction (defaults to 0 which means no limit)"));
   cmd->add_option("--max-net-usage", tx_max_net_usage, localized("set an upper limit on the net usage budget, in bytes, for the transaction (defaults to 0 which means no limit)"));
   cmd->add_option("--delay-sec", delaysec, localized("set the delay_sec seconds, defaults to 0s"));
}

vector<chain::permission_level> get_account_permissions(const vector<string>& permissions) {
   auto fixedPermissions = permissions | boost::adaptors::transformed([](const string& p) {
      vector<string> pieces;
      split(pieces, p, boost::algorithm::is_any_of("@"));
      if( pieces.size() == 1 ) pieces.push_back( "active" );
      return chain::permission_level{ .actor = pieces[0], .permission = pieces[1] };
   });
   vector<chain::permission_level> accountPermissions;
   boost::range::copy(fixedPermissions, back_inserter(accountPermissions));
   return accountPermissions;
}

template<typename T>
fc::variant call( const std::string& url,
                  const std::string& path,
                  const T& v ) {
   try {
      auto sp = std::make_unique<ultrainio::client::http::connection_param>(context, parse_url(url) + path, no_verify ? false : true, headers);
      return ultrainio::client::http::do_http_call( *sp, fc::variant(v), print_request, print_response );
   }
   catch(boost::system::system_error& e) {
      if(url == ::url)
         std::cerr << localized("Failed to connect to nodultrain at ${u}; is nodultrain running?", ("u", url)) << std::endl;
      else if(url == ::wallet_url)
         std::cerr << localized("Failed to connect to kultraind at ${u}; is kultraind running?", ("u", url)) << std::endl;
      throw connection_exception(fc::log_messages{FC_LOG_MESSAGE(error, e.what())});
   }
}

template<typename T>
fc::variant call( const std::string& path,
                  const T& v ) { return call( ::url, path, fc::variant(v) ); }

template<>
fc::variant call( const std::string& url,
                  const std::string& path) { return call( url, path, fc::variant() ); }

ultrainio::chain_apis::read_only::get_info_results get_info() {
   return call(::url, get_info_func).as<ultrainio::chain_apis::read_only::get_info_results>();
}

string generate_nonce_string() {
   return fc::to_string(fc::time_point::now().time_since_epoch().count());
}

chain::action generate_nonce_action() {
   return chain::action( {}, config::null_account_name, "nonce", fc::raw::pack(fc::time_point::now().time_since_epoch().count()));
}

void prompt_for_wallet_password(string& pw, const string& name) {
   if(pw.size() == 0 && name != "SecureEnclave") {
      std::cout << localized("password: ");
      fc::set_console_echo(false);
      std::getline( std::cin, pw, '\n' );
      fc::set_console_echo(true);
   }
}

fc::variant determine_required_keys(const signed_transaction& trx) {
   // TODO better error checking
   //wdump((trx));
   const auto& public_keys = call(wallet_url, wallet_public_keys);
   fc::mutable_variant_object get_arg;
   flat_set<public_key_type> all_keys;

   if (tx_multi_urls && addl_wallet_urls.size() > 0) {
      all_keys = public_keys.as<flat_set<public_key_type>>();

      for (const auto& addl_url : addl_wallet_urls) {
         const auto& more_keys = call(addl_url, wallet_public_keys);
         all_keys.merge(more_keys.as<flat_set<public_key_type>>());
      }
      get_arg = fc::mutable_variant_object
           ("transaction", (transaction)trx)
           ("available_keys", all_keys);
   } else {
      get_arg = fc::mutable_variant_object
           ("transaction", (transaction)trx)
           ("available_keys", public_keys);
   }
   const auto& required_keys = call(get_required_keys, get_arg);
   return required_keys["required_keys"];
}

fc::variant bin_to_json( const account_name& account, const action_name& action, const vector<char>& binargs );

void sign_transaction(signed_transaction& trx, fc::variant& required_keys, const chain_id_type& chain_id) {
   // Try to sign trx from more wallet urls when trx -m flag is set
   if (tx_multi_urls && addl_wallet_urls.size() > 0) {
      vector<string> args;
      for (auto act : trx.actions) {
         auto res = bin_to_json(act.account, act.name, act.data);
         auto str = fc::json::to_string(res);
         if( str.size() > 100 ) str = str.substr(0,100) + "...";
         args.push_back("account: " + act.account.to_string() + " name: " + act.name.to_string() + " " + str);
      }

      fc::variants sign_args = {fc::variant(trx), required_keys, fc::variant(chain_id), fc::variant(args)};

      const auto& signed_trx = call(wallet_url, wallet_sign_trx_multi, sign_args);
      trx = signed_trx.as<signed_transaction>();
      for (const auto& addl_url : addl_wallet_urls) {
         const auto& addl_trx = call(addl_url, wallet_sign_trx_multi, sign_args);
         for (const auto& sig : addl_trx.as<signed_transaction>().signatures) {
            trx.signatures.push_back(sig);
         }
      }
      fc::deduplicate(trx.signatures);

      // The check in wallet manager is removed so we add check here
      if (trx.signatures.size() != required_keys.as<flat_set<public_key_type>>().size()) {
         ULTRAIN_THROW(chain::wallet_missing_pub_key_exception, "Public key not found in unlocked wallets");
      }
   } else {
      fc::variants sign_args = {fc::variant(trx), required_keys, fc::variant(chain_id)};
      const auto& signed_trx = call(wallet_url, wallet_sign_trx, sign_args);
      trx = signed_trx.as<signed_transaction>();
   }
}

fc::variant push_transaction( signed_transaction& trx, int32_t extra_kcpu = 1000, packed_transaction::compression_type compression = packed_transaction::none ) {
   auto info = get_info();
   trx.expiration = info.head_block_time + tx_expiration;

   // Set tapos, default to last irreversible block if it's not specified by the user
   block_id_type ref_block_id = info.last_irreversible_block_id;
   try {
      fc::variant ref_block;
      if (!tx_ref_block_num_or_id.empty()) {
         ref_block = call(get_block_func, fc::mutable_variant_object("block_num_or_id", tx_ref_block_num_or_id));
         ref_block_id = ref_block["id"].as<block_id_type>();
      }
   } ULTRAIN_RETHROW_EXCEPTIONS(invalid_ref_block_exception, "Invalid reference block num or id: ${block_num_or_id}", ("block_num_or_id", tx_ref_block_num_or_id));
   trx.set_reference_block(ref_block_id);

   if (tx_force_unique) {
      trx.context_free_actions.emplace_back( generate_nonce_action() );
   }

   trx.max_cpu_usage_ms = tx_max_cpu_usage;
   trx.max_net_usage_words = (tx_max_net_usage + 7)/8;
   trx.delay_sec = delaysec;

   if (!tx_skip_sign) {
      auto required_keys = determine_required_keys(trx);
      sign_transaction(trx, required_keys, info.chain_id);
   }

   if (!tx_dont_broadcast) {
      return call(push_txn_func, packed_transaction(trx, compression));
   } else {
      return fc::variant(trx);
   }
}

fc::variant push_actions(std::vector<chain::action>&& actions, int32_t extra_kcpu, packed_transaction::compression_type compression = packed_transaction::none ) {
   signed_transaction trx;
   trx.actions = std::forward<decltype(actions)>(actions);

   return push_transaction(trx, extra_kcpu, compression);
}

void print_action( const fc::variant& at ) {
   const auto& receipt = at["receipt"];
   auto receiver = receipt["receiver"].as_string();
   const auto& act = at["act"].get_object();
   auto code = act["account"].as_string();
   auto func = act["name"].as_string();
   auto args = fc::json::to_string( act["data"] );
   auto console = at["console"].as_string();
   auto ret_value = at["return_value"].as_string();

   /*
   if( code == "ultrainio" && func == "setcode" )
      args = args.substr(40)+"...";
   if( name(code) == config::system_account_name && func == "setabi" )
      args = args.substr(40)+"...";
   */
   if( args.size() > 100 ) args = args.substr(0,100) + "...";
   cout << "#" << std::setw(14) << std::right << receiver << " <= " << std::setw(28) << std::left << (code +"::" + func) << " " << args;
   if (!ret_value.empty()) {
       cout << "  => " << ret_value;
   }
   cout << std::endl;

   if( console.size() ) {
      std::stringstream ss(console);
      string line;
      std::getline( ss, line );
      cout << ">> " << line << "\n";
   }
}

static unordered_map<account_name, std::vector<char> > abi_cache;

bytes variant_to_bin( const account_name& account, const action_name& action, const fc::variant& action_args_var ) {
   auto it = abi_cache.find( account );
   if ( it == abi_cache.end() ) {
      const auto result = call(get_raw_code_and_abi_func, fc::mutable_variant_object("account_name", account));
      std::tie( it, std::ignore ) = abi_cache.emplace( account, result["abi"].as_blob().data );
      //we also received result["wasm"], but we don't use it
   }
   const std::vector<char>& abi_v = it->second;

   abi_def abi;
   if( abi_serializer::to_abi(abi_v, abi) ) {
      abi_serializer abis( abi, fc::seconds(10) );
      auto action_type = abis.get_action_type(action);
      FC_ASSERT(!action_type.empty(), "Unknown action ${action} in contract ${contract}", ("action", action)("contract", account));
      return abis.variant_to_binary(action_type, action_args_var, fc::seconds(10));
   } else {
      FC_ASSERT(false, "No ABI found for ${contract}", ("contract", account));
   }
}

fc::variant bin_to_json( const account_name& account, const action_name& action, const vector<char>& binargs ) {
   auto it = abi_cache.find( account );
   if ( it == abi_cache.end() ) {
      const auto result = call(get_raw_code_and_abi_func, fc::mutable_variant_object("account_name", account));
      std::tie( it, std::ignore ) = abi_cache.emplace( account, result["abi"].as_blob().data );
      //we also received result["wasm"], but we don't use it
   }
   const std::vector<char>& abi_v = it->second;

   abi_def abi;
   fc::variant result;
   fc::microseconds abi_serializer_max_time = fc::seconds(1);

   if( abi_serializer::to_abi(abi_v, abi) ) {
      abi_serializer abis( abi, abi_serializer_max_time );
      result = abis.binary_to_variant( abis.get_action_type(action ), binargs, abi_serializer_max_time );
   } else {
      ULTRAIN_ASSERT(false, abi_not_found_exception, "No ABI found for ${contract}", ("contract", account));
   }
   return result;
}

fc::variant json_from_file_or_string(const string& file_or_str, fc::json::parse_type ptype = fc::json::legacy_parser)
{
   regex r("^[ \t]*[\{\[]");
   if ( !regex_search(file_or_str, r) && fc::is_regular_file(file_or_str) ) {
      return fc::json::from_file(file_or_str, ptype);
   } else {
      return fc::json::from_string(file_or_str, ptype);
   }
}

bytes json_or_file_to_bin( const account_name& account, const action_name& action, const string& data_or_filename ) {
   fc::variant action_args_var;
   if( !data_or_filename.empty() ) {
      try {
         action_args_var = json_from_file_or_string(data_or_filename, fc::json::relaxed_parser);
      } ULTRAIN_RETHROW_EXCEPTIONS(action_type_exception, "Fail to parse action JSON data='${data}'", ("data", data_or_filename));
   }
   return variant_to_bin( account, action, action_args_var );
}

void print_action_tree( const fc::variant& action ) {
   print_action( action );
   const auto& inline_traces = action["inline_traces"].get_array();
   for( const auto& t : inline_traces ) {
      print_action_tree( t );
   }
}

void print_result( const fc::variant& result ) { try {
      if (result.is_object() && result.get_object().contains("processed")) {
         const auto& processed = result["processed"];
         const auto& transaction_id = processed["id"].as_string();
         string status = processed["receipt"].is_object() ? processed["receipt"]["status"].as_string() : "failed";
         int64_t net = -1;
         int64_t cpu = -1;
         if( processed.get_object().contains( "receipt" )) {
            const auto& receipt = processed["receipt"];
            if( receipt.is_object()) {
               net = receipt["net_usage_words"].as_int64() * 8;
               cpu = receipt["cpu_usage_us"].as_int64();
            }
         }

         cerr << status << " transaction: " << transaction_id << "  ";
         if( net < 0 ) {
            cerr << "<unknown>";
         } else {
            cerr << net;
         }
         cerr << " bytes  ";
         if( cpu < 0 ) {
            cerr << "<unknown>";
         } else {
            cerr << cpu;
         }

         cerr << " us\n";

         if( status == "failed" ) {
            auto soft_except = processed["except"].as<optional<fc::exception>>();
            if( soft_except ) {
               edump((soft_except->to_detail_string()));
            }
         } else {
            const auto& actions = processed["action_traces"].get_array();
            for( const auto& a : actions ) {
               print_action_tree( a );
            }
            wlog( "\rwarning: transaction executed locally, but may not be confirmed by the network yet" );
         }
      } else {
         cerr << fc::json::to_pretty_string( result ) << endl;
      }
} FC_CAPTURE_AND_RETHROW( (result) ) }

using std::cout;
void send_actions(std::vector<chain::action>&& actions, int32_t extra_kcpu = 1000, packed_transaction::compression_type compression = packed_transaction::none ) {
   auto result = push_actions( move(actions), extra_kcpu, compression);
   if( tx_print_json ) {
      cout << fc::json::to_pretty_string( result ) << endl;
   } else {
      print_result( result );
   }
}

void send_transaction( signed_transaction& trx, int32_t extra_kcpu, packed_transaction::compression_type compression = packed_transaction::none  ) {
   auto result = push_transaction(trx, extra_kcpu, compression);

   if( tx_print_json ) {
      cout << fc::json::to_pretty_string( result ) << endl;
   } else {
      print_result( result );
   }
}

chain::action create_newaccount(const name& creator, const name& newaccount, public_key_type owner, public_key_type active, bool updateable = false) {
   return action {
      tx_permission.empty() ? vector<chain::permission_level>{{creator,config::active_name}} : get_account_permissions(tx_permission),
      ultrainio::chain::newaccount{
         .creator      = creator,
         .name         = newaccount,
         .owner        = ultrainio::chain::authority{1, {{owner, 1}}, {}},
         .active       = ultrainio::chain::authority{1, {{active, 1}}, {}},
         .updateable   = updateable
      }
   };
}

chain::action create_action(const vector<permission_level>& authorization, const account_name& code, const action_name& act, const fc::variant& args) {
   return chain::action{authorization, code, act, variant_to_bin(code, act, args)};
}

fc::variant regproducer_variant(const account_name& producer, const std::string& key, const std::string& bls_key, const account_name& rewards_account, const string& url, string location) {
    return fc::mutable_variant_object()
        ("producer", producer)
        ("producer_key", key)
        ("bls_key", bls_key)
        ("rewards_account", rewards_account)
        ("url", url)
        ("location", location)
        ;
}

chain::action create_transfer(const string& contract, const name& sender, const name& recipient, asset amount, const string& memo ) {

   auto transfer = fc::mutable_variant_object
      ("from", sender)
      ("to", recipient)
      ("quantity", amount)
      ("memo", memo);

   auto args = fc::mutable_variant_object
      ("code", contract)
      ("action", "transfer")
      ("args", transfer);

   return action {
      tx_permission.empty() ? vector<chain::permission_level>{{sender,config::active_name}} : get_account_permissions(tx_permission),
      contract, "transfer", variant_to_bin( contract, NEX(transfer), transfer )
   };
}

chain::action create_setabi(const name& account, const abi_def& abi) {
   return action {
      tx_permission.empty() ? vector<chain::permission_level>{{account,config::active_name}} : get_account_permissions(tx_permission),
      setabi{
         .account   = account,
         .abi       = fc::raw::pack(abi)
      }
   };
}

chain::action create_setcode(const name& account, const bytes& code) {
   return action {
      tx_permission.empty() ? vector<chain::permission_level>{{account,config::active_name}} : get_account_permissions(tx_permission),
      setcode{
         .account   = account,
         .vmtype    = 0,
         .vmversion = 0,
         .code      = code
      }
   };
}

chain::action create_updateauth(const name& account, const name& permission, const name& parent, const authority& auth) {
   return action { tx_permission.empty() ? vector<chain::permission_level>{{account,config::active_name}} : get_account_permissions(tx_permission),
                   updateauth{account, permission, parent, auth}};
}

chain::action create_deleteauth(const name& account, const name& permission) {
   return action { tx_permission.empty() ? vector<chain::permission_level>{{account,config::active_name}} : get_account_permissions(tx_permission),
                   deleteauth{account, permission}};
}

chain::action create_linkauth(const name& account, const name& code, const action_name& type, const name& requirement) {
   return action { tx_permission.empty() ? vector<chain::permission_level>{{account,config::active_name}} : get_account_permissions(tx_permission),
                   linkauth{account, code, type, requirement}};
}

chain::action create_unlinkauth(const name& account, const name& code, const action_name& type) {
   return action { tx_permission.empty() ? vector<chain::permission_level>{{account,config::active_name}} : get_account_permissions(tx_permission),
                   unlinkauth{account, code, type}};
}

authority parse_json_authority(const std::string& authorityJsonOrFile) {
   try {
      return json_from_file_or_string(authorityJsonOrFile).as<authority>();
   } ULTRAIN_RETHROW_EXCEPTIONS(authority_type_exception, "Fail to parse Authority JSON '${data}'", ("data",authorityJsonOrFile))
}

authority parse_json_authority_or_key(const std::string& authorityJsonOrFile) {
   if (boost::istarts_with(authorityJsonOrFile, "UTR") || boost::istarts_with(authorityJsonOrFile, "PUB_R1")) {
      try {
         return authority(public_key_type(authorityJsonOrFile));
      } ULTRAIN_RETHROW_EXCEPTIONS(public_key_type_exception, "Invalid public key: ${public_key}", ("public_key", authorityJsonOrFile))
   } else {
      auto result = parse_json_authority(authorityJsonOrFile);
      ULTRAIN_ASSERT( ultrainio::chain::validate(result), authority_type_exception, "Authority failed validation! ensure that keys, accounts, and waits are sorted and that the threshold is valid and satisfiable!");
      return result;
   }
}

asset to_asset( const string& code, const string& s ) {
   static map<ultrainio::chain::symbol_code, ultrainio::chain::symbol> cache;
   auto a = asset::from_string( s );
   ultrainio::chain::symbol_code sym = a.get_symbol().to_symbol_code();
   auto it = cache.find( sym );
   auto sym_str = a.symbol_name();
   if ( it == cache.end() ) {
      auto json = call(get_currency_stats_func, fc::mutable_variant_object("json", false)
                       ("code", code)
                       ("symbol", sym_str)
      );
      auto obj = json.get_object();
      auto obj_it = obj.find( sym_str );
      if (obj_it != obj.end()) {
         auto result = obj_it->value().as<ultrainio::chain_apis::read_only::get_currency_stats_result>();
         auto p = cache.insert(make_pair( sym, result.max_supply.get_symbol() ));
         it = p.first;
      } else {
         ULTRAIN_THROW(symbol_type_exception, "Symbol ${s} is not supported by token contract ${c}", ("s", sym_str)("c", code));
      }
   }
   auto expected_symbol = it->second;
   if ( a.decimals() < expected_symbol.decimals() ) {
      auto factor = expected_symbol.precision() / a.precision();
      auto a_old = a;
      a = asset( a.get_amount() * factor, expected_symbol );
   } else if ( a.decimals() > expected_symbol.decimals() ) {
      ULTRAIN_THROW(symbol_type_exception, "Too many decimal digits in ${a}, only ${d} supported", ("a", a)("d", expected_symbol.decimals()));
   } // else precision matches
   return a;
}

inline asset to_asset( const string& s ) {
   return to_asset( "utrio.token", s );
}

struct set_account_permission_subcommand {
   string accountStr;
   string permissionStr;
   string authorityJsonOrFile;
   string parentStr;

   set_account_permission_subcommand(CLI::App* accountCmd) {
      auto permissions = accountCmd->add_subcommand("permission", localized("set parameters dealing with account permissions"));
      permissions->add_option("account", accountStr, localized("The account to set/delete a permission authority for"))->required();
      permissions->add_option("permission", permissionStr, localized("The permission name to set/delete an authority for"))->required();
      permissions->add_option("authority", authorityJsonOrFile, localized("[delete] NULL, [create/update] public key, JSON string, or filename defining the authority"))->required();
      permissions->add_option("parent", parentStr, localized("[create] The permission name of this parents permission (Defaults to: \"Active\")"));

      add_standard_transaction_options(permissions, "account@active");

      permissions->set_callback([this] {
         name account = name(accountStr);
         name permission = name(permissionStr);
         bool is_delete = boost::iequals(authorityJsonOrFile, "null");

         if (is_delete) {
            send_actions({create_deleteauth(account, permission)});
         } else {
            authority auth = parse_json_authority_or_key(authorityJsonOrFile);

            name parent;
            if (parentStr.size() == 0 && permissionStr != "owner") {
               // see if we can auto-determine the proper parent
               const auto account_result = call(get_account_func, fc::mutable_variant_object("account_name", accountStr));
               const auto& existing_permissions = account_result.get_object()["permissions"].get_array();
               auto permissionPredicate = [this](const auto& perm) {
                  return perm.is_object() &&
                        perm.get_object().contains("permission") &&
                        boost::equals(perm.get_object()["permission"].get_string(), permissionStr);
               };

               auto itr = boost::find_if(existing_permissions, permissionPredicate);
               if (itr != existing_permissions.end()) {
                  parent = name((*itr).get_object()["parent"].get_string());
               } else {
                  // if this is a new permission and there is no parent we default to "active"
                  parent = name(config::active_name);

               }
            } else {
               parent = name(parentStr);
            }

            send_actions({create_updateauth(account, permission, parent, auth)});
         }
      });
   }

};

struct set_action_permission_subcommand {
   string accountStr;
   string codeStr;
   string typeStr;
   string requirementStr;

   set_action_permission_subcommand(CLI::App* actionRoot) {
      auto permissions = actionRoot->add_subcommand("permission", localized("set parmaters dealing with account permissions"));
      permissions->add_option("account", accountStr, localized("The account to set/delete a permission authority for"))->required();
      permissions->add_option("code", codeStr, localized("The account that owns the code for the action"))->required();
      permissions->add_option("type", typeStr, localized("the type of the action"))->required();
      permissions->add_option("requirement", requirementStr, localized("[delete] NULL, [set/update] The permission name require for executing the given action"))->required();

      add_standard_transaction_options(permissions, "account@active");

      permissions->set_callback([this] {
         name account = name(accountStr);
         name code = name(codeStr);
         action_name type = action_name(typeStr);
         bool is_delete = boost::iequals(requirementStr, "null");

         if (is_delete) {
            send_actions({create_unlinkauth(account, code, type)});
         } else {
            name requirement = name(requirementStr);
            send_actions({create_linkauth(account, code, type, requirement)});
         }
      });
   }
};


bool local_port_used(const string& lo_address, uint16_t port) {
    using namespace boost::asio;

    io_service ios;
    boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address::from_string(lo_address), port);
    boost::asio::ip::tcp::socket socket(ios);
    boost::system::error_code ec = error::would_block;
    //connecting/failing to connect to localhost should be always fast - don't care about timeouts
    socket.async_connect(endpoint, [&](const boost::system::error_code& error) { ec = error; } );
    do {
        ios.run_one();
    } while (ec == error::would_block);
    return !ec;
}

void try_local_port( const string& lo_address, uint16_t port, uint32_t duration ) {
   using namespace std::chrono;
   auto start_time = duration_cast<std::chrono::milliseconds>( system_clock::now().time_since_epoch() ).count();
   while ( !local_port_used(lo_address, port)) {
      if (duration_cast<std::chrono::milliseconds>( system_clock::now().time_since_epoch()).count() - start_time > duration ) {
         std::cerr << "Unable to connect to kultraind, if kultraind is running please kill the process and try again.\n";
         throw connection_exception(fc::log_messages{FC_LOG_MESSAGE(error, "Unable to connect to kultraind")});
      }
   }
}

void make_kultraind_running(CLI::App* app, const string& url) {
    // get, version, net do not require kultraind
    if (tx_skip_sign || app->got_subcommand("get") || app->got_subcommand("version") || app->got_subcommand("net"))
        return;
    if (app->get_subcommand("create")->got_subcommand("key")) // create key does not require wallet
       return;
    if (auto* subapp = app->get_subcommand("system")) {
       if (subapp->got_subcommand("listproducers") || subapp->got_subcommand("listdelcons")) // system list* do not require wallet
         return;
    }

    auto parsed_url = parse_url(url);
    auto resolved_url = resolve_url(context, parsed_url);

    if (!resolved_url.is_loopback)
        return;

    for (const auto& addr: resolved_url.resolved_addresses)
       if (local_port_used(addr, resolved_url.resolved_port))  // Hopefully taken by kultraind
          return;

    boost::filesystem::path binPath = boost::dll::program_location();
    binPath.remove_filename();
    // This extra check is necessary when running clultrain like this: ./clultrain ...
    if (binPath.filename_is_dot())
        binPath.remove_filename();
    binPath.append("kultraind"); // if clultrain and kultraind are in the same installation directory
    if (!boost::filesystem::exists(binPath)) {
        binPath.remove_filename().remove_filename().append("kultraind").append("kultraind");
    }

    const auto& lo_address = resolved_url.resolved_addresses.front();
    if (boost::filesystem::exists(binPath)) {
        namespace bp = boost::process;
        binPath = boost::filesystem::canonical(binPath);

        vector<std::string> pargs;
        pargs.push_back("--http-server-address=" + lo_address + ":" + std::to_string(resolved_url.resolved_port));

        ::boost::process::child kultrain(binPath, pargs,
                                     bp::std_in.close(),
                                     bp::std_out > bp::null,
                                     bp::std_err > bp::null);
        if (kultrain.running()) {
            std::cerr << binPath << " launched" << std::endl;
            kultrain.detach();
            try_local_port(lo_address, resolved_url.resolved_port, 2000);
        } else {
            std::cerr << "No wallet service listening on " << lo_address << ":"
                      << std::to_string(resolved_url.resolved_port) << ". Failed to launch " << binPath << std::endl;
        }
    } else {
        std::cerr << "No wallet service listening on " << lo_address << ":" << std::to_string(resolved_url.resolved_port)
                  << ". Cannot automatically start kultraind because kultraind was not found." << std::endl;
    }
}

void ensure_kultraind_running(CLI::App* app) {
    make_kultraind_running(app, wallet_url);
    if (tx_multi_urls && addl_wallet_urls.size() > 0) {
        for (const auto& addl_url : addl_wallet_urls) {
            make_kultraind_running(app, addl_url);
        }
    }
}

CLI::callback_t obsoleted_option_host_port = [](CLI::results_t) {
   std::cerr << localized("Host and port options (-H, --wallet-host, etc.) have been replaced with -u/--url and --wallet-url\n"
                          "Use for example -u http://localhost:8888 or --url https://example.invalid/\n");
   exit(1);
   return false;
};

struct register_producer_subcommand {
   string producer_str;
   string producer_key_str;
   string bls_key_str;
   string url;
   string loc;
   string rewards_account;
   bool superprivflg = false;

   register_producer_subcommand(CLI::App* actionRoot) {
      auto register_producer = actionRoot->add_subcommand("regproducer", localized("Register a new producer"));
      register_producer->add_option("account", producer_str, localized("The account to register as a producer"))->required();
      register_producer->add_option("producer_key", producer_key_str, localized("The producer's produce block public key"))->required();
      register_producer->add_option("bls_key", bls_key_str, localized("The producer's bls public key"))->required();
      register_producer->add_option("rewards_account", rewards_account, localized("The producer's block reward refund account"))->required();
      register_producer->add_option("url", url, localized("url where info about producer can be found"), true)->required();
      register_producer->add_option("location", loc, localized("name of the side chain to be added"), true)->required();
      register_producer->add_flag("-u,--superpriv", superprivflg, localized("register_producer add privileged (rarely used)"));
      add_standard_transaction_options(register_producer);

      register_producer->set_callback([this] {
         // TODO(yufengshen): Check if the key is valid.
         auto regprod_var = regproducer_variant(producer_str, producer_key_str, bls_key_str, rewards_account, url, loc );
         vector<permission_level> permiss_info{permission_level{producer_str,config::active_name}};
         if(superprivflg)
            permiss_info.push_back(permission_level{N(ultrainio),config::active_name});
         send_actions({create_action(permiss_info, config::system_account_name, NEX(regproducer), regprod_var)});
      });
   }
};

struct create_account_subcommand {
   string creator;
   string account_name;
   string owner_key_str;
   string active_key_str;
   bool   unrenewable_val = false;

   create_account_subcommand( CLI::App* actionRoot ) {
      auto createAccount = actionRoot->add_subcommand( "account", localized("Create an account, buy ram, stake for bandwidth for the account"));
      createAccount->add_option("creator", creator, localized("The name of the account creating the new account"))->required();
      createAccount->add_option("name", account_name, localized("The name of the new account"))->required();
      createAccount->add_option("OwnerKey", owner_key_str, localized("The owner public key for the new account"))->required();
      createAccount->add_option("ActiveKey", active_key_str, localized("The active public key for the new account"));
      createAccount->add_flag("-u,--unrenewable", unrenewable_val, localized("The unrenewable setting for the new account"));

      add_standard_transaction_options(createAccount);

      createAccount->set_callback([this] {
            if( !active_key_str.size() )
               active_key_str = owner_key_str;
            public_key_type owner_key, active_key;
            try {
               owner_key = public_key_type(owner_key_str);
            } ULTRAIN_RETHROW_EXCEPTIONS(public_key_type_exception, "Invalid owner public key: ${public_key}", ("public_key", owner_key_str));
            try {
               active_key = public_key_type(active_key_str);
            } ULTRAIN_RETHROW_EXCEPTIONS(public_key_type_exception, "Invalid active public key: ${public_key}", ("public_key", active_key_str));
            auto create = create_newaccount(creator, account_name, owner_key, active_key, !unrenewable_val);
            send_actions( { create } );
      });
   }
};

struct list_producers_subcommand {
   bool print_json = false;
   std::string lower;
   std::string  chain_name;
   list_producers_subcommand(CLI::App* actionRoot) {
      auto list_producers = actionRoot->add_subcommand("listproducers", localized("List producers"));
      list_producers->add_option("chain_name", chain_name, localized("Specify a chain so that only its producers will be shown"));
      list_producers->add_flag("--json,-j", print_json, localized("Output in JSON format"));
      list_producers->add_option("-L,--lower", lower, localized("lower bound value of key, defaults to first"));
      list_producers->set_callback([this] {
         bool show_all = false;
         if(chain_name.empty()){
             show_all = true;
         }
         auto rawResult = call(get_producers_func, fc::mutable_variant_object
            ("json", true)("lower_bound", lower)("all_chain", show_all)("chain_name", chain_name));
         if ( print_json ) {
            std::cout << fc::json::to_pretty_string(rawResult) << std::endl;
            return;
         }
         auto result = rawResult.as<ultrainio::chain_apis::read_only::get_producers_result>();
         if ( result.rows.empty() ) {
            std::cout << "No producers found" << std::endl;
            return;
         }
         uint64_t total_unpaid_balance = 0;
         uint64_t total_produce_blocks = 0;
         std::map<std::string,uint64_t>  chaininfo;
         printf("%-13s %-54s  %-16s  %-8s   %-13s    %-12s\n", "Producer", "Producer key", "Consensus_weight", "location","unpaid_balance","total_blocks");
         for ( auto& row : result.rows ){
            printf("%-13.13s %-54.54s  %-16ld  %-9s  %-.4f UGAS       %-12lu\n",
                   row.prod_detail["owner"].as_string().c_str(),
                   row.prod_detail["producer_key"].as_string().c_str(),
                   row.prod_detail["total_cons_staked"].as_int64(),
                   row.chain_name.to_string().c_str(),
                   ((double)row.prod_detail["unpaid_balance"].as_uint64())/10000,
                   row.prod_detail["total_produce_block"].as_uint64()
                   );
            total_unpaid_balance += row.prod_detail["unpaid_balance"].as_uint64();
            total_produce_blocks += row.prod_detail["total_produce_block"].as_uint64();
            chaininfo[row.chain_name.to_string()]++;
	     }
         printf("total_unpaid_balance: %-.4f UGAS \n",(double)total_unpaid_balance/10000);
         std::cout << "total_produce_blocks: " << total_produce_blocks << std::endl;
         for(auto iter = chaininfo.begin(); iter != chaininfo.end(); iter++)
            std::cout<<"location:"<< iter->first <<"    number:"<<iter->second<<std::endl;
         if ( !result.more.empty() )
            std::cout << "-L " << result.more << " for more" << std::endl;
      });
   }
};


struct buy_respackage_subcommand {
   string from_str;
   string receiver_str;
   int    combosize = 0;
   int    days = 0;
   string location;
   buy_respackage_subcommand(CLI::App* actionRoot) {
      auto buy_respackage = actionRoot->add_subcommand("resourcelease", localized("buy resources packages"));
      buy_respackage->add_option("from", from_str, localized("The account that pays for resources packages"))->required();
      buy_respackage->add_option("receiver", receiver_str, localized("The account to receive the resources packages"))->required();
      buy_respackage->add_option("combosize", combosize, localized("The amount of resources packages"))->required();
      buy_respackage->add_option("days", days, localized("days that receiver will own the resource lease for"))->required();
      buy_respackage->add_option("location", location, localized("sidechain name that the resource lease located on"))->required();
      add_standard_transaction_options(buy_respackage);

      buy_respackage->set_callback([this] {
         fc::variant act_payload = fc::mutable_variant_object()
                  ("from", from_str)
                  ("receiver", receiver_str)
                  ("combosize", combosize)
                  ("days", days)
                  ("location", location);
         std::vector<chain::action> acts{create_action({permission_level{from_str,config::active_name}}, config::system_account_name, NEX(resourcelease), act_payload)};
         send_actions(std::move(acts));
      });
   }
};

struct transfer_res_subcommand {
   string from_str;
   string receiver_str;
   int    combosize = 0;
   string location;
   transfer_res_subcommand(CLI::App* actionRoot) {
      auto transfer_res = actionRoot->add_subcommand("transferresource", localized("transfer resources from account to account"));
      transfer_res->add_option("from", from_str, localized("The account that sending resources"))->required();
      transfer_res->add_option("to", receiver_str, localized("The account that receiving the resources"))->required();
      transfer_res->add_option("combosize", combosize, localized("The amount of resources to transfer"))->required();
      transfer_res->add_option("location", location, localized("sidechain name of the resource lease"))->required();
      add_standard_transaction_options(transfer_res);

      transfer_res->set_callback([this] {
         fc::variant act_payload = fc::mutable_variant_object()
                  ("from", from_str)
                  ("to", receiver_str)
                  ("combosize", combosize)
                  ("location", location);
         vector<permission_level> permissions;
         if( tx_permission.empty() ) {
             permissions.emplace_back(permission_level{from_str, config::active_name});
         } else {
             permissions = get_account_permissions(tx_permission);
         }
         std::vector<chain::action> acts{create_action(permissions, config::system_account_name, NEX(transferresource), act_payload)};
         send_actions(std::move(acts));
      });
   }
};

struct delegate_cons_subcommand {
   string from_str;
   string receiver_str;
   string stake_cons_amount;

   delegate_cons_subcommand(CLI::App* actionRoot) {
      auto delegate_cons = actionRoot->add_subcommand("delegatecons", localized("Delegate consensus weight "));
      delegate_cons->add_option("from", from_str, localized("The account to delegate consensus weight from"))->required();
      delegate_cons->add_option("receiver", receiver_str, localized("The account to undelegate consensus weight bandwidth from"))->required();
      delegate_cons->add_option("stake_cons_quantity", stake_cons_amount, localized("The amount of UTR to stake for consensus weight"))->required();
      add_standard_transaction_options(delegate_cons);
      if(delaysec < default_cons_delaysec){
          delaysec = default_cons_delaysec;
      }
      delegate_cons->set_callback([this] {
         fc::variant act_payload = fc::mutable_variant_object()
                  ("from", from_str)
                  ("receiver", receiver_str)
                  ("stake_cons_quantity", to_asset(stake_cons_amount));
         std::vector<chain::action> acts{create_action({permission_level{from_str,config::active_name}}, config::system_account_name, NEX(delegatecons), act_payload)};
         send_actions(std::move(acts));
      });
   }
};

struct undelegate_cons_subcommand {
   string from_str;
   string receiver_str;

   undelegate_cons_subcommand(CLI::App* actionRoot) {
      auto undelegate_cons = actionRoot->add_subcommand("undelegatecons", localized("Undelegate consensus weight bandwidth"));
      undelegate_cons->add_option("from", from_str, localized("The account undelegating consensus weight bandwidth"))->required();
      undelegate_cons->add_option("receiver", receiver_str, localized("The account to undelegate consensus weight bandwidth from"))->required();
      add_standard_transaction_options(undelegate_cons);

      undelegate_cons->set_callback([this] {
         fc::variant act_payload = fc::mutable_variant_object()
                  ("from", from_str)
                  ("receiver", receiver_str);
         send_actions({create_action({permission_level{from_str,config::active_name}}, config::system_account_name, NEX(undelegatecons), act_payload)});
      });
   }
};

struct list_delcons_subcommand {
   ultrainio::name account;
   bool print_json = false;

   list_delcons_subcommand(CLI::App* actionRoot) {
      auto list_bw = actionRoot->add_subcommand("listdelcons", localized("List delegated consensus weight"));
      list_bw->add_option("account", account, localized("The account delegated consensus weight"))->required();
      list_bw->add_flag("--json,-j", print_json, localized("Output in JSON format") );

      list_bw->set_callback([this] {
            //get entire table in scope of user account
            auto result = call(get_table_func, fc::mutable_variant_object("json", true)
                               ("code", name(config::system_account_name).to_string())
                               ("scope", account.to_string())
                               ("table", "delcons")
            );
            if (!print_json) {
               auto res = result.as<ultrainio::chain_apis::read_only::get_table_records_result>();
               if ( !res.rows.empty() ) {
                  std::cout << std::setw(13) << std::left << "Receiver" << std::setw(21) << std::left << "consensus weight" << std::endl;
                  for ( auto& r : res.rows ){
                     std::cout << std::setw(13) << std::left << r["to"].as_string()
                               << std::setw(21) << std::left << r["cons_weight"].as_string()
                               << std::endl;
                  }
               } else {
                  std::cerr << "Delegated consensus not found" << std::endl;
               }
            } else {
               std::cout << fc::json::to_pretty_string(result) << std::endl;
            }
      });
   }
};

struct canceldelay_subcommand {
   string canceling_account;
   string canceling_permission;
   string trx_id;

   canceldelay_subcommand(CLI::App* actionRoot) {
      auto cancel_delay = actionRoot->add_subcommand("canceldelay", localized("Cancel a delayed transaction"));
      cancel_delay->add_option("canceling_account", canceling_account, localized("Account from authorization on the original delayed transaction"))->required();
      cancel_delay->add_option("canceling_permission", canceling_permission, localized("Permission from authorization on the original delayed transaction"))->required();
      cancel_delay->add_option("trx_id", trx_id, localized("The transaction id of the original delayed transaction"))->required();
      add_standard_transaction_options(cancel_delay);

      cancel_delay->set_callback([this] {
         const auto canceling_auth = permission_level{canceling_account, canceling_permission};
         fc::variant act_payload = fc::mutable_variant_object()
                  ("canceling_auth", canceling_auth)
                  ("trx_id", trx_id);
         send_actions({create_action({canceling_auth}, config::system_account_name, NEX(canceldelay), act_payload)});
      });
   }
};

struct empoweruser_subcommand {
   string user_account;
   string owner_public_key;
   string active_public_key;
   string chain_name;
   bool   updateable = true;
   bool   superprivflg = false;
   empoweruser_subcommand(CLI::App* actionRoot) {
      auto empower_user = actionRoot->add_subcommand("empoweruser", localized("Empower user's onwer&active permissions to a sidechain"));
      empower_user->add_option("user_account", user_account, localized("Account of the user to be empowered"))->required();
      empower_user->add_option("chain_name", chain_name, localized("The name of the sidechain which the user will be empowered to"))->required();
      empower_user->add_option("owner_public_key", owner_public_key, localized("Public key of owner permission"))->required();
      empower_user->add_option("active_public_key", active_public_key, localized("Public key of active permission"))->required();
      empower_user->add_option("updateable", updateable, localized("Set whether the account is updatable"))->required();
      empower_user->add_flag("-u,--superpriv", superprivflg, localized("empoweruser add privileged (rarely used)"));
      add_standard_transaction_options(empower_user);

      empower_user->set_callback([this] {
         fc::variant act_payload = fc::mutable_variant_object()
                  ("user", user_account)
                  ("chain_name", chain_name)
                  ("owner_pk", owner_public_key)
                  ("active_pk", active_public_key)
                  ("updateable", updateable);
         vector<permission_level> permiss_info;
         account_name auth_name = user_account;
         if(superprivflg)
            auth_name = N(ultrainio);
         permiss_info.push_back(permission_level{auth_name,config::active_name});
         send_actions({create_action(permiss_info, config::system_account_name, NEX(empoweruser), act_payload)});
      });
   }
};

struct put_order_subcommand {
   string owner_account;
   uint64_t period_id;
   uint16_t combosize;
   string price;
   bool   decrease = true;
   put_order_subcommand(CLI::App* actionRoot) {
      auto put_order = actionRoot->add_subcommand("putorder", localized("Put order to sell resource"));
      put_order->add_option("owner", owner_account, localized("Account of the user who want to sell resource"))->required();
      put_order->add_option("period", period_id, localized("The period id of the resource"))->required();
      put_order->add_option("combosize", combosize, localized("The amount of resources packages"))->required();
      put_order->add_option("price", price, localized("The unit price of resource package"))->required();
      put_order->add_option("decrease_by_day", decrease, localized("Set whether the price is decreasing by day"))->required();
      add_standard_transaction_options(put_order);

      put_order->set_callback([this] {
         fc::variant act_payload = fc::mutable_variant_object()
                  ("owner", owner_account)
                  ("period", period_id)
                  ("combosize", combosize)
                  ("price", price)
                  ("decrease", decrease);
         vector<permission_level> permiss_info;
         permiss_info.push_back(permission_level{owner_account,config::active_name});
         send_actions({create_action(permiss_info, N(utrio.res), NEX(putorder), act_payload)});
      });
   }
};

struct list_orders_subcommand {
   uint64_t period_id;
   list_orders_subcommand(CLI::App* actionRoot) {
      auto list_orders = actionRoot->add_subcommand("listorders", localized("List all resource sell orders of a specified period"));
      list_orders->add_option("period", period_id, localized("The period id of the resource"))->required();
      add_standard_transaction_options(list_orders);

      list_orders->set_callback([this] {
         auto rawResult = call(get_resource_orders_func, fc::mutable_variant_object("period_id", period_id));
         auto result = rawResult.as<vector<ultrainio::chain_apis::read_only::get_resource_orders_result>>();
         if(result.empty()) {
             std::cout << "No resource orders found" << std::endl;
             return;
         }
         printf("%-13s %-10s  %-10s  %-13s     %-13s    %-13s\n", "Owner", "Period", "Lease size", "initial_price", "daily decreasing","current_price");
         for ( const auto& row : result ) {
            printf("%-13.13s %-8lu    %-10d  %-9.4f UGAS    %-15.5s     %-9.4f UGAS\n",
                 row.owner.to_string().c_str(), period_id, row.lease_num,
                 double(row.initial_unit_price)/10000,
                 row.decrease_by_day ? "true" : "false",
                 double(row.current_price)/10000);
         }
      });
   }
};

struct update_order_subcommand {
   string owner_account;
   uint64_t period_id;
   string price;
   bool   decrease = true;
   update_order_subcommand(CLI::App* actionRoot) {
      auto update_order = actionRoot->add_subcommand("updateorder", localized("Update resource sell order"));
      update_order->add_option("owner", owner_account, localized("Account of the user who selling resource"))->required();
      update_order->add_option("period", period_id, localized("The period id of the resource"))->required();
      update_order->add_option("price", price, localized("The unit price of resource package"))->required();
      update_order->add_option("decrease_by_day", decrease, localized("Set whether the price is decreasing by day"))->required();
      add_standard_transaction_options(update_order);

      update_order->set_callback([this] {
         fc::variant act_payload = fc::mutable_variant_object()
                  ("owner", owner_account)
                  ("period", period_id)
                  ("price", price)
                  ("decrease", decrease);
         vector<permission_level> permiss_info;
         permiss_info.push_back(permission_level{owner_account,config::active_name});
         send_actions({create_action(permiss_info, N(utrio.res), NEX(updateorder), act_payload)});
      });
   }
};

struct cancel_order_subcommand {
   string owner_account;
   uint64_t period_id;
   cancel_order_subcommand(CLI::App* actionRoot) {
      auto cancel_order = actionRoot->add_subcommand("cancelorder", localized("Cancel resource sell order"));
      cancel_order->add_option("owner", owner_account, localized("Account of the user who selling resource"))->required();
      cancel_order->add_option("period", period_id, localized("The period id of the resource"))->required();
      add_standard_transaction_options(cancel_order);

      cancel_order->set_callback([this] {
         fc::variant act_payload = fc::mutable_variant_object()
                  ("owner", owner_account)
                  ("period", period_id);
         vector<permission_level> permiss_info;
         permiss_info.push_back(permission_level{owner_account,config::active_name});
         send_actions({create_action(permiss_info, N(utrio.res), NEX(cancelorder), act_payload)});
      });
   }
};

struct buy_in_subcommand {
   string owner_account;
   uint64_t period_id;
   string buyer_account;
   uint16_t combosize;
   buy_in_subcommand(CLI::App* actionRoot) {
      auto buy_in = actionRoot->add_subcommand("buyin", localized("Buy in resource"));
      buy_in->add_option("owner", owner_account, localized("Account of the user who selling the resource"))->required();
      buy_in->add_option("period", period_id, localized("The period id of the resource"))->required();
      buy_in->add_option("buyer", buyer_account, localized("Account of the user who buying the resource"))->required();
      buy_in->add_option("combosize", combosize, localized("The amount of resources packages the buyer "))->required();
      add_standard_transaction_options(buy_in);

      buy_in->set_callback([this] {
         fc::variant act_payload = fc::mutable_variant_object()
                  ("owner", owner_account)
                  ("period", period_id)
                  ("buyer", buyer_account)
                  ("combosize", combosize);
         vector<permission_level> permiss_info;
         permiss_info.push_back(permission_level{buyer_account,config::active_name});
         send_actions({create_action(permiss_info, N(utrio.res), NEX(buyin), act_payload)});
      });
   }
};

struct buy_resource_package_subcommand {
   string from_str;
   string receiver_str;
   int    combosize = 0;
   int    period_id = 0;
   string location;
   buy_resource_package_subcommand(CLI::App* actionRoot) {
      auto buy_respackage = actionRoot->add_subcommand("resourcelease", localized("buy resources packages"));
      buy_respackage->add_option("from", from_str, localized("The account that pays for resources packages"))->required();
      buy_respackage->add_option("receiver", receiver_str, localized("The account to receive the resources packages"))->required();
      buy_respackage->add_option("period", period_id, localized("period id(start from 1) of resource that receiver will own, 0 always indicates current period"))->required();
      buy_respackage->add_option("combosize", combosize, localized("The amount of resources packages"))->required();
      buy_respackage->add_option("location", location, localized("sidechain name that the resource lease located on"))->required();
      add_standard_transaction_options(buy_respackage);

      buy_respackage->set_callback([this] {
         fc::variant act_payload = fc::mutable_variant_object()
                  ("from", from_str)
                  ("receiver", receiver_str)
                  ("combosize", combosize)
                  ("period", period_id)
                  ("location", location);
         std::vector<chain::action> acts{create_action({permission_level{from_str,config::active_name}}, N(utrio.res), NEX(resourcelease), act_payload)};
         send_actions(std::move(acts));
      });
   }
};

struct transfer_resource_package_subcommand {
   string from_str;
   string to_str;
   int    combosize = 0;
   int    period_id = 0;
   string location;
   transfer_resource_package_subcommand(CLI::App* actionRoot) {
      auto transfer_respackage = actionRoot->add_subcommand("transresource", localized("transfer resources packages"));
      transfer_respackage->add_option("from", from_str, localized("The account that transfer resources packages from"))->required();
      transfer_respackage->add_option("to", to_str, localized("The account to receive the resources packages"))->required();
      transfer_respackage->add_option("period", period_id, localized("period id(start from 1) of resource that receiver will own, 0 always indicates current period"))->required();
      transfer_respackage->add_option("combosize", combosize, localized("The amount of resources packages"))->required();
      //transfer_respackage->add_option("location", location, localized("sidechain name that the resource lease located on"))->required();
      add_standard_transaction_options(transfer_respackage);

      transfer_respackage->set_callback([this] {
         fc::variant act_payload = fc::mutable_variant_object()
                  ("from", from_str)
                  ("to", to_str)
                  ("combosize", combosize)
                  ("period", period_id);
         std::vector<chain::action> acts{create_action({permission_level{from_str,config::active_name}}, N(utrio.res), NEX(transresource), act_payload)};
         send_actions(std::move(acts));
      });
   }
};

void get_account( const string& accountName, bool json_format ) {
   auto json = call(get_account_func, fc::mutable_variant_object("account_name", accountName));
   auto res = json.as<ultrainio::chain_apis::read_only::get_account_results>();

   if (!json_format) {
      asset staked;
      asset unstaking;
      asset cons_staked;
      if(res.privileged) std::cout << "privileged: true" << std::endl;

      constexpr size_t indent_size = 5;
      const string indent(indent_size, ' ');

      std::cout << "permissions: " << std::endl;
      unordered_map<name, vector<name>/*children*/> tree;
      vector<name> roots; //we don't have multiple roots, but we can easily handle them here, so let's do it just in case
      unordered_map<name, ultrainio::chain_apis::permission> cache;
      for ( auto& perm : res.permissions ) {
         if ( perm.parent ) {
            tree[perm.parent].push_back( perm.perm_name );
         } else {
            roots.push_back( perm.perm_name );
         }
         auto name = perm.perm_name; //keep copy before moving `perm`, since thirst argument of emplace can be evaluated first
         // looks a little crazy, but should be efficient
         cache.insert( std::make_pair(name, std::move(perm)) );
      }
      std::function<void (account_name, int)> dfs_print = [&]( account_name name, int depth ) -> void {
         auto& p = cache.at(name);
         std::cout << indent << std::string(depth*3, ' ') << name << ' ' << std::setw(5) << p.required_auth.threshold << ":    ";
         for ( auto it = p.required_auth.keys.begin(); it != p.required_auth.keys.end(); ++it ) {
            if ( it != p.required_auth.keys.begin() ) {
               std::cout  << ", ";
            }
            std::cout << it->weight << ' ' << string(it->key);
         }
         for ( auto& acc : p.required_auth.accounts ) {
            std::cout << acc.weight << ' ' << string(acc.permission.actor) << '@' << string(acc.permission.permission) << ", ";
         }
         std::cout << std::endl;
         auto it = tree.find( name );
         if (it != tree.end()) {
            auto& children = it->second;
            sort( children.begin(), children.end() );
            for ( auto& n : children ) {
               // we have a tree, not a graph, so no need to check for already visited nodes
               dfs_print( n, depth+1 );
            }
         } // else it's a leaf node
      };
      std::sort(roots.begin(), roots.end());
      for ( auto r : roots ) {
         dfs_print( r, 0 );
      }

      auto to_pretty_net = []( int64_t nbytes, uint8_t width_for_units = 5 ) {
         if(nbytes == -1) {
             // special case. Treat it as unlimited
             return std::string("unlimited");
         }

         string unit = "bytes";
         double bytes = static_cast<double> (nbytes);
         if (bytes >= 1024 * 1024 * 1024 * 1024ll) {
             unit = "TiB";
             bytes /= 1024 * 1024 * 1024 * 1024ll;
         } else if (bytes >= 1024 * 1024 * 1024) {
             unit = "GiB";
             bytes /= 1024 * 1024 * 1024;
         } else if (bytes >= 1024 * 1024) {
             unit = "MiB";
             bytes /= 1024 * 1024;
         } else if (bytes >= 1024) {
             unit = "KiB";
             bytes /= 1024;
         }
         std::stringstream ss;
         ss << setprecision(4);
         ss << bytes << " ";
         if( width_for_units > 0 )
            ss << std::left << setw( width_for_units );
         ss << unit;
         return ss.str();
      };
      std::cout << "contract updateable: " << res.updateable << std::endl;
      if ( !res.empowerchains.empty() ) {
         std::cout << "empower chains: ";
         for ( auto chain_name:res.empowerchains ) {
            std::cout << " " << chain_name;
         }
         std::cout << std::endl;
      }
      if ( res.chain_resource.size() > 0) {
         std::cout << std::endl;
         std::cout << "resource lease: " << std::endl;
         for(auto chainres : res.chain_resource){
            std::cout << std::fixed << setprecision(3);
            std::cout << indent << std::left << std::setw(11) << "location:"      << std::right << std::setw(13) << chainres.chain_name << "\n";
            std::cout << indent << std::left << std::setw(11) << "lease_num:"      << std::right << std::setw(13) << chainres.lease_num << "\n";
            std::cout << indent << std::left << std::setw(11) << "start_time:" << std::right << std::setw(13) << chainres.start_time << "\n";
            std::cout << indent << std::left << std::setw(11) << "end_time:"     << std::right << std::setw(14) << chainres.end_time << "\n";
            std::cout << std::endl;
         }
      }

      std::cout << "memory: " << std::endl
                << indent << "quota: " << std::setw(15) << to_pretty_net(res.ram_quota) << "  used: " << std::setw(15) << to_pretty_net(res.ram_usage) << std::endl << std::endl;

      std::cout << "net bandwidth: " << std::endl;

      auto to_pretty_time = []( int64_t nmicro, uint8_t width_for_units = 5 ) {
         if(nmicro == -1) {
             // special case. Treat it as unlimited
             return std::string("unlimited");
         }
         string unit = "us";
         double micro = static_cast<double>(nmicro);

         if( micro > 1000000*60*60ll ) {
            micro /= 1000000*60*60ll;
            unit = "hr";
         }
         else if( micro > 1000000*60 ) {
            micro /= 1000000*60;
            unit = "min";
         }
         else if( micro > 1000000 ) {
            micro /= 1000000;
            unit = "sec";
         }
         else if( micro > 1000 ) {
            micro /= 1000;
            unit = "ms";
         }
         std::stringstream ss;
         ss << setprecision(4);
         ss << micro << " ";
         if( width_for_units > 0 )
            ss << std::left << setw( width_for_units );
         ss << unit;
         return ss.str();
      };


      std::cout << std::fixed << setprecision(3);
      std::cout << indent << std::left << std::setw(11) << "used:"      << std::right << std::setw(18) << to_pretty_net( res.net_limit.used ) << "\n";
      std::cout << indent << std::left << std::setw(11) << "available:" << std::right << std::setw(18) << to_pretty_net( res.net_limit.available ) << "\n";
      std::cout << indent << std::left << std::setw(11) << "limit:"     << std::right << std::setw(18) << to_pretty_net( res.net_limit.max ) << "\n";
      std::cout << std::endl;

      std::cout << "cpu bandwidth:" << std::endl;

      std::cout << std::fixed << setprecision(3);
      std::cout << indent << std::left << std::setw(11) << "used:"      << std::right << std::setw(18) << to_pretty_time( res.cpu_limit.used ) << "\n";
      std::cout << indent << std::left << std::setw(11) << "available:" << std::right << std::setw(18) << to_pretty_time( res.cpu_limit.available ) << "\n";
      std::cout << indent << std::left << std::setw(11) << "limit:"     << std::right << std::setw(18) << to_pretty_time( res.cpu_limit.max ) << "\n";
      std::cout << std::endl;

      if ( res.producer_info.is_object() ) {
         std::cout << "consensus weight:" << std::endl;
         auto& obj = res.producer_info.get_object();
         uint64_t total_cons_staked = obj["total_cons_staked"].as_uint64();
         asset total_cons = asset(total_cons_staked);
         if( res.self_delegated_consensus.is_object() ) {
            asset cons_own = asset::from_string( res.self_delegated_consensus.get_object()["cons_weight"].as_string() );
            staked += cons_own;
            auto cons_others = total_cons - cons_own;

            std::cout << indent << "staked:" << std::setw(20) << cons_own
                      << std::string(11, ' ') << "(total stake delegated from account to self)" << std::endl
                      << indent << "delegated:" << std::setw(17) << cons_others
                      << std::string(11, ' ') << "(total staked delegated to account from others)" << std::endl;
         } else {
            auto cons_others = total_cons;
            std::cout << indent << "delegated:" << std::setw(17) << cons_others
                      << std::string(11, ' ') << "(total staked delegated to account from others)" << std::endl;
         }
         std::cout << std::endl;
      }

      if ( res.refund_cons.is_object() ) {
         auto& obj = res.refund_cons.get_object();
         auto request_time = fc::time_point_sec::from_iso_string( obj["request_time"].as_string() );
         fc::time_point refund_time = request_time + fc::days(30);
         auto now = res.head_block_time;
         asset cons = asset::from_string( obj["cons_amount"].as_string() );
         unstaking += cons;

         if( unstaking > asset( 0, unstaking.get_symbol() ) ) {
            std::cout << std::fixed << setprecision(3);
            std::cout << "cons unstaking tokens:" << std::endl;
            std::cout << indent << std::left << std::setw(25) << "time of cons unstake request:" << std::right << std::setw(20) << string(request_time);
            if( now >= refund_time ) {
               std::cout << " (available to claim now with 'ultrainio::refundcons' action)\n";
            } else {
               std::cout << " (funds will be available in " << to_pretty_time( (refund_time - now).count(), 0 ) << ")\n";
            }
            std::cout << indent << std::left << std::setw(25) << "from cons:" << std::right << std::setw(18) << cons << std::endl;
            std::cout << std::endl;
         }
      }

      if( res.core_liquid_balance.valid() ) {
         std::cout << res.core_liquid_balance->get_symbol().name() << " balances: " << std::endl;
         std::cout << indent << std::left << std::setw(11)
                   << "liquid:" << std::right << std::setw(18) << *res.core_liquid_balance << std::endl;
         std::cout << indent << std::left << std::setw(11)
                   << "staked:" << std::right << std::setw(18) << staked << std::endl;
         std::cout << indent << std::left << std::setw(11)
                   << "unstaking:" << std::right << std::setw(18) << unstaking << std::endl;
         std::cout << indent << std::left << std::setw(11) << "total:" << std::right << std::setw(18) << (*res.core_liquid_balance + staked + unstaking) << std::endl;
         std::cout << std::endl;
      }
      std::cout << std::endl;
   } else {
      std::cout << fc::json::to_pretty_string(json) << std::endl;
   }
}

CLI::callback_t addl_wallet_callback = [](CLI::results_t res) {
   vector<string>::iterator itr;

   for (itr = res.begin(); itr != res.end(); itr++) {
       addl_wallet_urls.push_back(*itr);
   }
   fc::deduplicate(addl_wallet_urls);
   auto iter = std::find(addl_wallet_urls.begin(), addl_wallet_urls.end(), wallet_url);
   if (iter != addl_wallet_urls.end()) {
       addl_wallet_urls.erase(iter);
   }

   return true;
};

CLI::callback_t header_opt_callback = [](CLI::results_t res) {
   vector<string>::iterator itr;

   for (itr = res.begin(); itr != res.end(); itr++) {
       headers.push_back(*itr);
   }

   return true;
};

int main( int argc, char** argv ) {
   setlocale(LC_ALL, "");
   bindtextdomain(locale_domain, locale_path);
   textdomain(locale_domain);
   context = ultrainio::client::http::create_http_context();

   fc::microseconds abi_serializer_max_time = fc::seconds(1); // No risk to client side serialization taking a long time

   CLI::App app{"Command Line Interface to UTRIO Client"};
   app.require_subcommand();
   app.add_option( "-H,--host", obsoleted_option_host_port, localized("the host where nodultrain is running") )->group("hidden");
   app.add_option( "-p,--port", obsoleted_option_host_port, localized("the port where nodultrain is running") )->group("hidden");
   app.add_option( "--wallet-host", obsoleted_option_host_port, localized("the host where kultraind is running") )->group("hidden");
   app.add_option( "--wallet-port", obsoleted_option_host_port, localized("the port where kultraind is running") )->group("hidden");

   app.add_option( "-u,--url", ::url, localized("the http/https URL where nodultrain is running"), true );
   app.add_option( "--wallet-url", wallet_url, localized("the http/https URL where kultraind is running"), true );
   app.add_option( "--addl-wallet-url", addl_wallet_callback, localized("pass more wallet URLs; repeat this option to pass multiple urls"));

   app.add_option( "-r,--header", header_opt_callback, localized("pass specific HTTP header; repeat this option to pass multiple headers"));
   app.add_flag( "-n,--no-verify", no_verify, localized("don't verify peer certificate when using HTTPS"));
   app.set_callback([&app]{ ensure_kultraind_running(&app);});

   bool verbose_errors = false;
   app.add_flag( "-v,--verbose", verbose_errors, localized("output verbose actions on error"));
   app.add_flag("--print-request", print_request, localized("print HTTP request to STDERR"));
   app.add_flag("--print-response", print_response, localized("print HTTP response to STDERR"));

   auto version = app.add_subcommand("version", localized("Retrieve version information"), false);
   version->require_subcommand();

   version->add_subcommand("client", localized("Retrieve version information of the client"))->set_callback([] {
     std::cout << localized("Build version: ${ver}", ("ver", ultrainio::client::config::version_str)) << std::endl;
   });

   // Create subcommand
   auto create = app.add_subcommand("create", localized("Create various items, on and off the blockchain"), false);
   create->require_subcommand();

   bool r1 = false;
   int  keysize = 1;
   // create key
   auto create_key = create->add_subcommand("key", localized("Create a new keypair and print the public and private keys"))->set_callback( [&](){
        vector<std::pair<string,string>> keypairvec;
        string privs,pubs;
        for(int i = 0; i < keysize; i++){
            if( r1 ) {
                auto pk    = private_key_type::generate_r1();
                privs = string(pk);
                pubs  = string(pk.get_public_key());
            } else {
                auto pk    = private_key_type::generate();
                privs = string(pk);
                pubs  = string(pk.get_public_key());
            }
            std::cout << localized("Private key: ${key}", ("key",  privs) ) << std::endl;
            std::cout << localized("Public key: ${key}", ("key", pubs ) ) << std::endl;
            std::cout << std::endl;
            keypairvec.push_back(std::pair<string,string>(privs,pubs));
        }
        if(keysize > 1){
            std::cout << std::endl;
            std::cout <<"pri keys:"<< std::endl;
            for(auto keypair:keypairvec){
                std::cout << "\"" << keypair.first << "\"," <<std::endl;
            }
            std::cout << std::endl;
            std::cout <<"pub keys:"<< std::endl;
            for(auto keypair:keypairvec){
                std::cout << "\"" << keypair.second << "\"," << std::endl;
            }
        }
   });
   create_key->add_option("keysize", keysize, localized("create key sizes"));
   create_key->add_flag( "--r1", r1, "Generate a key using the R1 curve (iPhone), instead of the K1 curve (Bitcoin)"  );

   // create account
   auto createAccount = create_account_subcommand( create );

   // Get subcommand
   auto get = app.add_subcommand("get", localized("Retrieve various items and information from the blockchain"), false);
   get->require_subcommand();

   // get info
   get->add_subcommand("info", localized("Get current blockchain information"))->set_callback([] {
      std::cout << fc::json::to_pretty_string(get_info()) << std::endl;
   });

   // get block
   string blockArg;
   bool get_bhs = false;
   auto getBlock = get->add_subcommand("block", localized("Retrieve a full block from the blockchain"), false);
   getBlock->add_option("block", blockArg, localized("The number or ID of the block to retrieve"))->required();
   getBlock->add_flag("--header-state", get_bhs, localized("Get block header state from fork database instead") );
   getBlock->set_callback([&blockArg,&get_bhs] {
      auto arg = fc::mutable_variant_object("block_num_or_id", blockArg);
      if( get_bhs ) {
         std::cout << fc::json::to_pretty_string(call(get_block_header_state_func, arg)) << std::endl;
      } else {
         std::cout << fc::json::to_pretty_string(call(get_block_func, arg)) << std::endl;
      }
   });

   // get account
   string accountName;
   bool print_json;
   auto getAccount = get->add_subcommand("account", localized("Retrieve an account from the blockchain"), false);
   getAccount->add_option("name", accountName, localized("The name of the account to retrieve"))->required();
   getAccount->add_flag("--json,-j", print_json, localized("Output in JSON format") );
   getAccount->set_callback([&]() { get_account(accountName, print_json); });

   // get code
   string codeFilename;
   string abiFilename;
   bool code_as_wasm = false;
   auto getCode = get->add_subcommand("code", localized("Retrieve the code and ABI for an account"), false);
   getCode->add_option("name", accountName, localized("The name of the account whose code should be retrieved"))->required();
   getCode->add_option("-c,--code",codeFilename, localized("The name of the file to save the contract .wast/wasm to") );
   getCode->add_option("-a,--abi",abiFilename, localized("The name of the file to save the contract .abi to") );
   getCode->add_flag("--wasm", code_as_wasm, localized("Save contract as wasm"));
   getCode->set_callback([&] {
      string code_hash, wasm, wast, abi;
      try {
         const auto result = call(get_raw_code_and_abi_func, fc::mutable_variant_object("account_name", accountName));
         const std::vector<char> wasm_v = result["wasm"].as_blob().data;
         const std::vector<char> abi_v = result["abi"].as_blob().data;

         fc::sha256 hash;
         if(wasm_v.size())
            hash = fc::sha256::hash(wasm_v.data(), wasm_v.size());
         code_hash = (string)hash;

         wasm = string(wasm_v.begin(), wasm_v.end());
         if(!code_as_wasm && wasm_v.size())
            wast = wasm_to_wast((const uint8_t*)wasm_v.data(), wasm_v.size(), false);

         abi_def abi_d;
         if(abi_serializer::to_abi(abi_v, abi_d))
            abi = fc::json::to_pretty_string(abi_d);
      }
      catch(chain::missing_chain_api_plugin_exception&) {
         //see if this is an old nodultrain that doesn't support get_raw_code_and_abi
         const auto old_result = call(get_code_func, fc::mutable_variant_object("account_name", accountName)("code_as_wasm",code_as_wasm));
         code_hash = old_result["code_hash"].as_string();
         if(code_as_wasm) {
            wasm = old_result["wasm"].as_string();
            std::cout << localized("Warning: communicating to older nodultrain which returns malformed binary wasm") << std::endl;
         }
         else
            wast = old_result["wast"].as_string();
         abi = fc::json::to_pretty_string(old_result["abi"]);
      }

      std::cout << localized("code hash: ${code_hash}", ("code_hash", code_hash)) << std::endl;

      if( codeFilename.size() ){
         std::cout << localized("saving ${type} to ${codeFilename}", ("type", (code_as_wasm ? "wasm" : "wast"))("codeFilename", codeFilename)) << std::endl;

         std::ofstream out( codeFilename.c_str() );
         if(code_as_wasm)
            out << wasm;
         else
            out << wast;
      }
      if( abiFilename.size() ) {
         std::cout << localized("saving abi to ${abiFilename}", ("abiFilename", abiFilename)) << std::endl;
         std::ofstream abiout( abiFilename.c_str() );
         abiout << abi;
      }
   });

   // get abi
   string filename;
   auto getAbi = get->add_subcommand("abi", localized("Retrieve the ABI for an account"), false);
   getAbi->add_option("name", accountName, localized("The name of the account whose abi should be retrieved"))->required();
   getAbi->add_option("-f,--file",filename, localized("The name of the file to save the contract .abi to instead of writing to console") );
   getAbi->set_callback([&] {
      auto result = call(get_abi_func, fc::mutable_variant_object("account_name", accountName));

      auto abi  = fc::json::to_pretty_string( result["abi"] );
      if( filename.size() ) {
         std::cerr << localized("saving abi to ${filename}", ("filename", filename)) << std::endl;
         std::ofstream abiout( filename.c_str() );
         abiout << abi;
      } else {
         std::cout << abi << "\n";
      }
   });

   // get table
   string scope;
   string code;
   string table;
   string lower;
   string upper;
   string table_key_type;
   string table_key;
   string scope_type;
   string key_type;
   bool binary = false;
   uint32_t limit = 10;
   string index_position;
   auto getTable = get->add_subcommand( "table", localized("Retrieve the contents of a database table"), false);
   getTable->add_option( "contract", code, localized("The contract who owns the table") )->required();
   getTable->add_option( "scope", scope, localized("The scope within the contract in which the table is found") )->required();
   getTable->add_option( "table", table, localized("The name of the table as specified by the contract abi") )->required();
   getTable->add_option( "-b,--binary", binary, localized("Return the value as BINARY rather than using abi to interpret as JSON") );
   getTable->add_option( "-l,--limit", limit, localized("The maximum number of rows to return") );
   getTable->add_option( "-p,--type", table_key_type, localized("select table key type") );
   getTable->add_option( "-k,--key", table_key, localized("filter index key") );
   getTable->add_option( "-s,--scopetype", scope_type, localized("scope type:name,symbol,uint64. default sequential conversion") );
   getTable->add_option( "-L,--lower", lower, localized("JSON representation of lower bound value of key, defaults to first") );
   getTable->add_option( "-U,--upper", upper, localized("JSON representation of upper bound value value of key, defaults to last") );
   getTable->add_option( "--index", index_position,
                         localized("Index number, 1 - primary (first), 2 - secondary index (in order defined by multi_index), 3 - third index, etc.\n"
                                   "\t\t\t\tNumber or name of index can be specified, e.g. 'secondary' or '2'."));
   getTable->add_option( "--key-type", key_type,
                         localized("The key type of --index, primary only supports (i64), all others support (i64, i128, i256, float64, float128).\n"
                                   "\t\t\t\tSpecial type 'name' indicates an account name."));

   getTable->set_callback([&] {
      auto result = call(get_table_func, fc::mutable_variant_object("json", !binary)
                         ("code",code)
                         ("scope",scope)
                         ("table",table)
                         ("table_key_type",table_key_type)
                         ("table_key",table_key)
                         ("scope_type",scope_type)
                         ("lower_bound",lower)
                         ("upper_bound",upper)
                         ("limit",limit)
                         ("key_type",key_type)
                         ("index_position", index_position)
                         );

      std::cout << fc::json::to_pretty_string(result)
                << std::endl;
   });
   bool scope_to_name = false;
   auto getScope = get->add_subcommand( "scope", localized("Retrieve a list of scopes and tables owned by a contract"), false);
   getScope->add_option( "contract", code, localized("The contract who owns the table") )->required();
   getScope->add_option( "-t,--table", table, localized("The name of the table as filter") );
   getScope->add_option( "-l,--limit", limit, localized("The maximum number of rows to return") );
   getScope->add_option( "-L,--lower", lower, localized("lower bound of scope") );
   getScope->add_option( "-U,--upper", upper, localized("upper bound of scope") );
   getScope->add_flag("-n,--name", scope_to_name, localized("scope to name string from uint64"));
   getScope->set_callback([&] {
      auto result = call(get_table_by_scope_func, fc::mutable_variant_object("code",code)
                         ("table",table)
                         ("lower_bound",lower)
                         ("upper_bound",upper)
                         ("limit",limit)
                         );
      if(scope_to_name){
         fc::variants rows = result.get_object()["rows"].get_array();
         if (rows.empty()) {
            std::cerr << "table not data" << std::endl;
            return;
         }
         auto const get_symbol_name = [](uint64_t value)-> string{
            uint64_t v = value;
            string result;
            while (v > 0) {
               char c = v & 0xFF;
               result += c;
               v >>= 8;
            }
            return result;
         };
         for(auto row : rows){
            std::string scope;
            if(std::string("stat") == row["table"].as_string()){
               scope = get_symbol_name(row["scope"].as_uint64());
            }else{
               scope = name{row["scope"].as_uint64()}.to_string();
            }
            std::cout << "code: "<< row["code"].as_string() << std::endl
                     << "scope: "<< scope << std::endl
                     << "table: "<< row["table"].as_string() << std::endl
                     << "payer: "<< row["payer"].as_string() << std::endl
                     << "count: "<< row["count"].as_string() << std::endl
                     << std::endl;
         }
         std::cout << "row size: "<< rows.size() << std::endl;
         fc::variant morevalue = result.get_object()["more"];
         std::cout << "more: " << morevalue.as_string()
               << std::endl;
      } else
         std::cout << fc::json::to_pretty_string(result)
               << std::endl;
   });

   // currency accessors
   // get currency balance
   string symbol;
   auto get_currency = get->add_subcommand( "currency", localized("Retrieve information related to standard currencies"), true);
   get_currency->require_subcommand();
   auto get_balance = get_currency->add_subcommand( "balance", localized("Retrieve the balance of an account for a given currency"), false);
   get_balance->add_option( "contract", code, localized("The contract that operates the currency") )->required();
   get_balance->add_option( "account", accountName, localized("The account to query balances for") )->required();
   get_balance->add_option( "symbol", symbol, localized("The symbol for the currency if the contract operates multiple currencies") );
   get_balance->set_callback([&] {
      auto result = call(get_currency_balance_func, fc::mutable_variant_object
         ("account", accountName)
         ("code", code)
         ("symbol", symbol.empty() ? fc::variant() : symbol)
      );

      const auto& rows = result.get_array();
      for( const auto& r : rows ) {
         std::cout << r.as_string()
                   << std::endl;
      }
   });

   auto get_currency_stats = get_currency->add_subcommand( "stats", localized("Retrieve the stats of for a given currency"), false);
   get_currency_stats->add_option( "contract", code, localized("The contract that operates the currency") )->required();
   get_currency_stats->add_option( "symbol", symbol, localized("The symbol for the currency if the contract operates multiple currencies") )->required();
   get_currency_stats->set_callback([&] {
      auto result = call(get_currency_stats_func, fc::mutable_variant_object("json", false)
         ("code", code)
         ("symbol", symbol)
      );

      std::cout << fc::json::to_pretty_string(result)
                << std::endl;
   });

   // get accounts
   string public_key_str;
   auto getAccounts = get->add_subcommand("accounts", localized("Retrieve accounts associated with a public key"), false);
   getAccounts->add_option("public_key", public_key_str, localized("The public key to retrieve accounts for"))->required();
   getAccounts->set_callback([&] {
      public_key_type public_key;
      try {
         public_key = public_key_type(public_key_str);
      } ULTRAIN_RETHROW_EXCEPTIONS(public_key_type_exception, "Invalid public key: ${public_key}", ("public_key", public_key_str))
      auto arg = fc::mutable_variant_object( "public_key", public_key);
      std::cout << fc::json::to_pretty_string(call(get_key_accounts_func, arg)) << std::endl;
   });

   // get servants
   string controllingAccount;
   auto getServants = get->add_subcommand("servants", localized("Retrieve accounts which are servants of a given account "), false);
   getServants->add_option("account", controllingAccount, localized("The name of the controlling account"))->required();
   getServants->set_callback([&] {
      auto arg = fc::mutable_variant_object( "controlling_account", controllingAccount);
      std::cout << fc::json::to_pretty_string(call(get_controlled_accounts_func, arg)) << std::endl;
   });

   // get transaction
   string transaction_id_str;
   uint32_t block_num_hint = 0;
   auto getTransaction = get->add_subcommand("transaction", localized("Retrieve a transaction from the blockchain"), false);
   getTransaction->add_option("id", transaction_id_str, localized("ID of the transaction to retrieve"))->required();
   getTransaction->add_option( "-b,--block-hint", block_num_hint, localized("the block number this transaction may be in") );
   getTransaction->set_callback([&] {
      transaction_id_type transaction_id;
      try {
         while( transaction_id_str.size() < 64 ) transaction_id_str += "0";
         transaction_id = transaction_id_type(transaction_id_str);
      } ULTRAIN_RETHROW_EXCEPTIONS(transaction_id_type_exception, "Invalid transaction ID: ${transaction_id}", ("transaction_id", transaction_id_str))
      auto arg= fc::mutable_variant_object( "id", transaction_id);
      if ( block_num_hint > 0 ) {
         arg = arg("block_num_hint", block_num_hint);
      }
      std::cout << fc::json::to_pretty_string(call(get_transaction_func, arg)) << std::endl;
   });

   // get actions
   string account_name;
   string skip_seq_str;
   string num_seq_str;
   bool printjson = false;
   bool fullact = false;
   bool prettyact = false;
   bool printconsole = false;

   int32_t pos_seq = -1;
   int32_t offset = -20;
   auto getActions = get->add_subcommand("actions", localized("Retrieve all actions with specific account name referenced in authorization or receiver"), false);
   getActions->add_option("account_name", account_name, localized("name of account to query on"))->required();
   getActions->add_option("pos", pos_seq, localized("sequence number of action for this account, -1 for last"));
   getActions->add_option("offset", offset, localized("get actions [pos,pos+offset] for positive offset or [pos-offset,pos) for negative offset"));
   getActions->add_flag("--json,-j", printjson, localized("print full json"));
   getActions->add_flag("--full", fullact, localized("don't truncate action json"));
   getActions->add_flag("--pretty", prettyact, localized("pretty print full action json "));
   getActions->add_flag("--console", printconsole, localized("print console output generated by action "));
   getActions->set_callback([&] {
      fc::mutable_variant_object arg;
      arg( "account_name", account_name );
      arg( "pos", pos_seq );
      arg( "offset", offset);

      auto result = call(get_actions_func, arg);


      if( printjson ) {
         std::cout << fc::json::to_pretty_string(result) << std::endl;
      } else {
          auto& traces = result["actions"].get_array();
          uint32_t lib = result["last_irreversible_block"].as_uint64();


          cout  << "#" << setw(5) << "seq" << "  " << setw( 24 ) << left << "when"<< "  " << setw(24) << right << "contract::action" << " => " << setw(13) << left << "receiver" << " " << setw(11) << left << "trx id..." << " args\n";
          cout  << "================================================================================================================\n";
          for( const auto& trace: traces ) {
              std::stringstream out;
              if( trace["block_num"].as_uint64() <= lib )
                 out << "#";
              else
                 out << "?";

              out << setw(5) << trace["account_action_seq"].as_uint64() <<"  ";
              out << setw(24) << trace["block_time"].as_string() <<"  ";

              const auto& at = trace["action_trace"].get_object();

              auto id = at["trx_id"].as_string();
              const auto& receipt = at["receipt"];
              auto receiver = receipt["receiver"].as_string();
              const auto& act = at["act"].get_object();
              auto code = act["account"].as_string();
              auto func = act["name"].as_string();
              string args;
              if( prettyact ) {
                  args = fc::json::to_pretty_string( act["data"] );
              }
              else {
                 args = fc::json::to_string( act["data"] );
                 if( !fullact ) {
                    args = args.substr(0,60) + "...";
                 }
              }
              out << std::setw(24) << std::right<< (code +"::" + func) << " => " << left << std::setw(13) << receiver;

              out << " " << setw(11) << (id.substr(0,8) + "...");

              if( fullact || prettyact ) out << "\n";
              else out << " ";

              out << args ;//<< "\n";

              if( trace["block_num"].as_uint64() <= lib ) {
                 dlog( "\r${m}", ("m",out.str()) );
              } else {
                 wlog( "\r${m}", ("m",out.str()) );
              }
              if( printconsole ) {
                 auto console = at["console"].as_string();
                 if( console.size() ) {
                    stringstream out;
                    std::stringstream ss(console);
                    string line;
                    std::getline( ss, line );
                    out << ">> " << line << "\n";
                    cerr << out.str(); //ilog( "\r${m}                                   ", ("m",out.str()) );
                 }
              }
          }
      }
   });

   /*
   auto getTransactions = get->add_subcommand("transactions", localized("Retrieve all transactions with specific account name referenced in their scope"), false);
   getTransactions->add_option("account_name", account_name, localized("name of account to query on"))->required();
   getTransactions->add_option("skip_seq", skip_seq_str, localized("Number of most recent transactions to skip (0 would start at most recent transaction)"));
   getTransactions->add_option("num_seq", num_seq_str, localized("Number of transactions to return"));
   getTransactions->add_flag("--json,-j", printjson, localized("print full json"));
   getTransactions->set_callback([&] {
      fc::mutable_variant_object arg;
      if (skip_seq_str.empty()) {
         arg = fc::mutable_variant_object( "account_name", account_name);
      } else {
         uint64_t skip_seq;
         try {
            skip_seq = boost::lexical_cast<uint64_t>(skip_seq_str);
         } ULTRAIN_RETHROW_EXCEPTIONS(chain_type_exception, "Invalid Skip Seq: ${skip_seq}", ("skip_seq", skip_seq_str))
         if (num_seq_str.empty()) {
            arg = fc::mutable_variant_object( "account_name", account_name)("skip_seq", skip_seq);
         } else {
            uint64_t num_seq;
            try {
               num_seq = boost::lexical_cast<uint64_t>(num_seq_str);
            } ULTRAIN_RETHROW_EXCEPTIONS(chain_type_exception, "Invalid Num Seq: ${num_seq}", ("num_seq", num_seq_str))
            arg = fc::mutable_variant_object( "account_name", account_name)("skip_seq", skip_seq_str)("num_seq", num_seq);
         }
      }
      auto result = call(get_transactions_func, arg);
      if( printjson ) {
         std::cout << fc::json::to_pretty_string(result) << std::endl;
      }
      else {
         const auto& trxs = result.get_object()["transactions"].get_array();
         for( const auto& t : trxs ) {

            const auto& tobj = t.get_object();
            const auto& trx  = tobj["transaction"].get_object();
            const auto& data = trx["transaction"].get_object();
            const auto& msgs = data["actions"].get_array();

            for( const auto& msg : msgs ) {
               int64_t seq_num  = tobj["seq_num"].as<int64_t>();
               string  id       = tobj["transaction_id"].as_string();
               const auto& exp  = data["expiration"].as<fc::time_point_sec>();
               std::cout << tobj["seq_num"].as_string() <<"] " << id.substr(0,8) << "...  " << data["expiration"].as_string() << "  ";
               auto code = msg["account"].as_string();
               auto func = msg["name"].as_string();
               auto args = fc::json::to_string( msg["data"] );
               std::cout << setw(26) << left << (code + "::" + func) << "  " << args;
               std::cout << std::endl;
            }
         }
      }

   });
   */

   // set subcommand
   auto setSubcommand = app.add_subcommand("set", localized("Set or update blockchain state"));
   setSubcommand->require_subcommand();

   // set contract subcommand
   string account;
   string contractPath;
   string wastPath;
   string abiPath;
   bool shouldSend = true;
   auto codeSubcommand = setSubcommand->add_subcommand("code", localized("Create or update the code on an account"));
   codeSubcommand->add_option("account", account, localized("The account to set code for"))->required();
   codeSubcommand->add_option("code-file", wastPath, localized("The fullpath containing the contract WAST or WASM"))->required();

   auto abiSubcommand = setSubcommand->add_subcommand("abi", localized("Create or update the abi on an account"));
   abiSubcommand->add_option("account", account, localized("The account to set the ABI for"))->required();
   abiSubcommand->add_option("abi-file", abiPath, localized("The fullpath containing the contract WAST or WASM"))->required();

   auto contractSubcommand = setSubcommand->add_subcommand("contract", localized("Create or update the contract on an account"));
   contractSubcommand->add_option("account", account, localized("The account to publish a contract for"))
                     ->required();
   contractSubcommand->add_option("contract-dir", contractPath, localized("The path containing the .wast and .abi"))
                     ->required();
   contractSubcommand->add_option("wast-file", wastPath, localized("The file containing the contract WAST or WASM relative to contract-dir"));
//                     ->check(CLI::ExistingFile);
   auto abi = contractSubcommand->add_option("abi-file,-a,--abi", abiPath, localized("The ABI for the contract relative to contract-dir"));
//                                ->check(CLI::ExistingFile);

   std::vector<chain::action> actions;
   auto set_code_callback = [&]() {
      std::string wast;
      fc::path cpath(contractPath);

      if( cpath.filename().generic_string() == "." ) cpath = cpath.parent_path();

      if( wastPath.empty() )
      {
         wastPath = (cpath / (cpath.filename().generic_string()+".wasm")).generic_string();
         if (!fc::exists(wastPath))
            wastPath = (cpath / (cpath.filename().generic_string()+".wast")).generic_string();
      }

      std::cerr << localized(("Reading WAST/WASM from " + wastPath + "...").c_str()) << std::endl;
      fc::read_file_contents(wastPath, wast);
      ULTRAIN_ASSERT( !wast.empty(), wast_file_not_found, "no wast file found ${f}", ("f", wastPath) );
      vector<uint8_t> wasm;
      const string binary_wasm_header("\x00\x61\x73\x6d", 4);
      if(wast.compare(0, 4, binary_wasm_header) == 0) {
         std::cerr << localized("Using already assembled WASM...") << std::endl;
         wasm = vector<uint8_t>(wast.begin(), wast.end());
      }
      else {
         std::cerr << localized("Assembling WASM...") << std::endl;
         wasm = wast_to_wasm(wast);
      }

      actions.emplace_back( create_setcode(account, bytes(wasm.begin(), wasm.end()) ) );
      if ( shouldSend ) {
         std::cerr << localized("Setting Code...") << std::endl;
         send_actions(std::move(actions), 10000, packed_transaction::zlib);
      }
   };

   auto set_abi_callback = [&]() {
      fc::path cpath(contractPath);
      if( cpath.filename().generic_string() == "." ) cpath = cpath.parent_path();

      if( abiPath.empty() )
      {
         abiPath = (cpath / (cpath.filename().generic_string()+".abi")).generic_string();
      }

      ULTRAIN_ASSERT( fc::exists( abiPath ), abi_file_not_found, "no abi file found ${f}", ("f", abiPath)  );

      try {
         actions.emplace_back( create_setabi(account, fc::json::from_file(abiPath).as<abi_def>()) );
      } ULTRAIN_RETHROW_EXCEPTIONS(abi_type_exception,  "Fail to parse ABI JSON")
      if ( shouldSend ) {
         std::cerr << localized("Setting ABI...") << std::endl;
         send_actions(std::move(actions), 10000, packed_transaction::zlib);
      }
   };

   add_standard_transaction_options(contractSubcommand, "account@active");
   add_standard_transaction_options(codeSubcommand, "account@active");
   add_standard_transaction_options(abiSubcommand, "account@active");
   contractSubcommand->set_callback([&] {
      shouldSend = false;
      set_code_callback();
      set_abi_callback();
      std::cerr << localized("Publishing contract...") << std::endl;
      send_actions(std::move(actions), 10000, packed_transaction::zlib);
   });
   codeSubcommand->set_callback(set_code_callback);
   abiSubcommand->set_callback(set_abi_callback);

   // set account
   auto setAccount = setSubcommand->add_subcommand("account", localized("set or update blockchain account state"))->require_subcommand();

   // set account permission
   auto setAccountPermission = set_account_permission_subcommand(setAccount);

   // set action
   auto setAction = setSubcommand->add_subcommand("action", localized("set or update blockchain action state"))->require_subcommand();

   // set action permission
   auto setActionPermission = set_action_permission_subcommand(setAction);

   // Transfer subcommand
   string con = "utrio.token";
   string sender;
   string recipient;
   string amount;
   string memo;
   auto transfer = app.add_subcommand("transfer", localized("Transfer UGAS from account to account"), false);
   transfer->add_option("sender", sender, localized("The account sending UGAS"))->required();
   transfer->add_option("recipient", recipient, localized("The account receiving UGAS"))->required();
   transfer->add_option("amount", amount, localized("The amount of UGAS to send"))->required();
   transfer->add_option("memo", memo, localized("The memo for the transfer"));
   transfer->add_option("--contract,-c", con, localized("The contract which controls the token"));

   add_standard_transaction_options(transfer, "sender@active");
   transfer->set_callback([&] {
      signed_transaction trx;
      if (tx_force_unique && memo.size() == 0) {
         // use the memo to add a nonce
         memo = generate_nonce_string();
         tx_force_unique = false;
      }

      send_actions({create_transfer(con,sender, recipient, to_asset(amount), memo)});
   });

   //set clear contract
    auto empty_contract = app.add_subcommand("clearcontract", localized("clear contract"), false);
    empty_contract->require_subcommand();
    auto emptyContractSubcommand = empty_contract->add_subcommand("contract", localized("clear contract code and abi on an account"));
    emptyContractSubcommand->add_option("account", account, localized("The account to remove contract for"))->required();

    auto emptyCodeSubcommand = empty_contract->add_subcommand("code", localized("Remove the code on an account"));
    emptyCodeSubcommand->add_option("account", account, localized("The account to remove code for"))->required();

    auto emptyAbiSubcommand = empty_contract->add_subcommand("abi", localized("Remove the abi on an account"));
    emptyAbiSubcommand->add_option("account", account, localized("The account to remove abi for"))->required();

    bool shouldSendEmpty = true;

    auto set_empty_code_callback = [&]() {

      chain::action code_action = chain::action {
            tx_permission.empty() ? vector<chain::permission_level>{{account,config::active_name}} : get_account_permissions(tx_permission),
            setcode{
               .account   = account,
               .vmtype    = 0,
               .vmversion = 0,
               .code      = bytes()
            }
        };

      actions.emplace_back(
        std::move(code_action)
      );
      if ( shouldSendEmpty ) {
         std::cerr << localized("Setting Code...") << std::endl;
         send_actions(std::move(actions), 10000, packed_transaction::zlib);
      }

    };

    auto set_empty_abi_callback = [&]() {
      try {
         chain::action abi_action = chain::action {
            tx_permission.empty() ? vector<chain::permission_level>{{account,config::active_name}} : get_account_permissions(tx_permission),
            setabi{
               .account   = account,
               .abi      = bytes()
            }
         };
         actions.emplace_back(std::move(abi_action));
      } ULTRAIN_RETHROW_EXCEPTIONS(abi_type_exception,  "Fail to parse ABI JSON")
      if ( shouldSendEmpty ) {
         std::cerr << localized("Setting ABI...") << std::endl;
         send_actions(std::move(actions), 10000, packed_transaction::zlib);
      }
    };

    add_standard_transaction_options(emptyContractSubcommand, "account@active");
    add_standard_transaction_options(emptyCodeSubcommand, "account@active");
    add_standard_transaction_options(emptyAbiSubcommand, "account@active");
    emptyContractSubcommand->set_callback([&] {
      shouldSendEmpty = false;
      set_empty_code_callback();
      set_empty_abi_callback();
      std::cerr << localized("Publishing contract...") << std::endl;
      send_actions(std::move(actions), 10000, packed_transaction::zlib);
    });
    emptyCodeSubcommand->set_callback(set_empty_code_callback);
    emptyAbiSubcommand->set_callback(set_empty_abi_callback);

   // Net subcommand
   string new_host;
   auto net = app.add_subcommand( "net", localized("Interact with local p2p network connections"), false );
   net->require_subcommand();
   auto connect = net->add_subcommand("connect", localized("start a new connection to a peer"), false);
   connect->add_option("host", new_host, localized("The hostname:port to connect to."))->required();
   connect->set_callback([&] {
      const auto& v = call(::url, net_connect, new_host);
      std::cout << fc::json::to_pretty_string(v) << std::endl;
   });

   auto disconnect = net->add_subcommand("disconnect", localized("close an existing connection"), false);
   disconnect->add_option("host", new_host, localized("The hostname:port to disconnect from."))->required();
   disconnect->set_callback([&] {
      const auto& v = call(::url, net_disconnect, new_host);
      std::cout << fc::json::to_pretty_string(v) << std::endl;
   });

   auto status = net->add_subcommand("status", localized("status of existing connection"), false);
   status->add_option("host", new_host, localized("The hostname:port to query status of connection"))->required();
   status->set_callback([&] {
      const auto& v = call(::url, net_status, new_host);
      std::cout << fc::json::to_pretty_string(v) << std::endl;
   });

   auto connections = net->add_subcommand("peers", localized("status of all existing peers"), false);
   connections->set_callback([&] {
      const auto& v = call(::url, net_connections, new_host);
      std::cout << fc::json::to_pretty_string(v) << std::endl;
   });



   // Wallet subcommand
   auto wallet = app.add_subcommand( "wallet", localized("Interact with local wallet"), false );
   wallet->require_subcommand();
   // create wallet
   string wallet_name = "default";
   auto createWallet = wallet->add_subcommand("create", localized("Create a new wallet locally"), false);
   createWallet->add_option("-n,--name", wallet_name, localized("The name of the new wallet"), true);
   createWallet->set_callback([&wallet_name] {
      const auto& v = call(wallet_url, wallet_create, wallet_name);
      std::cout << localized("Creating wallet: ${wallet_name}", ("wallet_name", wallet_name)) << std::endl;
      std::cout << localized("Save password to use in the future to unlock this wallet.") << std::endl;
      std::cout << localized("Without password imported keys will not be retrievable.") << std::endl;
      std::cout << fc::json::to_pretty_string(v) << std::endl;
   });

   // open wallet
   auto openWallet = wallet->add_subcommand("open", localized("Open an existing wallet"), false);
   openWallet->add_option("-n,--name", wallet_name, localized("The name of the wallet to open"));
   openWallet->set_callback([&wallet_name] {
      call(wallet_url, wallet_open, wallet_name);
      std::cout << localized("Opened: ${wallet_name}", ("wallet_name", wallet_name)) << std::endl;
   });

   // lock wallet
   auto lockWallet = wallet->add_subcommand("lock", localized("Lock wallet"), false);
   lockWallet->add_option("-n,--name", wallet_name, localized("The name of the wallet to lock"));
   lockWallet->set_callback([&wallet_name] {
      call(wallet_url, wallet_lock, wallet_name);
      std::cout << localized("Locked: ${wallet_name}", ("wallet_name", wallet_name)) << std::endl;
   });

   // lock all wallets
   auto locakAllWallets = wallet->add_subcommand("lock_all", localized("Lock all unlocked wallets"), false);
   locakAllWallets->set_callback([] {
      call(wallet_url, wallet_lock_all);
      std::cout << localized("Locked All Wallets") << std::endl;
   });

   // unlock wallet
   string wallet_pw;
   auto unlockWallet = wallet->add_subcommand("unlock", localized("Unlock wallet"), false);
   unlockWallet->add_option("-n,--name", wallet_name, localized("The name of the wallet to unlock"));
   unlockWallet->add_option("--password", wallet_pw, localized("The password returned by wallet create"));
   unlockWallet->set_callback([&wallet_name, &wallet_pw] {
      prompt_for_wallet_password(wallet_pw, wallet_name);

      fc::variants vs = {fc::variant(wallet_name), fc::variant(wallet_pw)};
      call(wallet_url, wallet_unlock, vs);
      std::cout << localized("Unlocked: ${wallet_name}", ("wallet_name", wallet_name)) << std::endl;
   });

   // import keys into wallet
   string wallet_key_str;
   auto importWallet = wallet->add_subcommand("import", localized("Import private key into wallet"), false);
   importWallet->add_option("-n,--name", wallet_name, localized("The name of the wallet to import key into"));
   importWallet->add_option("--private-key", wallet_key_str, localized("Private key in WIF format to import"));
   importWallet->set_callback([&wallet_name, &wallet_key_str] {
      if( wallet_key_str.size() == 0 ) {
         std::cout << localized("private key: ");
         fc::set_console_echo(false);
         std::getline( std::cin, wallet_key_str, '\n' );
         fc::set_console_echo(true);
      }

      private_key_type wallet_key;
      try {
         wallet_key = private_key_type( wallet_key_str );
      } catch (...) {
         ULTRAIN_THROW(private_key_type_exception, "Invalid private key: ${private_key}", ("private_key", wallet_key_str))
      }
      public_key_type pubkey = wallet_key.get_public_key();

      fc::variants vs = {fc::variant(wallet_name), fc::variant(wallet_key)};
      call(wallet_url, wallet_import_key, vs);
      std::cout << localized("imported private key for: ${pubkey}", ("pubkey", std::string(pubkey))) << std::endl;
   });

   // remove keys from wallet
   string wallet_rm_key_str;
   auto removeKeyWallet = wallet->add_subcommand("remove_key", localized("Remove key from wallet"), false);
   removeKeyWallet->add_option("-n,--name", wallet_name, localized("The name of the wallet to remove key from"));
   removeKeyWallet->add_option("key", wallet_rm_key_str, localized("Public key in WIF format to remove"))->required();
   removeKeyWallet->add_option("--password", wallet_pw, localized("The password returned by wallet create"));
   removeKeyWallet->set_callback([&wallet_name, &wallet_pw, &wallet_rm_key_str] {
      prompt_for_wallet_password(wallet_pw, wallet_name);
      public_key_type pubkey;
      try {
         pubkey = public_key_type( wallet_rm_key_str );
      } catch (...) {
         ULTRAIN_THROW(public_key_type_exception, "Invalid public key: ${public_key}", ("public_key", wallet_rm_key_str))
      }
      fc::variants vs = {fc::variant(wallet_name), fc::variant(wallet_pw), fc::variant(wallet_rm_key_str)};
      call(wallet_url, wallet_remove_key, vs);
      std::cout << localized("removed private key for: ${pubkey}", ("pubkey", wallet_rm_key_str)) << std::endl;
   });

   // create a key within wallet
   string wallet_create_key_type;
   auto createKeyInWallet = wallet->add_subcommand("create_key", localized("Create private key within wallet"), false);
   createKeyInWallet->add_option("-n,--name", wallet_name, localized("The name of the wallet to create key into"), true);
   createKeyInWallet->add_option("key_type", wallet_create_key_type, localized("Key type to create (K1/R1)"), true)->set_type_name("K1/R1");
   createKeyInWallet->set_callback([&wallet_name, &wallet_create_key_type] {
      //an empty key type is allowed -- it will let the underlying wallet pick which type it prefers
      fc::variants vs = {fc::variant(wallet_name), fc::variant(wallet_create_key_type)};
      const auto& v = call(wallet_url, wallet_create_key, vs);
      std::cout << localized("Created new private key with a public key of: ") << fc::json::to_pretty_string(v) << std::endl;
   });

   // list wallets
   auto listWallet = wallet->add_subcommand("list", localized("List opened wallets, * = unlocked"), false);
   listWallet->set_callback([] {
      std::cout << localized("Wallets:") << std::endl;
      const auto& v = call(wallet_url, wallet_list);
      std::cout << fc::json::to_pretty_string(v) << std::endl;
   });

   // list keys
   auto listKeys = wallet->add_subcommand("keys", localized("List of public keys from all unlocked wallets."), false);
   listKeys->set_callback([] {
      const auto& v = call(wallet_url, wallet_public_keys);
      std::cout << fc::json::to_pretty_string(v) << std::endl;
   });

   // list private keys
   auto listPrivKeys = wallet->add_subcommand("private_keys", localized("List of private keys from an unlocked wallet in wif or PVT_R1 format."), false);
   listPrivKeys->add_option("-n,--name", wallet_name, localized("The name of the wallet to list keys from"), true);
   listPrivKeys->add_option("--password", wallet_pw, localized("The password returned by wallet create"));
   listPrivKeys->set_callback([&wallet_name, &wallet_pw] {
      prompt_for_wallet_password(wallet_pw, wallet_name);
      fc::variants vs = {fc::variant(wallet_name), fc::variant(wallet_pw)};
      const auto& v = call(wallet_url, wallet_list_keys, vs);
      std::cout << fc::json::to_pretty_string(v) << std::endl;
   });

   auto stopKultraind = wallet->add_subcommand("stop", localized("Stop kultraind (doesn't work with nodultrain)."), false);
   stopKultraind->set_callback([] {
      const auto& v = call(wallet_url, kultraind_stop);
      if ( !v.is_object() || v.get_object().size() != 0 ) { //on success kultraind responds with empty object
         std::cerr << fc::json::to_pretty_string(v) << std::endl;
      } else {
         std::cout << "OK" << std::endl;
      }
   });

   // sign subcommand
   string trx_json_to_sign;
   string str_private_key;
   string str_chain_id;
   bool push_trx = false;

   auto sign = app.add_subcommand("sign", localized("Sign a transaction"), false);
   sign->add_option("transaction", trx_json_to_sign,
                                 localized("The JSON string or filename defining the transaction to sign"), true)->required();
   sign->add_option("-k,--private-key", str_private_key, localized("The private key that will be used to sign the transaction"));
   sign->add_option("-c,--chain-id", str_chain_id, localized("The chain id that will be used to sign the transaction"));
   sign->add_flag( "-p,--push-transaction", push_trx, localized("Push transaction after signing"));

   sign->set_callback([&] {
      signed_transaction trx = json_from_file_or_string(trx_json_to_sign).as<signed_transaction>();

      fc::optional<chain_id_type> chain_id;

      if( str_chain_id.size() == 0 ) {
         ilog( "grabbing chain_id from nodultrain" );
         auto info = get_info();
         chain_id = info.chain_id;
      } else {
         chain_id = chain_id_type(str_chain_id);
      }

      if( str_private_key.size() == 0 ) {
         std::cerr << localized("private key: ");
         fc::set_console_echo(false);
         std::getline( std::cin, str_private_key, '\n' );
         fc::set_console_echo(true);
      }

      auto priv_key = fc::crypto::private_key::regenerate(*utilities::wif_to_key(str_private_key));
      trx.sign(priv_key, *chain_id);

      if(push_trx) {
         auto trx_result = call(push_txn_func, packed_transaction(trx, packed_transaction::none));
         std::cout << fc::json::to_pretty_string(trx_result) << std::endl;
      } else {
         std::cout << fc::json::to_pretty_string(trx) << std::endl;
      }
   });

   // Push subcommand
   auto push = app.add_subcommand("push", localized("Push arbitrary transactions to the blockchain"), false);
   push->require_subcommand();

   // push action
   string contract_account;
   string action;
   string data;
   vector<string> permissions;
   auto actionsSubcommand = push->add_subcommand("action", localized("Push a transaction with a single action"));
   actionsSubcommand->fallthrough(false);
   actionsSubcommand->add_option("account", contract_account,
                                 localized("The account providing the contract to execute"), true)->required();
   actionsSubcommand->add_option("action", action,
                                 localized("A JSON string or filename defining the action to execute on the contract"), true)->required();
   actionsSubcommand->add_option("data", data, localized("The arguments to the contract"))->required();

   add_standard_transaction_options(actionsSubcommand);
   actionsSubcommand->set_callback([&] {
      fc::variant action_args_var;
      if( !data.empty() ) {
         try {
            action_args_var = json_from_file_or_string(data, fc::json::relaxed_parser);
         } ULTRAIN_RETHROW_EXCEPTIONS(action_type_exception, "Fail to parse action JSON data='${data}'", ("data", data))
      }
      auto accountPermissions = get_account_permissions(tx_permission);

      send_actions({chain::action{accountPermissions, contract_account, action, variant_to_bin( contract_account, action, action_args_var ) }});
   });

   // push transaction
   string trx_to_push;
   auto trxSubcommand = push->add_subcommand("transaction", localized("Push an arbitrary JSON transaction"));
   trxSubcommand->add_option("transaction", trx_to_push, localized("The JSON string or filename defining the transaction to push"))->required();

   trxSubcommand->set_callback([&] {
      fc::variant trx_var;
      try {
         trx_var = json_from_file_or_string(trx_to_push);
      } ULTRAIN_RETHROW_EXCEPTIONS(transaction_type_exception, "Fail to parse transaction JSON '${data}'", ("data",trx_to_push))
      signed_transaction trx = trx_var.as<signed_transaction>();
      auto trx_result = call(push_txn_func, packed_transaction(trx, packed_transaction::none));
      std::cout << fc::json::to_pretty_string(trx_result) << std::endl;
   });


   string trxsJson;
   auto trxsSubcommand = push->add_subcommand("transactions", localized("Push an array of arbitrary JSON transactions"));
   trxsSubcommand->add_option("transactions", trxsJson, localized("The JSON string or filename defining the array of the transactions to push"))->required();
   trxsSubcommand->set_callback([&] {
      fc::variant trx_var;
      try {
         trx_var = json_from_file_or_string(trxsJson);
      } ULTRAIN_RETHROW_EXCEPTIONS(transaction_type_exception, "Fail to parse transaction JSON '${data}'", ("data",trxsJson))
      auto trxs_result = call(push_txns_func, trx_var);
      std::cout << fc::json::to_pretty_string(trxs_result) << std::endl;
   });


   // multisig subcommand
   auto msig = app.add_subcommand("multisig", localized("Multisig contract commands"), false);
   msig->require_subcommand();

   // multisig propose
   string proposal_name;
   string requested_data;
   string transaction_perm;
   string proposed_transaction;
   string proposed_contract;
   string proposed_action;
   string proposer;
   unsigned int proposal_expiration_hours = 24;
   CLI::callback_t parse_expiration_hours = [&](CLI::results_t res) -> bool {
      unsigned int value_s;
      if (res.size() == 0 || !CLI::detail::lexical_cast(res[0], value_s)) {
         return false;
      }

      proposal_expiration_hours = static_cast<uint64_t>(value_s);
      return true;
   };

   auto propose_action = msig->add_subcommand("propose", localized("Propose action"));
   add_standard_transaction_options(propose_action);
   propose_action->add_option("proposal_name", proposal_name, localized("proposal name (string)"))->required();
   propose_action->add_option("requested_permissions", requested_data, localized("The JSON string or filename defining requested permissions"))->required();
   propose_action->add_option("trx_permissions", transaction_perm, localized("The JSON string or filename defining transaction permissions"))->required();
   propose_action->add_option("contract", proposed_contract, localized("contract to which deferred transaction should be delivered"))->required();
   propose_action->add_option("action", proposed_action, localized("action of deferred transaction"))->required();
   propose_action->add_option("data", proposed_transaction, localized("The JSON string or filename defining the action to propose"))->required();
   propose_action->add_option("proposer", proposer, localized("Account proposing the transaction"));
   propose_action->add_option("proposal_expiration", parse_expiration_hours, localized("Proposal expiration interval in hours"));

   propose_action->set_callback([&] {
      fc::variant requested_data_var;
      try {
         requested_data_var = json_from_file_or_string(requested_data);
      } ULTRAIN_RETHROW_EXCEPTIONS(transaction_type_exception, "Fail to parse permissions JSON '${data}'", ("data",requested_data))
      fc::variant transaction_perm_var;
      try {
         transaction_perm_var = json_from_file_or_string(transaction_perm);
      } ULTRAIN_RETHROW_EXCEPTIONS(transaction_type_exception, "Fail to parse permissions JSON '${data}'", ("data",transaction_perm))
      fc::variant trx_var;
      try {
         trx_var = json_from_file_or_string(proposed_transaction);
      } ULTRAIN_RETHROW_EXCEPTIONS(transaction_type_exception, "Fail to parse transaction JSON '${data}'", ("data",proposed_transaction))
      transaction proposed_trx = trx_var.as<transaction>();
      bytes proposed_trx_serialized = variant_to_bin( proposed_contract, proposed_action, trx_var );

      vector<permission_level> reqperm;
      try {
         reqperm = requested_data_var.as<vector<permission_level>>();
      } ULTRAIN_RETHROW_EXCEPTIONS(transaction_type_exception, "Wrong requested permissions format: '${data}'", ("data",requested_data_var));

      vector<permission_level> trxperm;
      try {
         trxperm = transaction_perm_var.as<vector<permission_level>>();
      } ULTRAIN_RETHROW_EXCEPTIONS(transaction_type_exception, "Wrong transaction permissions format: '${data}'", ("data",transaction_perm_var));

      auto accountPermissions = get_account_permissions(tx_permission);
      if (accountPermissions.empty()) {
         if (!proposer.empty()) {
            accountPermissions = vector<permission_level>{{proposer, config::active_name}};
         } else {
            ULTRAIN_THROW(missing_auth_exception, "Authority is not provided (either by multisig parameter <proposer> or -p)");
         }
      }
      if (proposer.empty()) {
         proposer = name(accountPermissions.at(0).actor).to_string();
      }

      transaction trx;

      trx.expiration = fc::time_point_sec( fc::time_point::now() + fc::hours(proposal_expiration_hours) );
      trx.ref_block_num = 0;
      trx.ref_block_prefix = 0;
      trx.max_net_usage_words = 0;
      trx.max_cpu_usage_ms = 0;
      trx.delay_sec = 0;
      trx.actions = { chain::action(trxperm, name(proposed_contract), name_ex(proposed_action), proposed_trx_serialized) };

      fc::to_variant(trx, trx_var);

      auto args = fc::mutable_variant_object()
         ("proposer", proposer )
         ("proposal_name", proposal_name)
         ("requested", requested_data_var)
         ("trx", trx_var);

      send_actions({chain::action{accountPermissions, "utrio.msig", "propose", variant_to_bin( N(utrio.msig), NEX(propose), args ) }});
   });

   //resolver for ABI serializer to decode actions in proposed transaction in multisig contract
   auto resolver = [abi_serializer_max_time](const name& code) -> optional<abi_serializer> {
      auto result = call(get_abi_func, fc::mutable_variant_object("account_name", code.to_string()));
      if (result["abi"].is_object()) {
         //std::cout << "ABI: " << fc::json::to_pretty_string(result) << std::endl;
         return optional<abi_serializer>(abi_serializer(result["abi"].as<abi_def>(), abi_serializer_max_time));
      } else {
         std::cerr << "ABI for contract " << code.to_string() << " not found. Action data will be shown in hex only." << std::endl;
         return optional<abi_serializer>();
      }
   };

   //multisige propose transaction
   auto propose_trx = msig->add_subcommand("propose_trx", localized("Propose transaction"));
   add_standard_transaction_options(propose_trx);
   propose_trx->add_option("proposal_name", proposal_name, localized("proposal name (string)"))->required();
   propose_trx->add_option("requested_permissions", requested_data, localized("The JSON string or filename defining requested permissions"))->required();
   propose_trx->add_option("transaction", trx_to_push, localized("The JSON string or filename defining the transaction to push"))->required();
   propose_trx->add_option("proposer", proposer, localized("Account proposing the transaction"));

   propose_trx->set_callback([&] {
      fc::variant requested_data_var;
      try {
         requested_data_var = json_from_file_or_string(requested_data);
      } ULTRAIN_RETHROW_EXCEPTIONS(transaction_type_exception, "Fail to parse permissions JSON '${data}'", ("data",requested_data))

      fc::variant trx_var;
      try {
         trx_var = json_from_file_or_string(trx_to_push);
      } ULTRAIN_RETHROW_EXCEPTIONS(transaction_type_exception, "Fail to parse transaction JSON '${data}'", ("data",trx_to_push))

      auto accountPermissions = get_account_permissions(tx_permission);
      if (accountPermissions.empty()) {
         if (!proposer.empty()) {
            accountPermissions = vector<permission_level>{{proposer, config::active_name}};
         } else {
            ULTRAIN_THROW(missing_auth_exception, "Authority is not provided (either by multisig parameter <proposer> or -p)");
         }
      }
      if (proposer.empty()) {
         proposer = name(accountPermissions.at(0).actor).to_string();
      }

      auto args = fc::mutable_variant_object()
         ("proposer", proposer )
         ("proposal_name", proposal_name)
         ("requested", requested_data_var)
         ("trx", trx_var);

      send_actions({chain::action{accountPermissions, "utrio.msig", "propose", variant_to_bin( N(utrio.msig), NEX(propose), args ) }});
   });


   // multisig review
   auto review = msig->add_subcommand("review", localized("Review transaction"));
   review->add_option("proposer", proposer, localized("proposer name (string)"))->required();
   review->add_option("proposal_name", proposal_name, localized("proposal name (string)"))->required();

   review->set_callback([&] {
      auto result = call(get_table_func, fc::mutable_variant_object("json", true)
                         ("code","utrio.msig")
                         ("scope", proposer)
                         ("table", "proposal")
                         ("table_key", "")
                         ("lower_bound", ultrainio::chain::string_to_name(proposal_name.c_str()))
                         ("upper_bound", "")
                         ("limit", 1)
                         );
      //std::cout << fc::json::to_pretty_string(result) << std::endl;

      fc::variants rows = result.get_object()["rows"].get_array();
      if (rows.empty()) {
         std::cerr << "Proposal not found" << std::endl;
         return;
      }
      fc::mutable_variant_object obj = rows[0].get_object();
      if (obj["proposal_name"] != proposal_name) {
         std::cerr << "Proposal not found" << std::endl;
         return;
      }
      auto trx_hex = obj["packed_transaction"].as_string();
      vector<char> trx_blob(trx_hex.size()/2);
      fc::from_hex(trx_hex, trx_blob.data(), trx_blob.size());
      transaction trx = fc::raw::unpack<transaction>(trx_blob);

      fc::variant trx_var;
      abi_serializer abi;
      abi.to_variant(trx, trx_var, resolver, abi_serializer_max_time);
      obj["transaction"] = trx_var;
      std::cout << fc::json::to_pretty_string(obj)
                << std::endl;
   });

   string perm;
   auto approve_or_unapprove = [&](const string& action) {
      fc::variant perm_var;
      try {
         perm_var = json_from_file_or_string(perm);
      } ULTRAIN_RETHROW_EXCEPTIONS(transaction_type_exception, "Fail to parse permissions JSON '${data}'", ("data",perm))
      auto args = fc::mutable_variant_object()
         ("proposer", proposer)
         ("proposal_name", proposal_name)
         ("level", perm_var);

      auto accountPermissions = tx_permission.empty() ? vector<chain::permission_level>{{sender,config::active_name}} : get_account_permissions(tx_permission);
      send_actions({chain::action{accountPermissions, "utrio.msig", action, variant_to_bin( N(utrio.msig), action, args ) }});
   };

   // multisig approve
   auto approve = msig->add_subcommand("approve", localized("Approve proposed transaction"));
   add_standard_transaction_options(approve);
   approve->add_option("proposer", proposer, localized("proposer name (string)"))->required();
   approve->add_option("proposal_name", proposal_name, localized("proposal name (string)"))->required();
   approve->add_option("permissions", perm, localized("The JSON string of filename defining approving permissions"))->required();
   approve->set_callback([&] { approve_or_unapprove("approve"); });

   // multisig unapprove
   auto unapprove = msig->add_subcommand("unapprove", localized("Unapprove proposed transaction"));
   add_standard_transaction_options(unapprove);
   unapprove->add_option("proposer", proposer, localized("proposer name (string)"))->required();
   unapprove->add_option("proposal_name", proposal_name, localized("proposal name (string)"))->required();
   unapprove->add_option("permissions", perm, localized("The JSON string of filename defining approving permissions"))->required();
   unapprove->set_callback([&] { approve_or_unapprove("unapprove"); });

   // multisig cancel
   string canceler;
   auto cancel = msig->add_subcommand("cancel", localized("Cancel proposed transaction"));
   add_standard_transaction_options(cancel);
   cancel->add_option("proposer", proposer, localized("proposer name (string)"))->required();
   cancel->add_option("proposal_name", proposal_name, localized("proposal name (string)"))->required();
   cancel->add_option("canceler", canceler, localized("canceler name (string)"));
   cancel->set_callback([&]() {
      auto accountPermissions = get_account_permissions(tx_permission);
      if (accountPermissions.empty()) {
         if (!canceler.empty()) {
            accountPermissions = vector<permission_level>{{canceler, config::active_name}};
         } else {
            ULTRAIN_THROW(missing_auth_exception, "Authority is not provided (either by multisig parameter <canceler> or -p)");
         }
      }
      if (canceler.empty()) {
         canceler = name(accountPermissions.at(0).actor).to_string();
      }
      auto args = fc::mutable_variant_object()
         ("proposer", proposer)
         ("proposal_name", proposal_name)
         ("canceler", canceler);

      send_actions({chain::action{accountPermissions, "utrio.msig", "cancel", variant_to_bin( N(utrio.msig), NEX(cancel), args ) }});
      }
   );

   // multisig exec
   string executer;
   auto exec = msig->add_subcommand("exec", localized("Execute proposed transaction"));
   add_standard_transaction_options(exec);
   exec->add_option("proposer", proposer, localized("proposer name (string)"))->required();
   exec->add_option("proposal_name", proposal_name, localized("proposal name (string)"))->required();
   exec->add_option("executer", executer, localized("account paying for execution (string)"));
   exec->set_callback([&] {
      auto accountPermissions = get_account_permissions(tx_permission);
      if (accountPermissions.empty()) {
         if (!executer.empty()) {
            accountPermissions = vector<permission_level>{{executer, config::active_name}};
         } else {
            ULTRAIN_THROW(missing_auth_exception, "Authority is not provided (either by multisig parameter <executer> or -p)");
         }
      }
      if (executer.empty()) {
         executer = name(accountPermissions.at(0).actor).to_string();
      }

      auto args = fc::mutable_variant_object()
         ("proposer", proposer )
         ("proposal_name", proposal_name)
         ("executer", executer);

      send_actions({chain::action{accountPermissions, "utrio.msig", "exec", variant_to_bin( N(utrio.msig), NEX(exec), args ) }});
      }
   );

   // sudo subcommand
   auto sudo = app.add_subcommand("sudo", localized("Sudo contract commands"), false);
   sudo->require_subcommand();

   // sudo exec
   executer = "";
   string trx_to_exec;
   auto sudo_exec = sudo->add_subcommand("exec", localized("Execute a transaction while bypassing authorization checks"));
   add_standard_transaction_options(sudo_exec);
   sudo_exec->add_option("executer", executer, localized("Account executing the transaction and paying for the deferred transaction RAM"))->required();
   sudo_exec->add_option("transaction", trx_to_exec, localized("The JSON string or filename defining the transaction to execute"))->required();

   sudo_exec->set_callback([&] {
      fc::variant trx_var;
      try {
         trx_var = json_from_file_or_string(trx_to_exec);
      } ULTRAIN_RETHROW_EXCEPTIONS(transaction_type_exception, "Fail to parse transaction JSON '${data}'", ("data",trx_to_exec))

      auto accountPermissions = get_account_permissions(tx_permission);
      if( accountPermissions.empty() ) {
         accountPermissions = vector<permission_level>{{executer, config::active_name}, {"utrio.sudo", config::active_name}};
      }

      auto args = fc::mutable_variant_object()
         ("executer", executer )
         ("trx", trx_var);

      send_actions({chain::action{accountPermissions, "utrio.sudo", "exec", variant_to_bin( N(utrio.sudo), NEX(exec), args ) }});
   });

   // system subcommand
   auto system = app.add_subcommand("system", localized("Send ultrainio.system contract action to the blockchain."), false);
   system->require_subcommand();

   auto registerProducer = register_producer_subcommand(system);

   auto listProducers = list_producers_subcommand(system);
   auto buyresourcespackage = buy_respackage_subcommand(system);
   auto transferResource = transfer_res_subcommand(system);
   auto delegatecons = delegate_cons_subcommand(system);
   auto undelegatecons = undelegate_cons_subcommand(system);
   auto listdelcons = list_delcons_subcommand(system);
   auto cancelDelay = canceldelay_subcommand(system);
   auto empowerUser = empoweruser_subcommand(system);

   // resource subcommand
   auto resource = app.add_subcommand("resource", localized("Send resource contract action to the blockchain."), false);
   resource->require_subcommand();

   auto buyresource = buy_resource_package_subcommand(resource);
   auto transresource = transfer_resource_package_subcommand(resource);
   auto putorder = put_order_subcommand(resource);
   auto listorders = list_orders_subcommand(resource);
   auto updateorder = update_order_subcommand(resource);
   auto cancelorder = cancel_order_subcommand(resource);
   auto buyin = buy_in_subcommand(resource);

   try {
       app.parse(argc, argv);
   } catch (const CLI::ParseError &e) {
       return app.exit(e);
   } catch (const explained_exception& e) {
      return 1;
   } catch (connection_exception& e) {
      if (verbose_errors) {
         elog("connect error: ${e}", ("e", e.to_detail_string()));
      }
   } catch (const fc::exception& e) {
      // attempt to extract the error code if one is present
      if (!print_recognized_errors(e, verbose_errors)) {
         // Error is not recognized
         if (!print_help_text(e) || verbose_errors) {
            elog("Failed with error: ${e}", ("e", verbose_errors ? e.to_detail_string() : e.to_string()));
         }
      }
      return 1;
   }

   return 0;
}
