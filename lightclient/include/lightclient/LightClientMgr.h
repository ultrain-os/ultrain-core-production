#pragma once

#include <list>
#include <memory>

namespace ultrainio {
    class LightClient;
    class LightClientProducer;

    class LightClientMgr {
    public:
        static std::shared_ptr<LightClientMgr> getInstance();

        std::shared_ptr<LightClient> getLightClient(uint64_t chainName);

        std::shared_ptr<LightClientProducer> getLightClientProducer();

    private:
        static std::shared_ptr<LightClientMgr> s_self;

        std::list<std::shared_ptr<LightClient>> m_lightClientList;

        std::shared_ptr<LightClientProducer> m_lightClientProducer;
    };
}