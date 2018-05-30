/**
 *  @file
 *  @copyright defined in ultrain/LICENSE.txt
 */
#include <ultrainio/producer_uranus_plugin/producer_uranus_plugin.hpp>

#include <iostream>
#include <unistd.h>
#include <string>
#include <boost/asio.hpp>
#include <chrono>

#include <fc/log/logger.hpp>

#include <ultrainio/producer_uranus_plugin/connect.hpp>
#include <ultrainio/producer_uranus_plugin/define.hpp>
#include <ultrainio/producer_uranus_plugin/log.hpp>
#include <ultrainio/producer_uranus_plugin/node.hpp>
#include <ultrainio/producer_uranus_plugin/node_state.hpp>
#include <ultrainio/producer_uranus_plugin/pktmanage.hpp>
#include <ultrainio/producer_uranus_plugin/security.hpp>

namespace ultrainio {
    static appbase::abstract_plugin &_producer_uranus_plugin = app().register_plugin<producer_uranus_plugin>();

    static bool parse_genesis(boost::chrono::system_clock::time_point &out_time_point, const char *time_format);

    class producer_uranus_plugin_impl {
    public:
        producer_uranus_plugin_impl(boost::asio::io_service &io) {}

        boost::asio::io_service io_service;
    };

    producer_uranus_plugin::producer_uranus_plugin() : my(new producer_uranus_plugin_impl(app().get_io_service())) {}

    producer_uranus_plugin::~producer_uranus_plugin() {}

    void producer_uranus_plugin::set_program_options(options_description &, options_description &cfg) {
        cfg.add_options()
                ("option-name", bpo::value<string>()->default_value("default value"),
                 "Option Description");
    }

    void producer_uranus_plugin::plugin_initialize(const variables_map &options) {
        if (options.count("option-name")) {
            // Handle the option
        }
    }

    void producer_uranus_plugin::plugin_startup() {
        // Make the magic happen
        ilog("uranus algorithm starting");
        ultrainio::UltrainLog::Init("./log");

        boost::asio::io_service io_service;
        ultrainio::UranusNode node(io_service);

        // set default genesis
        const char *default_genesis = "2018-5-21 13:58:0";
        boost::chrono::system_clock::time_point tp;
        if (!parse_genesis(tp, default_genesis)) {
            return;
        }
        ultrainio::UranusNode::GENESIS = tp;

        while (!node.ready_to_join()) {}

        if (!node.startup()) {
            return;
        }
        LOG_INFO << "uranus algorithm begin" << std::endl;
        while (true) {
            try {
                node.run();
            } catch (const std::exception &e) {
                std::cerr << "Exception:" << e.what() << std::endl;
                return;
            }
            node.reset();
        }
    }

    void producer_uranus_plugin::plugin_shutdown() {
        // OK, that's enough magic
    }

    static bool parse_genesis(boost::chrono::system_clock::time_point &out_time_point, const char *time_format) {
        if (!time_format) {
            LOG_INFO << "genesis time parameter error." << std::endl;
            return false;
        }
        std::tm t;
        if (6 != std::sscanf(time_format, "%d-%d-%d %d:%d:%d", &t.tm_year, &t.tm_mon, &t.tm_mday, &t.tm_hour, &t.tm_min,
                             &t.tm_sec)) {
            LOG_INFO << "format error : " << std::string(time_format) << std::endl;
            return false;
        }
        t.tm_year -= 1900;
        t.tm_mon -= 1;
        out_time_point = boost::chrono::system_clock::from_time_t(std::mktime(&t));
        return true;
    }
}
