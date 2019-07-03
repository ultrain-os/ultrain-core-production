/**
 *  @file
 *  @copyright defined in ultrain/LICENSE.txt
 */
#pragma once
#include <appbase/application.hpp>
#include <ultrainio/producer_uranus_plugin/producer_uranus_plugin.hpp>
#include "uranus_node_monitor.hpp"
#include "uranus_controller_monitor.hpp"

namespace ultrainio {

   using namespace appbase;
   using std::shared_ptr;
   using fc::optional;

   typedef shared_ptr<class monitor_plugin_impl> monitor_ptr;
   typedef shared_ptr<const class monitor_plugin_impl> monitor_const_ptr;

namespace monitor_apis {

class monitor_only {
   public:
      monitor_only() = default;

      // server response to clultrain
      struct monitor_node_params {};

      typedef UranusNodeInfo  monitor_node_result;

      monitor_node_result monitor_node(const monitor_node_params& params) const;

      typedef BlockHeaderDigest monitor_propose_msg_result;

      struct monitor_propose_msg_params
      {
            std::string block_id;
      };

      monitor_propose_msg_result monitor_propose_msg( const monitor_propose_msg_params& params) const;

      typedef monitor_propose_msg_params monitor_echo_msg_params; //: public monitor_propose_msg_params {};
      typedef EchoMsgInfoDigest monitor_echo_msg_result;

      monitor_echo_msg_result monitor_echo_msg( const monitor_echo_msg_params& params) const;

      typedef RoundInfo monitor_propose_cache_params;
      struct monitor_propose_cache_result{
            std::vector<BlockHeaderDigest>  proposeCache;
      };

      monitor_propose_cache_result monitor_propose_cache(const monitor_propose_cache_params& params) const;

      typedef RoundInfo monitor_echo_cache_params;
      struct monitor_echo_cache_result{
            std::vector<EchoMsgDigest>   echoCache;
      };

      monitor_echo_cache_result monitor_echo_cache(const monitor_echo_cache_params& params) const;

      typedef RoundInfo monitor_echo_ap_cache_params;
      struct monitor_echo_ap_cache_result{
            std::vector<std::pair<chain::block_id_type, EchoMsgInfoDigest>>   echoApCache;
      };

      monitor_echo_ap_cache_result monitor_echo_ap_cache(const monitor_echo_ap_cache_params& params) const;

      //client request to monitor central server
      void getDynamicNodeData(periodic_report_dynamic_data& reportData);
      void getDynamicOsData(periodic_report_dynamic_data& reportData);
      periodic_report_static_data getStaticConfigInfo();

   private:

      const std::shared_ptr<UranusNode>  getNodePtr() const;

      std::shared_ptr<UranusNodeMonitor> m_nodeMonitor;
};
}  //namespace monitor_apis

class monitor_plugin : public appbase::plugin<monitor_plugin> {
public:
   monitor_plugin();
   virtual ~monitor_plugin();
 
   APPBASE_PLUGIN_REQUIRES((producer_uranus_plugin))
   virtual void set_program_options(options_description&, options_description& cfg) override;
 
   void plugin_initialize(const variables_map& options);
   void plugin_startup();
   void plugin_shutdown();

   monitor_apis::monitor_only  get_monitor_only_api()const;
private:
   monitor_ptr my;
};

} ///namespace ultrainio

FC_REFLECT( ultrainio::monitor_apis::monitor_only::monitor_node_params, )
FC_REFLECT( ultrainio::monitor_apis::monitor_only::monitor_node_result, (ready)(connected)(syncing)(syncFailed)
            (isNonProducingNode)(globalProducingNodeNumber)(phase)(baxCount)(proposeMsgNum)(echoMsgnum)
            (proposeMsgCacheSize)(echoMsgCacheSize)(allPhaseEchoMsgNum) )

FC_REFLECT( ultrainio::monitor_apis::monitor_only::monitor_propose_msg_params, (block_id) )
FC_REFLECT( ultrainio::monitor_apis::monitor_only::monitor_propose_msg_result, (myid)(blockNum) )
FC_REFLECT( ultrainio::EchoMsgDigest, (head)(phase)(baxCount) )
FC_REFLECT( ultrainio::monitor_apis::monitor_only::monitor_echo_msg_result, (echoMsg)(hasSend)(accountPoolSize)(account_pool) )

FC_REFLECT( ultrainio::monitor_apis::monitor_only::monitor_propose_cache_params, (blockNum)(phase) )
FC_REFLECT( ultrainio::monitor_apis::monitor_only::monitor_propose_cache_result, (proposeCache) )

FC_REFLECT( ultrainio::monitor_apis::monitor_only::monitor_echo_cache_result, (echoCache) )

FC_REFLECT( ultrainio::monitor_apis::monitor_only::monitor_echo_ap_cache_result, (echoApCache) )
