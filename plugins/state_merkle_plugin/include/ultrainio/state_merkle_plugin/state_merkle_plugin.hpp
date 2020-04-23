/**                                                                                                                       
 *  @file
 *  @copyright defined in ultrain/LICENSE.txt
 */
#pragma once
#include <appbase/application.hpp>
#include <ultrainio/chain_plugin/chain_plugin.hpp>

namespace ultrainio {
using namespace chain;
using boost::signals2::scoped_connection;

using state_merkle_ptr = std::shared_ptr<struct state_merkle_plugin_impl>;

class state_merkle_plugin : public plugin<state_merkle_plugin> {
 public:
     APPBASE_PLUGIN_REQUIRES((chain_plugin))

     state_merkle_plugin();
     virtual ~state_merkle_plugin();

     virtual void set_program_options(options_description& cli, options_description& cfg) override;
     void plugin_initialize(const variables_map& options);
     void plugin_startup();
     void plugin_shutdown();
 private:
     state_merkle_ptr my;
};
}
