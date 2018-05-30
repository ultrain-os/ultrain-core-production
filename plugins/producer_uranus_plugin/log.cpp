#include <ultrainio/producer_uranus_plugin/log.hpp>

#include <iostream>
#include <string>

#include <boost/filesystem.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sources/global_logger_storage.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/attributes/named_scope.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/detail/format.hpp>
#include <vector>

#include <ultrainio/producer_uranus_plugin/define.hpp>
#include <ultrainio/producer_uranus_plugin/connect.hpp>

namespace ultrainio {

    namespace logging = boost::log;
    namespace src = boost::log::sources;
    namespace keywords = boost::log::keywords;
    namespace sinks = boost::log::sinks;
    namespace expr = boost::log::expressions;

    void UltrainLog::Init(const std::string &dir) {
        if (boost::filesystem::exists(dir) == false) {
            boost::filesystem::create_directories(dir);
        }

        auto pSink = logging::add_file_log
            (
                keywords::open_mode = std::ios::app,
                keywords::file_name = dir + "/%Y%m%d_" + boost::asio::ip::host_name() + ".log",
                keywords::rotation_size = 10 * 1024 * 1024,
                keywords::time_based_rotation = sinks::file::rotation_at_time_point(0, 0, 0),
                keywords::format =
                    (
                        expr::stream
                            << "[" << expr::format_date_time<boost::posix_time::ptime>("TimeStamp","%Y-%m-%d %H:%M:%S.%f")
                            << "] [" << logging::trivial::severity
                            << "] " << expr::smessage
                    )
            );

        pSink->locked_backend()->auto_flush(true);//使日志实时更新
        logging::add_common_attributes();
    }

    std::string UltrainLog::get_unprintable(const std::string& str) {
        std::string hex_str;
        uint8_t* c_str = (uint8_t*)str.data();
        char hex_char[3];
        for (int i = 0; i < str.length(); i++) {
            snprintf(hex_char, sizeof(hex_char), "%02x", c_str[i]);
            hex_str.append(hex_char);
            if (i != 0 && i%8 == 0) {
                hex_str.append(" ");
            }
        }
        return hex_str;
    }

    void UltrainLog::displayMsgInfo(MsgInfo &msg_info) {
        LOG_DEBUG << "type = " << msg_info.type;
        LOG_DEBUG << "phase = " << msg_info.phase;
        LOG_DEBUG << "blockid = " << msg_info.block_id;
        LOG_DEBUG << "txs = " << get_unprintable(msg_info.txs);
        LOG_DEBUG << "txs_signature = " << get_unprintable(msg_info.txs_signature);
        LOG_DEBUG << "proposer_pk = " << get_unprintable(msg_info.proposer_pk);
        LOG_DEBUG << "txs_hash_signature = " << get_unprintable(msg_info.txs_hash_signature);
        LOG_DEBUG << "pk = " << get_unprintable(msg_info.pk);
    }

    void UltrainLog::displayPropose(ProposeMsg &msg_info) {
        LOG_DEBUG << "phase = " << msg_info.phase;
        LOG_DEBUG << "txs = " << msg_info.txs;
        LOG_DEBUG << "txs_hash = " << get_unprintable(msg_info.txs_hash);
        LOG_DEBUG << "txs_signature = " << get_unprintable(msg_info.txs_signature);
        LOG_DEBUG << "proposer_pk = " << get_unprintable(msg_info.proposer_pk);
        LOG_DEBUG << "proposer_role_vrf = " << get_unprintable(msg_info.proposer_role_vrf);
    }

    void UltrainLog::displayEcho(EchoMsg &msg_info) {
        LOG_DEBUG << "phase = " << msg_info.phase;
        LOG_DEBUG << "txs_hash = " << get_unprintable(msg_info.txs_hash);
        LOG_DEBUG << "txs_signature = " << get_unprintable(msg_info.txs_signature);
        LOG_DEBUG << "proposer_pk = " << get_unprintable(msg_info.proposer_pk);
        LOG_DEBUG << "proposer_role_vrf = " << get_unprintable(msg_info.proposer_role_vrf);
        LOG_DEBUG << "pool size = " << msg_info.pk_pool.size() << " list : ";
        for (auto sign_it = msg_info.pk_pool.begin(); sign_it != msg_info.pk_pool.end(); sign_it++) {
            LOG_DEBUG << get_unprintable(*sign_it) << "||";
        }
    }

    void UltrainLog::displayBlock(TxsBlock &msg_info) {
        LOG_DEBUG << "start block'info : ";
        LOG_DEBUG << "txs = " << msg_info.txs;
        LOG_DEBUG << "txs_hash = " << get_unprintable(msg_info.txs_hash);
        LOG_DEBUG << "txs_signature = " << get_unprintable(msg_info.txs_signature);
        LOG_DEBUG << "proposer_pk = " << get_unprintable(msg_info.proposer_pk);
        LOG_DEBUG << "proposer_role_vrf = " << get_unprintable(msg_info.proposer_role_vrf);
        LOG_DEBUG << "pool size = " << msg_info.pk_pool.size() << " list : ";
        for (auto sign_it = msg_info.pk_pool.begin(); sign_it != msg_info.pk_pool.end(); sign_it++) {
            LOG_DEBUG << get_unprintable(*sign_it) << "|";
        }
        LOG_DEBUG << "end block'info";
    }
}

