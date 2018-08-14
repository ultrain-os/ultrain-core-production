#pragma once

#include <string>

#include <boost/log/trivial.hpp>

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

    class UltrainLog {
    public:
        static void init(const std::string& dir);

        static std::string convert2Hex(const std::string& str);
    };

}
