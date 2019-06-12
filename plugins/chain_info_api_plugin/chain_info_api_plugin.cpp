/**
 *  @file
 *  @copyright defined in ultrain/LICENSE.txt
 */
#include <ultrainio/chain_info_api_plugin/chain_info_api_plugin.hpp>
#include <ultrainio/chain/exceptions.hpp>

#include <fc/io/json.hpp>

namespace ultrainio {

static appbase::abstract_plugin& _chain_info_api_plugin = app().register_plugin<chain_info_api_plugin>();

using namespace ultrainio;

class chain_info_api_plugin_impl {
public:
   chain_info_api_plugin_impl(controller& db)
      : db(db) {}

   controller& db;
};


chain_info_api_plugin::chain_info_api_plugin(){}
chain_info_api_plugin::~chain_info_api_plugin(){}

void chain_info_api_plugin::set_program_options(options_description&, options_description&) {}
void chain_info_api_plugin::plugin_initialize(const variables_map&) {}

#define CALL(api_name, api_handle, api_namespace, call_name, http_response_code) \
{std::string("/v1/" #api_name "/" #call_name), \
   [this, api_handle](string, string body, url_response_callback cb) mutable { \
          try { \
             if (body.empty()) body = "{}"; \
             auto result = api_handle.call_name(fc::json::from_string(body).as<api_namespace::call_name ## _params>()); \
             cb(http_response_code, fc::json::to_string(result)); \
          } catch (...) { \
             http_plugin::handle_exception(#api_name, #call_name, body, cb); \
          } \
       }}

#define CHAIN_RO_CALL(call_name, http_response_code) CALL(chain_info, ro_api, chain_apis::read_only, call_name, http_response_code)

void chain_info_api_plugin::plugin_startup() {
   ilog( "starting chain_info_api_plugin" );
   my.reset(new chain_info_api_plugin_impl(app().get_plugin<chain_plugin>().chain()));
   auto ro_api = app().get_plugin<chain_plugin>().get_read_only_api();

   app().get_plugin<http_plugin>().add_api({
      CHAIN_RO_CALL(get_chain_info, 200),
      CHAIN_RO_CALL(get_block_info, 200),
      CHAIN_RO_CALL(get_table_records, 200),
      CHAIN_RO_CALL(get_account_exist, 200),
      CHAIN_RO_CALL(get_producers, 200),
      CHAIN_RO_CALL(get_master_block_num, 200),
      CHAIN_RO_CALL(get_merkle_proof, 200),
      CHAIN_RO_CALL(verify_merkle_proof, 200),
      CHAIN_RO_CALL(get_account_info, 200)
   });
}

void chain_info_api_plugin::plugin_shutdown() {}

}
