#include <lightclient/LightClient.h>

namespace ultrainio {
    LightClient::LightClient(uint64_t chainName) : m_chainName(chainName) {}

    uint64_t LightClient::chainName() const {
        return m_chainName;
    }

    int LightClient::accept(const BlockHeader& blockHeader) {
        return 0;
    }
}