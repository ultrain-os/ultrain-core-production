#pragma once

#include <string>
#include <boost/chrono.hpp>
#include <fc/time.hpp>

namespace ultrainio {
    class Genesis {
    public:
        static const std::string kGenesisAccount;

        static fc::time_point s_time;

        static int s_genesisStartupTime;

        static int s_genesisStartupBlockNum;

        static std::string s_genesisPk;

        static std::string s_genesisBlsPk;
    };
}
