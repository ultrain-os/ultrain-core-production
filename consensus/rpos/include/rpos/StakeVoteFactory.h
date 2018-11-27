#pragma once

#include <memory>
#include <core/Redefined.h>

namespace ultrainio {
    struct CommitteeState;
    class StakeVoteBase;

    class StakeVoteFactory {
    public:
        static std::shared_ptr<StakeVoteBase> createRandom(uint32_t blockNum,
                std::shared_ptr<CommitteeState> committeeStatePtr, const BlockIdType& rand);
    };
}