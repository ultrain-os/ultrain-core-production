#include "rpos/Genesis.h"

#include <rpos/Config.h>

namespace ultrainio {
    const std::string Genesis::kGenesisAccount("genesis");

    fc::time_point Genesis::s_time;

    int Genesis::s_genesisStartupTime = 60;

    int Genesis::s_genesisStartupBlockNum = 60 * Config::kAverageBlockPerMinutes;

    // set in config.ini for single test net
    std::string Genesis::s_genesisPk("UTR7fzCYqCvi5WiAq6sVWqhqwcJ9UXuefASqptqa76hLV4gfU7LdD");

    std::string Genesis::s_genesisBlsPk("149342d0565f1645e40cd5aaaba7c6a29589d9346adb9c78e990b9f0d111dd3f48f23910fd96cf1d888fd9e841e8bc54934a572058158ed43edb4bb97c5672ef01");
}
