#include <chrono>
#include <iomanip>
#include <iostream>
#include <string>

#include <boost/asio.hpp>

#include "connect.hpp"
#include "define.hpp"
#include "log.hpp"
#include "node.hpp"
#include "pktmanage.hpp"
#include "security.hpp"

static bool parse_genesis(boost::chrono::system_clock::time_point& out_time_point, const char* time_format);
static void print_hint();
/**
 * entry for UranusNode
 * @param argc
 * @param argv
 * @return
 *
 * The following options are available:
 * -g   Set genesis block time
 */
int main(int argc, char* argv[]) {
    ultrainio::UltrainLog::Init("./log");

    const char* default_genesis = "2018-5-21 13:58:0";

    boost::asio::io_service io_service;
    ultrainio::UranusNode node(io_service);
    boost::chrono::system_clock::time_point genesis;
    if (argc % 2 != 1) {
        print_hint();
        return 1;
    }
    for (int i = 1; i < argc; i++) {
        std::string p = std::string(argv[i]);
        if (std::string("-g") == p) {
            default_genesis = argv[++i];
        }
    }
    // set default genesis
    boost::chrono::system_clock::time_point tp;
    if (!parse_genesis(tp, default_genesis)) {
        print_hint();
        return 1;
    }
    ultrainio::UranusNode::GENESIS = tp;

    while(!node.ready_to_join()) {}

    if (!node.startup()) {
        return 1;
    }
    LOG_INFO << "uranus algorithm begin" << std::endl;
    while (true) {
        try {
            node.run();
        } catch (const std::exception& e) {
            std::cerr << "Exception:" << e.what() << std::endl;
            return 1;
        }
        node.reset();
    }
}

static bool parse_genesis(boost::chrono::system_clock::time_point& out_time_point, const char* time_format)
{
    if (!time_format) {
        LOG_INFO << "genesis time parameter error." << std::endl;
        return false;
    }
    std::tm t;
    if (6 != std::sscanf(time_format, "%d-%d-%d %d:%d:%d", &t.tm_year, &t.tm_mon, &t.tm_mday, &t.tm_hour, &t.tm_min, &t.tm_sec)) {
        LOG_INFO << "format error : " << std::string(time_format) << std::endl;
        return false;
    }
    t.tm_year -= 1900;
    t.tm_mon -= 1;
    out_time_point = boost::chrono::system_clock::from_time_t(std::mktime(&t));
    return true;
}

static void print_hint() {
    std::cout << "config genesis : " << std::endl;
    std::cout << "    -g [--genesis] set genesis time " << std::endl;
}
