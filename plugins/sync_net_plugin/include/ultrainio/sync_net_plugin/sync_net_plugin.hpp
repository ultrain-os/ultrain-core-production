/**
 *  @file
 *  @copyright defined in ultrain/LICENSE.txt
 */
#pragma once
#include <appbase/application.hpp>
#include <ultrainio/sync_net_plugin/protocol.hpp>
#include <ultrainio/chain/worldstate_file_manager.hpp>
#include <fc/optional.hpp>

namespace ultrainio {
    using namespace appbase;
    using namespace fc;

    struct connection_status {
        string            peer;
        bool              connecting = false;
        wss::handshake_message last_handshake;
    };

    struct sync_wss_params {
        string              chainId;
        flat_set<string>    hosts;
    };

    class sync_net_plugin : public appbase::plugin<sync_net_plugin>
    {
    public:
        sync_net_plugin();
        virtual ~sync_net_plugin();

        APPBASE_PLUGIN_REQUIRES()

        virtual void set_program_options(options_description& cli, options_description& cfg) override;

        void plugin_initialize(const variables_map& options);
        void plugin_startup();
        void plugin_shutdown();

        string                       connect( const string& endpoint );
        string                       disconnect( const string& endpoint );
        optional<connection_status>  status( const string& endpoint )const;
        vector<connection_status>    connections()const;

        string                       require_ws();
        string                       sync_ws(const sync_wss_params& syncWssParams);
        string                       require_block(uint32_t begin,uint32_t end);
        string                       sync_block(uint32_t block_height);
        string                       poll_status(string id);
        string                       test_latancy();

        size_t num_peers() const;
    private:
        std::unique_ptr<class sync_net_plugin_impl> my;
    };

}

FC_REFLECT( ultrainio::connection_status, (peer)(connecting)(last_handshake) )
FC_REFLECT( ultrainio::sync_wss_params, (chainId)(hosts) )
