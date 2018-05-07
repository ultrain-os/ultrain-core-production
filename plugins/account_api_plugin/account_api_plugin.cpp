/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */
#include <ultrainio/account_api_plugin/account_api_plugin.hpp>
#include <ultrainio/chain/chain_controller.hpp>
#include <ultrainio/chain/exceptions.hpp>

#include <ultrainio/chain_plugin/chain_plugin.hpp>
#include <ultrainio/chain/wast_to_wasm.hpp>
#include <ultrainio/utilities/key_conversion.hpp>

#include <fc/variant.hpp>
#include <fc/io/json.hpp>
#include <fc/exception/exception.hpp>
#include <fc/reflect/variant.hpp>
#include <fc/io/json.hpp>

#include <boost/asio/high_resolution_timer.hpp>
#include <boost/algorithm/clamp.hpp>

#include <Inline/BasicTypes.h>
#include <IR/Module.h>
#include <IR/Validate.h>
#include <WAST/WAST.h>
#include <WASM/WASM.h>
#include <Runtime/Runtime.h>

#include <currency/currency.wast.hpp>
#include <currency/currency.abi.hpp>

#include <ultrainio/wallet_plugin/wallet_plugin.hpp>
#include <ultrainio/wallet_plugin/wallet_manager.hpp>

namespace ultrainio {

    using namespace ultrainio;

    static appbase::abstract_plugin &_account_api_plugin = app().register_plugin<account_api_plugin>();

    using namespace ultrainio::chain;

    account_api_plugin::account_api_plugin() {}

    account_api_plugin::~account_api_plugin() {}

    void account_api_plugin::set_program_options(options_description &, options_description &) {}

    void account_api_plugin::plugin_initialize(const variables_map &) {}

#define CALL(api_name, api_handle, api_namespace, call_name) \
{std::string("/v1/" #api_name "/" #call_name), \
   [this](string, string body, url_response_callback cb) mutable { \
          try { \
             if (body.empty()) body = "{}"; \
             api_handle->call_name(fc::json::from_string(body).as<api_namespace::call_name ## _params>()); \
            auto result = "";\
            cb(200, fc::json::to_string(result)); \
          } catch (fc::eof_exception& e) { \
             error_results results{400, "Bad Request", e}; \
             cb(400, fc::json::to_string(results)); \
             elog("Unable to parse arguments: ${args}", ("args", body)); \
          } catch (fc::exception& e) { \
             error_results results{500, "Internal Service Error", e}; \
             cb(500, fc::json::to_string(results)); \
             elog("Exception encountered while processing ${call}: ${e}", ("call", #api_name "." #call_name)("e", e)); \
          } \
       }}

#define CHAIN_RO_CALL(call_name, api_handle) CALL(account, api_handle, account_api_plugin_impl, call_name)
//#define CHAIN_RW_CALL(call_name) CALL(account, rw_api, account_history_apis::read_write, call_name)

    struct account_api_plugin_impl {
        struct create_account_params {
            string control_account;
            string account_name;
            string owner_public_key;
            string active_public_key;
        };

        void create_account(const create_account_params &params) {
            name newaccount(params.account_name);
            name creator(params.control_account);

            chain_controller &cc = app().get_plugin<chain_plugin>().chain();
            chain::chain_id_type chainid;
            app().get_plugin<chain_plugin>().get_chain_id(chainid);

            fc::crypto::public_key owner_public_key = fc::crypto::public_key(params.owner_public_key);
            fc::crypto::public_key active_public_key = fc::crypto::public_key(params.active_public_key);

            signed_transaction trx;
            auto owner_auth = ultrainio::chain::authority{1, {{owner_public_key, 1}}, {}};
            auto active_auth = ultrainio::chain::authority{1, {{owner_public_key, 1}}, {}};
            auto recovery_auth = ultrainio::chain::authority{1, {}, {{{creator, config::active_name}, 1}}};

            trx.actions.emplace_back(vector<chain::permission_level>{{creator, config::active_name}},
                                     contracts::newaccount{creator, newaccount, owner_auth, active_auth,
                                                           recovery_auth});


            trx.expiration = cc.head_block_time() + fc::seconds(30);
            trx.set_reference_block(cc.head_block_id());
//            trx.set_reference_block(cc.get_block_id_for_num(cc.last_irreversible_block_num()));

            //trx.sign(creator_priv_key, chainid);
            wallet_plugin *wallet_plug = app().find_plugin<wallet_plugin>();
            chain_plugin *chain_plug = app().find_plugin<chain_plugin>();
//            const auto& public_keys = call(wallet_host, wallet_port, wallet_public_keys);
            const auto &public_keys = wallet_plug->get_wallet_manager().get_public_keys();
            chain_apis::read_only::get_required_keys_params get_arg{fc::variant(trx), public_keys};
//            const auto& required_keys = call(host, port, get_required_keys, get_arg);
            chain_apis::read_only::get_required_keys_result rkeys = chain_plug->get_read_only_api().get_required_keys(
                    get_arg);
//            const auto& signed_trx = call(wallet_host, wallet_port, wallet_sign_trx, sign_args);
            trx = wallet_plug->get_wallet_manager().sign_transaction(trx, rkeys.required_keys, chain_id_type{});//cc.head_block_id());

            cc.push_transaction(packed_transaction(trx));
        }

    };

    void account_api_plugin::plugin_startup() {
        ilog("starting account_api_plugin");
        app().get_plugin<http_plugin>().add_api({
        CHAIN_RO_CALL(create_account,my)
//      CHAIN_RO_CALL(get_transactions),
//      CHAIN_RO_CALL(get_key_accounts),
//      CHAIN_RO_CALL(get_controlled_accounts)
                                                });
        my.reset(new account_api_plugin_impl);
    }

    void account_api_plugin::plugin_shutdown() {}

}

FC_REFLECT(ultrainio::account_api_plugin_impl::create_account_params, (control_account)(account_name)(owner_public_key)(active_public_key) )