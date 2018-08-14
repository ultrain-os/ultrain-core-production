/**
 *  @file
 *  @copyright defined in ultrain/LICENSE.txt
 */
#pragma once

#include <ultrainio/producer_uranus_plugin/producer_uranus_plugin.hpp>
#include <ultrainio/http_plugin/http_plugin.hpp>

#include <appbase/application.hpp>

namespace ultrainio {

using namespace appbase;

class producer_api_plugin : public plugin<producer_api_plugin> {
   public:
      APPBASE_PLUGIN_REQUIRES((producer_uranus_plugin) (http_plugin))

      producer_api_plugin() = default;
      producer_api_plugin(const producer_api_plugin&) = delete;
      producer_api_plugin(producer_api_plugin&&) = delete;
      producer_api_plugin& operator=(const producer_api_plugin&) = delete;
      producer_api_plugin& operator=(producer_api_plugin&&) = delete;
      virtual ~producer_api_plugin() override = default;

      virtual void set_program_options(options_description& cli, options_description& cfg) override {}
      void plugin_initialize(const variables_map& vm);
      void plugin_startup();
      void plugin_shutdown() {}

   private:
};

}
