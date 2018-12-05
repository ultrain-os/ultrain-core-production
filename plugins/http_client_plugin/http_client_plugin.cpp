/**
 *  @file
 *  @copyright defined in ultrain/LICENSE.txt
 */
#include <ultrainio/http_client_plugin/http_client_plugin.hpp>
#include <ultrainio/chain/controller.hpp>
#include <ultrainio/chain/exceptions.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <fstream>
#include <thread>
#include <ultrainio/chain_plugin/chain_plugin.hpp>
#include <chrono>

namespace ultrainio {

http_client_plugin::http_client_plugin():my(new http_client()){}
http_client_plugin::~http_client_plugin(){}

void http_client_plugin::set_program_options(options_description&, options_description& cfg) {
    cfg.add_options()
        ("https-client-root-cert", boost::program_options::value<vector<string>>()->composing()->multitoken(),
         "PEM encoded trusted root certificate (or path to file containing one) used to validate any TLS connections made.  (may specify multiple times)\n")
        ("https-client-validate-peers", boost::program_options::value<bool>()->default_value(true),
         "true: validate that the peer certificates are valid and trusted, false: ignore cert errors");

}

void http_client_plugin::plugin_initialize(const variables_map& options) {
    try {
        if( options.count( "https-client-root-cert" )) {
            const std::vector<std::string> root_pems = options["https-client-root-cert"].as<std::vector<std::string>>();
            for( const auto& root_pem : root_pems ) {
                std::string pem_str = root_pem;
                if( !boost::algorithm::starts_with( pem_str, "-----BEGIN CERTIFICATE-----\n" )) {
                    try {
                        auto infile = std::ifstream( pem_str );
                        std::stringstream sstr;
                        sstr << infile.rdbuf();
                        pem_str = sstr.str();
                        ULTRAIN_ASSERT( boost::algorithm::starts_with( pem_str, "-----BEGIN CERTIFICATE-----\n" ),
                                        chain::invalid_http_client_root_cert,
                                        "File does not appear to be a PEM encoded certificate" );
                    } catch ( const fc::exception& e ) {
                        elog( "Failed to read PEM ${f} : ${e}", ("f", root_pem)( "e", e.to_detail_string()));
                    }
                }

                try {
                    my->add_cert( pem_str );
                } catch ( const fc::exception& e ) {
                    elog( "Failed to read PEM : ${e} \n${pem}\n", ("pem", pem_str)( "e", e.to_detail_string()));
                }
            }
        }

        my->set_verify_peers( options.at( "https-client-validate-peers" ).as<bool>());
    } FC_LOG_AND_RETHROW()
}

bool http_client_plugin::enqueue(const fc::url& dest, const variant& payload, const time_point& deadline) {
    std::lock_guard<std::mutex> guard(msg_queue_lock);

    if (msg_queue.size() >= MAX_HTTP_MSG_QUEUE_SIZE) {
        elog("http msg queue is full!!! Discard msg with url: ${url}", ("url", dest));
        return false;
    }
    msg_queue.emplace_back(dest, payload, deadline);
    return true;
}

void http_client_plugin::plugin_startup() {
    chain::controller& cc = app().get_plugin<chain_plugin>().chain();
    cc.http_async_post.connect(boost::bind(&http_client_plugin::post_async, this, _1, _2, _3));
    boost::function0<void> f =  boost::bind(&http_client_plugin::schedule, this);
    std::thread thrd(f);
    thrd.detach();
}

void http_client_plugin::plugin_shutdown() {

}

void http_client_plugin::schedule() {
    std::list<async_http_msg> send_msg_queue;
    std::chrono::milliseconds sleep_duration(50);
    while (true)
    {
        msg_queue_lock.lock();
        if (msg_queue.empty()) {
            msg_queue_lock.unlock();
            std::this_thread::sleep_for(sleep_duration);
        } else {
            send_msg_queue.swap(msg_queue);
            msg_queue_lock.unlock();
            for (auto it = send_msg_queue.begin(); it != send_msg_queue.end(); ++it) {
                try {
                    my->post(it->dest, it->payload, it->deadline);
                } catch (...) { // TODO: We skip exception when no response happends, but need to process some type of exception if we can
                }
            }
            send_msg_queue.clear();
        }
    }
}

}
