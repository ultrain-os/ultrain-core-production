/**
 *  @file
 *  @copyright defined in ultrain/LICENSE.txt
 */
#pragma once
#include <appbase/application.hpp>
#include <ultrainio/chain_plugin/chain_plugin.hpp>
#include <core/protocol.hpp>
#include <core/net_kcp_plugin_base.hpp>

namespace ultrainio { namespace kcp_plugin_n {
    using namespace appbase;

    class kcp_plugin : public appbase::plugin<kcp_plugin>
    {
    public:
        kcp_plugin();
        virtual ~kcp_plugin();

        APPBASE_PLUGIN_REQUIRES((chain_plugin))
        virtual void set_program_options(options_description& cli, options_description& cfg) override;

        void plugin_initialize(const variables_map& options);
        void plugin_startup();
        void plugin_shutdown();

        void   broadcast(const ProposeMsg& propose);
        void   partial_broadcast(const ProposeMsg& propose,bool rtn);
        void   broadcast(const EchoMsg& echo);
        void   partial_broadcast(const EchoMsg& echo,bool rtn);
        void   broadcast(const SignedTransaction& trx);
        void   send_block(const fc::sha256 &node_id, const SyncBlockMsg& sync_block);
        bool   send_req_sync(const ReqSyncMsg& reqSyncMsg);
        void   send_block_num_range(const fc::sha256 &node_id, const RspBlockNumRangeMsg& last_block_num);
        void   stop_sync_block();

        string                       connect( const string& endpoint );
        string                       disconnect( const string& endpoint );
        optional<connection_status>  status( const string& endpoint )const;
        vector<connection_status>    connections()const;
        vector<connection_status>    get_connected_connections()const;

        size_t num_peers() const;
    private:
        std::unique_ptr<class kcp_plugin_impl> my;
    };

}
}
