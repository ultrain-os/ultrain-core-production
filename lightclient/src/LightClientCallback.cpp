#include <lightclient/LightClientCallback.h>

namespace ultrainio {
    LightClientCallback::~LightClientCallback() {}

    void LightClientCallback::onConfirmed(const std::list<BlockHeader>&) {};

    void LightClientCallback::onError(LightClientError errorType, const BlockHeader&) {}
}
