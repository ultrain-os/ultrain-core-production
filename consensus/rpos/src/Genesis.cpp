#include "rpos/Genesis.h"

#include <rpos/Config.h>

namespace ultrainio {
    const std::string Genesis::kGenesisAccount("genesis");

    boost::chrono::system_clock::time_point Genesis::s_time;

    int Genesis::s_genesisStartupTime = 60;

    int Genesis::s_genesisStartupBlockNum = 60 * Config::kAverageBlockPerMinutes;

    // set in config.ini for single test net
    std::string Genesis::s_genesisPk("9bb728c7e29e2f7a0c34c954382ea3ad8e4a93bf3455f9438afa086b3d67304b3b2a7dd684f0cda7d71ca9116128b1b7e49f31b2b10c0174b21b98c82161294da6d88bdbc92a897006e547c5ee5a3277f74fb4f9c0b9d8339fc3de8e6f467aac558859cfd0b9d3eea828e29b33b4f819f14ffbbcc7bea2ac585228fa8f12c42f");
}
