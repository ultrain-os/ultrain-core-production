/**
 *  @file
 *  @copyright defined in ultrain/LICENSE.txt
 */
#pragma once
#include <ultrainio/monitor_plugin/monitor_plugin.hpp>
#include <ultrainio/http_plugin/http_plugin.hpp>
#include <appbase/application.hpp>

namespace ultrainio {

using namespace appbase;

class monitor_api_plugin : public appbase::plugin<monitor_api_plugin> {
public:
   monitor_api_plugin() = default;
   virtual ~monitor_api_plugin() override = default;

   monitor_api_plugin(const monitor_api_plugin&) = delete;
   monitor_api_plugin(monitor_api_plugin&&) = delete;
   monitor_api_plugin& operator=(const monitor_api_plugin&) = delete;
   monitor_api_plugin& operator=(monitor_api_plugin&&) = delete;

   APPBASE_PLUGIN_REQUIRES((monitor_plugin))
   virtual void set_program_options(options_description&, options_description& cfg) override {}

   void plugin_initialize(const variables_map& options);
   void plugin_startup();
   void plugin_shutdown(){}

private:
};

}
