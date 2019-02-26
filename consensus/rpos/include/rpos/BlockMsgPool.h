#pragma once

#include <map>
#include <memory>
#include <vector>

#include <core/Message.h>
#include <rpos/Proof.h>

namespace ultrainio {
    class StakeVoteBase;

    class BlockMsgPool {
    public:
        BlockMsgPool(uint32_t blockNum);
        std::shared_ptr<StakeVoteBase> getVoterSys();
    private:
        uint32_t m_blockNum = 0;
        std::shared_ptr<StakeVoteBase> m_stakeVote = nullptr;

        friend class MsgMgr;

        // Only for debug purpose.
        friend class Scheduler;
    };

    typedef std::shared_ptr<BlockMsgPool> BlockMsgPoolPtr;
}
