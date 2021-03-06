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

    struct status_code {
        int32_t             code;
        string              des;
        string              host;
        string              msg;
    };

    struct block_info {
        uint32_t             block_height;
        uint32_t             first_block_height;
        string              msg;
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

        string                       require_ws(const chain::ws_info& info);
        string                       require_block(const std::string& chain_id_text,uint32_t end);
        status_code                  ws_status(string id);
        string                       test_latancy(uint32_t send_size, uint32_t requst_size);
        string                       repair_blog(string path,int32_t height);

        chain::ws_info               latest_wsinfo();
        void                         set_vaild_ws(uint32_t vaild_block_height);

        size_t num_peers() const;
        void sync_ws(const chain::ws_info& info, int try_cnt, int waiting_time = 0);
        block_info get_local_block_info();
    private:
        std::unique_ptr<class sync_net_plugin_impl> my;
    };

}

FC_REFLECT( ultrainio::connection_status, (peer)(connecting)(last_handshake) )
FC_REFLECT( ultrainio::sync_wss_params, (chainId)(hosts) )
FC_REFLECT( ultrainio::status_code,(code)(des)(host)(msg))
FC_REFLECT( ultrainio::block_info,(block_height)(first_block_height)(msg))
