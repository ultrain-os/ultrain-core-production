/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */

#pragma once
#include <ultrainio/account_history_plugin/account_history_plugin.hpp>
#include <ultrainio/chain_plugin/chain_plugin.hpp>
#include <ultrainio/http_plugin/http_plugin.hpp>

#include <appbase/application.hpp>

namespace ultrainio {

   using namespace appbase;

   class account_api_plugin : public plugin<account_api_plugin> {
      public:
        APPBASE_PLUGIN_REQUIRES((account_history_plugin)(chain_plugin)(http_plugin))

        account_api_plugin();
        virtual ~account_api_plugin();

        virtual void set_program_options(options_description&, options_description&) override;

        void plugin_initialize(const variables_map&);
        void plugin_startup();
        void plugin_shutdown();

      private:
       std::unique_ptr<struct account_api_plugin_impl> my;
   };

}
