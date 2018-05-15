/**
 *  @file
 *  @copyright defined in ultrain/LICENSE.txt
 */
#pragma once
#include <appbase/application.hpp>

namespace ultrainio {

using namespace appbase;

/**
 *  This is a template plugin, intended to serve as a starting point for making new plugins
 */
class producer_uranus_plugin : public appbase::plugin<producer_uranus_plugin> {
public:
   APPBASE_PLUGIN_REQUIRES()
   producer_uranus_plugin();
   virtual ~producer_uranus_plugin();
 

   virtual void set_program_options(options_description&, options_description& cfg) override;
 
   void plugin_initialize(const variables_map& options);
   void plugin_startup();
   void plugin_shutdown();

private:
   std::unique_ptr<class producer_uranus_plugin_impl> my;
};

}
