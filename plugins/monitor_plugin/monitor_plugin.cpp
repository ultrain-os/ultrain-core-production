/**
 *  @file
 *  @copyright defined in ultrain/LICENSE.txt
 */
#include <ultrainio/monitor_plugin/monitor_plugin.hpp>
#include <fc/exception/exception.hpp>
#include <ultrainio/chain/exceptions.hpp>

namespace ultrainio {
   static appbase::abstract_plugin& _monitor_plugin = app().register_plugin<monitor_plugin>();

class monitor_plugin_impl {
   public:

   monitor_plugin_impl() = default;
   ~monitor_plugin_impl() = default;

   producer_uranus_plugin* producer_plug = nullptr;
};

monitor_plugin::monitor_plugin():my(new monitor_plugin_impl()){}
monitor_plugin::~monitor_plugin(){}

void monitor_plugin::set_program_options(options_description&, options_description& cfg) {
   //cfg.add_options()
   //      ("option-name", bpo::value<string>()->default_value("default value"),
   //       "Option Description");
}

void monitor_plugin::plugin_initialize(const variables_map& options) {
   if(options.count("option-name")) {
      // Handle the option
   }

   my->producer_plug = app().find_plugin<producer_uranus_plugin>();
}

void monitor_plugin::plugin_startup() {
   // Make the magic happen
}

void monitor_plugin::plugin_shutdown() {
   // OK, that's enough magic
}

   namespace monitor_apis {

     const std::shared_ptr<UranusNode>  monitor_only::getNodePtr() const {
       auto nodePtr = UranusNode::getInstance();
       ULTRAIN_ASSERT( nodePtr != nullptr, chain::node_not_found_exception, "Failed to get UranusNode. Most possible reason: This Node is not a producer." );
       return nodePtr;
     }

     monitor_only::monitor_node_result monitor_only::monitor_node( const monitor_only::monitor_node_params& params )const {
        auto nodePtr = getNodePtr();
        UranusNodeMonitor nodeMonitor(nodePtr);
        return nodeMonitor.getNodeInfo();
     }

     monitor_only::monitor_propose_msg_result monitor_only::monitor_propose_msg( const monitor_only::monitor_propose_msg_params& params) const {
        auto nodePtr = getNodePtr();
        UranusControllerMonitor controllerMonitor(nodePtr->getController());
        return controllerMonitor.findProposeMsgByBlockId(chain::block_id_type(params.block_id));
     }

     monitor_only::monitor_echo_msg_result monitor_only::monitor_echo_msg( const monitor_only::monitor_echo_msg_params& params) const{
        auto nodePtr = getNodePtr();
        UranusControllerMonitor controllerMonitor(nodePtr->getController());
        return controllerMonitor.findEchoMsgByBlockId(chain::block_id_type(params.block_id));
     }

     monitor_only::monitor_propose_cache_result monitor_only::monitor_propose_cache(const monitor_only::monitor_propose_cache_params& params) const{
        auto nodePtr = getNodePtr();
        UranusControllerMonitor controllerMonitor(nodePtr->getController());
        return {controllerMonitor.findProposeCacheByKey(params)};
     }

     monitor_only::monitor_echo_cache_result monitor_only::monitor_echo_cache(const monitor_only::monitor_echo_cache_params& params) const {
        auto nodePtr = getNodePtr();
        UranusControllerMonitor controllerMonitor(nodePtr->getController());
        return {controllerMonitor.findEchoCacheByKey(params)};
     }

     monitor_only::monitor_echo_ap_cache_result monitor_only::monitor_echo_ap_cache(const monitor_only::monitor_echo_ap_cache_params& params) const {
        auto nodePtr = getNodePtr();
        UranusControllerMonitor controllerMonitor(nodePtr->getController());
        return {controllerMonitor.findEchoApMsgByKey(params)};
     }
   }
} ///namespace ultrainio