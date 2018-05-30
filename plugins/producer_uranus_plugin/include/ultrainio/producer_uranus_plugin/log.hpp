#pragma once

#include <string>

#include <boost/log/trivial.hpp>

#include "define.hpp"
#include "node_state.hpp"
#include "pktmanage.hpp"

namespace logging = boost::log;
namespace src = boost::log::sources;
namespace keywords = boost::log::keywords;
namespace sinks = boost::log::sinks;
namespace expr = boost::log::expressions;

namespace ultrainio {

#define LOG_DEBUG   BOOST_LOG_TRIVIAL(debug)
#define LOG_INFO    BOOST_LOG_TRIVIAL(info) << "(" << __FILE__ << ", " << __LINE__ << ") "
#define LOG_WARNING BOOST_LOG_TRIVIAL(warning) << "(" << __FILE__ << ", " << __LINE__ << ") "
#define LOG_ERROR   BOOST_LOG_TRIVIAL(error) << "(" << __FILE__ << ", " << __LINE__ << ") "

#define OUTPUT_KEY_POINTER(state)                                                                   \
            LOG_INFO << "consensus_key_point : { " << "role:" << state.role << ", phase:" << state.phase      \
            << ",block_id:" << state.block_id                                                       \
            << ",self_txs_hash:" << UltrainLog::get_unprintable(state.self_txs_hash)                \
            << ",self_proof:" << UltrainLog::get_unprintable(state.self_proof)                      \
            << ",min_txs_hash:" << UltrainLog::get_unprintable(state.min_txs_hash)                  \
            << ",min_proof:" << UltrainLog::get_unprintable(state.min_proof)                        \
            << "}"

    class UltrainLog {
    public:
        static void Init(const std::string &dir);

        static std::string get_unprintable(const std::string &str);

        static void displayMsgInfo(MsgInfo &msg_info);

        static void displayPropose(ProposeMsg &msg_info);

        static void displayBlock(TxsBlock &msg_info);

        static void displayEcho(EchoMsg &msg_info);
    };

}
