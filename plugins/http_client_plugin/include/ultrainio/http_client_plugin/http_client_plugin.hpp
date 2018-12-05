/**
 *  @file
 *  @copyright defined in ultrain/LICENSE.txt
 */
#pragma once
#include <appbase/application.hpp>
#include <fc/network/http/http_client.hpp>
#include <mutex>
#include <ultrainio/chain_plugin/chain_plugin.hpp>


namespace ultrainio {
    using namespace appbase;
    using fc::http_client;
    using fc::url;
    using fc::time_point;
    using fc::variant;

    static const size_t MAX_HTTP_MSG_QUEUE_SIZE = 1000000;

    struct async_http_msg{
        fc::url     dest;
        variant     payload;
        time_point  deadline;

        async_http_msg(const fc::url& dest_in,
                       const variant& payload_in,
                       const time_point& deadline_in) : dest(dest_in), payload(payload_in), deadline(deadline_in) {
        }
    };

    class http_client_plugin : public appbase::plugin<http_client_plugin>
    {
    public:
        http_client_plugin();
        virtual ~http_client_plugin();

        APPBASE_PLUGIN_REQUIRES((chain_plugin))
        virtual void set_program_options(options_description&, options_description& cfg) override;

        void plugin_initialize(const variables_map& options);
        void plugin_startup();
        void plugin_shutdown();
        void schedule();

        // When the msg_queue is full, return false, else true
        bool enqueue(const fc::url& dest, const variant& payload, const time_point& deadline);

        // async post http msg
        bool post(const fc::url& dest, const variant& payload, const time_point& deadline = time_point::maximum()) {
            return enqueue(dest, payload, deadline);
        }

        // async post http msg, this method is identical to post(), but we need it to connect to chain::controller
        bool post_async(const fc::url& dest, const variant& payload, const time_point& deadline = time_point::maximum()) {
            return enqueue(dest, payload, deadline);
        }

        // async post http msg with template T as payload
        template<typename T>
        bool post(const url& dest, const T& payload, const time_point& deadline = time_point::maximum()) {
            variant payload_v;
            to_variant(payload, payload_v);
            return enqueue(dest, payload_v, deadline);
        }

        // sync post http msg with template T as payload
        template<typename T>
        variant post_sync(const fc::url& dest, const T& payload, const time_point& deadline = time_point::maximum()) {
            return my->post_sync(dest, payload, deadline);
        }

        /*http_client& get_client() {
           return *my;
        }*/

    private:
        std::unique_ptr<http_client> my;
        std::list<async_http_msg> msg_queue;
        std::list<async_http_msg> send_msg_queue;
        std::mutex msg_queue_lock;
    };
}
