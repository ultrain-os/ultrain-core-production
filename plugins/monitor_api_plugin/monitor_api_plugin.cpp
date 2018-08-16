/**
 *  @file
 *  @copyright defined in ultrain/LICENSE.txt
 */
#include <ultrainio/monitor_api_plugin/monitor_api_plugin.hpp>
#include <fc/io/json.hpp>

namespace ultrainio {
   static appbase::abstract_plugin& _monitor_api_plugin = app().register_plugin<monitor_api_plugin>();

void monitor_api_plugin::plugin_initialize(const variables_map& options) {}

#define MONITOR_CALL(api_name, api_handle, api_namespace, call_name) \
{std::string("/v1/" #api_name "/" #call_name), \
   [this, api_handle](string, string body, url_response_callback cb) mutable { \
          try { \
             if (body.empty()) body = "{}"; \
             auto result = api_handle.call_name(fc::json::from_string(body).as<api_namespace::call_name ## _params>()); \
             cb(200, fc::json::to_string(result)); \
          } catch (...) { \
             http_plugin::handle_exception(#api_name, #call_name, body, cb); \
          } \
       }}

#define PRODUCER_MP_CALL(call_name) MONITOR_CALL(monitor, mp_api, monitor_apis::monitor_only, call_name)

void monitor_api_plugin::plugin_startup() {
   // Make the magic happen
   ilog( "starting monitor_api_plugin" );
   auto mp_api = app().get_plugin<monitor_plugin>().get_monitor_only_api();

   app().get_plugin<http_plugin>().add_api({
      PRODUCER_MP_CALL(monitor_node),
      PRODUCER_MP_CALL(monitor_propose_msg),
      PRODUCER_MP_CALL(monitor_echo_msg)
   });
}

}
