#include <rpos/StakeVoteFactory.h>

#include <rpos/RoleRandom.h>
#include <rpos/StakeVoteRandom.h>
#include <rpos/StakeVoteVrf.h>

namespace ultrainio {
    std::shared_ptr<StakeVoteBase> StakeVoteFactory::createRandom(uint32_t blockNum,
            std::shared_ptr<CommitteeState> committeeStatePtr, const BlockIdType &rand) {
        StakeVoteBase* stakeVotePtr = new StakeVoteRandom(blockNum, committeeStatePtr, RoleRandom(rand));
        return std::shared_ptr<StakeVoteBase>(stakeVotePtr);
    }

    std::shared_ptr<StakeVoteBase> StakeVoteFactory::createVrf(uint32_t blockNum,
                                                    std::shared_ptr<CommitteeState> committeeStatePtr) {
        return std::make_shared<StakeVoteVrf>(blockNum, committeeStatePtr);
    }
}
