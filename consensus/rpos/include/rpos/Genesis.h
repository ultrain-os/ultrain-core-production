#pragma once

#include <boost/chrono.hpp>

namespace ultrainio {
    class Genesis {
    public:
        static boost::chrono::system_clock::time_point s_time;
    };
}