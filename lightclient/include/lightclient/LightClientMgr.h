#pragma once

#include <list>
#include <memory>

namespace ultrainio {
    class LightClient;

    class LightClientMgr {
    public:
        static std::shared_ptr<LightClientMgr> getInstance();

        std::shared_ptr<LightClient> getLightClient(uint64_t chainName);

    private:
        static std::shared_ptr<LightClientMgr> s_self;

        std::list<std::shared_ptr<LightClient>> m_lightClientList;
    };
}