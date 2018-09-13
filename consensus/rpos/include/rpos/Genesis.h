#pragma once

#include <string>
#include <boost/chrono.hpp>

namespace ultrainio {
    class Genesis {
    public:
        static const std::string kGenesisAccount;

        static boost::chrono::system_clock::time_point s_time;

        static int s_genesisStartupTime;

        static int s_genesisStartupBlockNum;

        static std::string s_genesisPk;
    };
}