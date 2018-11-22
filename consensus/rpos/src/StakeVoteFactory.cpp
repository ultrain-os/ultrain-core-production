#include <rpos/StakeVoteFactory.h>

#include <rpos/StakeVoteRandom.h>

namespace ultrainio {
    std::shared_ptr<StakeVoteBase> StakeVoteFactory::createRandom(uint32_t blockNum,
            std::shared_ptr<CommitteeState> committeeStatePtr, const BlockIdType &rand) {
        StakeVoteBase* stakeVotePtr = new StakeVoteRandom(blockNum, committeeStatePtr, rand);
        return std::shared_ptr<StakeVoteBase>(stakeVotePtr);
    }
}
