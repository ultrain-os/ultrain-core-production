#pragma once

#include <list>

#include <core/types.h>

namespace ultrainio {
    enum LightClientError {
        //kNonError = 0,
        kOutOfRange,
        kPreviousError,
        kBlsVoterSetNotMatch
    };

    class LightClientCallback {
    public:
        virtual ~LightClientCallback();

        virtual void onConfirmed(const std::list<BlockHeader>&);

        virtual void onError(LightClientError errorType, const BlockHeader&);
    };
}