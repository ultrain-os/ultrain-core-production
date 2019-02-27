#include <lightclient/LightClientMgr.h>

#include <lightclient/LightClient.h>

namespace ultrainio {
    std::shared_ptr<LightClientMgr> LightClientMgr::s_self = nullptr;

    std::shared_ptr<LightClientMgr> LightClientMgr::getInstance() {
        if (!s_self) {
            s_self = std::make_shared<LightClientMgr>();
        }
        return s_self;
    }

    std::shared_ptr<LightClient> LightClientMgr::getLightClient(uint64_t chainName) {
        for (const auto& e : m_lightClientList) {
            if (e->chainName() == chainName) {
                return e;
            }
        }
        std::shared_ptr<LightClient> lightClient = std::make_shared<LightClient>(chainName);
        m_lightClientList.push_back(lightClient);
        return lightClient;
    }
}