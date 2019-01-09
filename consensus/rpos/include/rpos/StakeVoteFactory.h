#pragma once

#include <memory>

namespace ultrainio {
    struct CommitteeState;
    class StakeVoteBase;

    class StakeVoteFactory {
    public:
        static std::shared_ptr<StakeVoteBase> createRandom(uint32_t blockNum,
                std::shared_ptr<CommitteeState> committeeStatePtr);
        static std::shared_ptr<StakeVoteBase> createVrf(uint32_t blockNum,
                std::shared_ptr<CommitteeState> committeeStatePtr);
    };
}