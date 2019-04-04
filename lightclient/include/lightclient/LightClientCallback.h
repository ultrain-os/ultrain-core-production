#pragma once

#include <list>

#include <core/types.h>

namespace ultrainio {
    class LightClientCallback {
    public:
        enum LightClientError {
            kNone = 0,
            kOutOfRange,
            kSignatureError,
        };

        virtual ~LightClientCallback();

        virtual void onConfirmed(const std::list<BlockHeader>&);

        virtual void onError(LightClientError errorType, const BlockHeader&);
    };

    using LightClientError = LightClientCallback::LightClientError;
}