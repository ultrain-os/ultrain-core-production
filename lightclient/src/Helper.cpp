#include <lightclient/Helper.h>

namespace ultrainio {
    bool Helper::isGenesis(const BlockHeader& blockHeader) {
        return std::string(blockHeader.proposer) == std::string("genesis");
    }
}