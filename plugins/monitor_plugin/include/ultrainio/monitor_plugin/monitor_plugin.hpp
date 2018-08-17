/**
 *  @file
 *  @copyright defined in ultrain/LICENSE.txt
 */
#pragma once
#include <appbase/application.hpp>
#include <ultrainio/producer_uranus_plugin/producer_uranus_plugin.hpp>
#include <uranus/Node.h>
#include <uranus/UranusController.h>

namespace ultrainio {

   using namespace appbase;
   using std::shared_ptr;
   using fc::optional;

   typedef shared_ptr<class monitor_plugin_impl> monitor_ptr;
   typedef shared_ptr<const class monitor_plugin_impl> monitor_const_ptr;

namespace monitor_apis {

class monitor_only {
   monitor_const_ptr monitor;
   public:
      monitor_only(monitor_const_ptr&& _monitor)
      :monitor(_monitor) {}

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

      typedef msgkey monitor_propose_cache_params;
      struct monitor_propose_cache_result{
            std::vector<BlockHeaderDigest>  proposeCache;
      };
      
      monitor_propose_cache_result monitor_propose_cache(const monitor_propose_cache_params& params) const;

      typedef msgkey monitor_echo_cache_params;
      struct monitor_echo_cache_result{
            std::vector<EchoMsgDigest>   echoCache;
      };

      monitor_echo_cache_result monitor_echo_cache(const monitor_echo_cache_params& params) const;

      typedef msgkey monitor_echo_ap_cache_params;
      struct monitor_echo_ap_cache_result{
            std::vector<std::pair<chain::block_id_type, EchoMsgInfoDigest>>   echoApCache;
      };

      monitor_echo_ap_cache_result monitor_echo_ap_cache(const monitor_echo_ap_cache_params& params) const;
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

   monitor_apis::monitor_only  get_monitor_only_api()const { return monitor_apis::monitor_only(monitor_const_ptr(my)); }
private:
   monitor_ptr my;
};

} ///namespace ultrainio


FC_REFLECT( ultrainio::monitor_apis::monitor_only::monitor_node_params, )
FC_REFLECT( ultrainio::monitor_apis::monitor_only::monitor_node_result, (ready)(connected)(syncing)(syncFailed)
            (isNonProducingNode)(globalProducingNodeNumber)(phase)(baxCount)(proposeMsgNum)(echoMsgnum)
            (proposeMsgCacheSize)(echoMsgCacheSize)(allPhaseEchoMsgNum) )

FC_REFLECT( ultrainio::monitor_apis::monitor_only::monitor_propose_msg_params, (block_id) )
FC_REFLECT( ultrainio::monitor_apis::monitor_only::monitor_propose_msg_result, (timestamp)(proposerPk)(previous)(myid)(blockNum) )
FC_REFLECT( ultrainio::EchoMsgDigest, (head)(phase)(baxCount) )
FC_REFLECT( ultrainio::monitor_apis::monitor_only::monitor_echo_msg_result, (echoMsg)(hasSend)(pkPoolSize)(pk_pool) )

FC_REFLECT( ultrainio::monitor_apis::monitor_only::monitor_propose_cache_params, (blockNum)(phase) )
FC_REFLECT( ultrainio::monitor_apis::monitor_only::monitor_propose_cache_result, (proposeCache) )

FC_REFLECT( ultrainio::monitor_apis::monitor_only::monitor_echo_cache_result, (echoCache) )

FC_REFLECT( ultrainio::monitor_apis::monitor_only::monitor_echo_ap_cache_result, (echoApCache) )