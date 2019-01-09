#include "rpos/BlockMsgPool.h"

#include <crypto/PrivateKey.h>
#include <rpos/Node.h>
#include <rpos/Seed.h>
#include <rpos/StakeVoteBase.h>
#include <rpos/StakeVoteFactory.h>
#include <rpos/Vrf.h>

namespace ultrainio {
    BlockMsgPool::BlockMsgPool(uint32_t blockNum) : m_blockNum(blockNum) {
#ifdef CONSENSUS_VRF
        m_stakeVote = StakeVoteFactory::createVrf(m_blockNum, nullptr);
#else
        m_stakeVote = StakeVoteFactory::createRandom(m_blockNum, nullptr);
#endif
    }

    std::shared_ptr<StakeVoteBase> BlockMsgPool::getVoterSys() {
        ULTRAIN_ASSERT(m_stakeVote, chain::chain_exception, "m_stakeVote is nullptr");
        return m_stakeVote;
    }
}
