/**
 *  @file
 *  @copyright defined in ultrain/LICENSE.txt
 */
#include <ultrainio/monitor_plugin/monitor_plugin.hpp>

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
     monitor_only::monitor_node_result monitor_only::monitor_node( const monitor_only::monitor_node_params& params )const {
        auto nodePtr = UranusNode::getInstance();
        if(!nodePtr) {
            auto fce = fc::exception( FC_LOG_MESSAGE( info, "Failed to get UranusNode. Most possible reason: This Node is not a producer." ));
            throw fce;
        }
        return nodePtr->getNodeInfo();
     }

     monitor_only::monitor_propose_msg_result monitor_only::monitor_propose_msg( const monitor_only::monitor_propose_msg_params& params) const {
        auto nodePtr = UranusNode::getInstance();
        if(!nodePtr) {
            auto fce = fc::exception( FC_LOG_MESSAGE( info, "Failed to get UranusNode. Most possible reason: This Node is not a producer." ));
            throw fce;
        }
        return nodePtr->getController()->findProposeMsgByBlockId(chain::block_id_type(params.block_id));
     }

     monitor_only::monitor_echo_msg_result monitor_only::monitor_echo_msg( const monitor_only::monitor_echo_msg_params& params) const{
        auto nodePtr = UranusNode::getInstance();
        if(!nodePtr) {
            auto fce = fc::exception( FC_LOG_MESSAGE( info, "Failed to get UranusNode. Most possible reason: This Node is not a producer." ));
            throw fce;
        }
        return nodePtr->getController()->findEchoMsgByBlockId(chain::block_id_type(params.block_id));
     }

     monitor_only::monitor_propose_cache_result monitor_only::monitor_propose_cache(const monitor_only::monitor_propose_cache_params& params) const{
        auto nodePtr = UranusNode::getInstance();
        if(!nodePtr) {
            auto fce = fc::exception( FC_LOG_MESSAGE( info, "Failed to get UranusNode. Most possible reason: This Node is not a producer." ));
            throw fce;
        }
        return {nodePtr->getController()->findProposeCacheByKey(params)};
     }

     monitor_only::monitor_echo_cache_result monitor_only::monitor_echo_cache(const monitor_only::monitor_echo_cache_params& params) const {
        auto nodePtr = UranusNode::getInstance();
        if(!nodePtr) {
            auto fce = fc::exception( FC_LOG_MESSAGE( info, "Failed to get UranusNode. Most possible reason: This Node is not a producer." ));
            throw fce;
        }
        return {nodePtr->getController()->findEchoCacheByKey(params)};
     }

     monitor_only::monitor_echo_ap_cache_result monitor_only::monitor_echo_ap_cache(const monitor_only::monitor_echo_ap_cache_params& params) const {
        auto nodePtr = UranusNode::getInstance();
        if(!nodePtr) {
            auto fce = fc::exception( FC_LOG_MESSAGE( info, "Failed to get UranusNode. Most possible reason: This Node is not a producer." ));
            throw fce;
        }
        return {nodePtr->getController()->findEchoApMsgByKey(params)};
     }
   }
} ///namespace ultrainio