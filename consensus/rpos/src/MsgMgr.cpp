#include <rpos/MsgMgr.h>

#include <ultrainio/chain/exceptions.hpp>

#include <rpos/Config.h>
#include <rpos/Node.h>
#include <rpos/Seed.h>
#include <rpos/StakeVoteBase.h>
#include <rpos/Vrf.h>

namespace ultrainio {
    using namespace std;

    // class MessageManager
    shared_ptr<MsgMgr> MsgMgr::s_self = nullptr;

    std::shared_ptr<MsgMgr> MsgMgr::getInstance() {
        if (!s_self) {
            s_self = std::shared_ptr<MsgMgr>(new MsgMgr());
        }
        return s_self;
    }

    void MsgMgr::moveToNewStep(uint32_t blockNum, ConsensusPhase phase, int baxCount) {
        ilog("moveToNewStep blockNum = ${blockNum}, phase = ${phase}, baxCount = ${baxCount}",
                ("blockNum", blockNum)("phase", static_cast<int>(phase))("baxCount", baxCount));
        BlockMsgPoolPtr blockMsgPoolPtr = nullptr;
        if (StakeVoteBase::newRound(phase, baxCount)) {
            clearSomeBlockMessage(blockNum);
        }
        std::shared_ptr<StakeVoteBase> stakeVotePtr = getVoterSys(blockNum);
        stakeVotePtr->moveToNewStep(blockNum, phase, baxCount);
    }

    BlockMsgPoolPtr MsgMgr::getBlockMsgPool(uint32_t blockNum) {
        ULTRAIN_ASSERT(blockNum > 1, chain::chain_exception, "blockNum should > 1");
        auto itor = m_blockMsgPoolMap.find(blockNum);
        if (itor == m_blockMsgPoolMap.end()) {
            BlockMsgPoolPtr blockMsgPoolPtr = std::make_shared<BlockMsgPool>(blockNum);
            m_blockMsgPoolMap.insert(make_pair(blockNum, blockMsgPoolPtr));
            return blockMsgPoolPtr;
        }
        return itor->second;
    }

#ifdef CONSENSUS_VRF
    Proof MsgMgr::getVoterProof(uint32_t blockNum, ConsensusPhase phase, int baxCount) {
        std::shared_ptr<StakeVoteBase> stakeVotePtr = getVoterSys(blockNum);
        ULTRAIN_ASSERT(stakeVotePtr != nullptr, chain::chain_exception, "not init StakeVote");
        return stakeVotePtr->getVoterProof(blockNum, phase, baxCount);
    }

    Proof MsgMgr::getProposerProof(uint32_t blockNum) {
        std::shared_ptr<StakeVoteBase> stakeVotePtr = getVoterSys(blockNum);
        ULTRAIN_ASSERT(stakeVotePtr != nullptr, chain::chain_exception, "not init StakeVote");
        return stakeVotePtr->getProposerProof(blockNum);
    }
#endif

    bool MsgMgr::isVoter(uint32_t blockNum, ConsensusPhase phase, int baxCount) {
        std::shared_ptr<StakeVoteBase> stakeVotePtr = getVoterSys(blockNum);
        ULTRAIN_ASSERT(stakeVotePtr != nullptr, chain::chain_exception, "not init StakeVote");
#ifdef CONSENSUS_VRF
        return stakeVotePtr->isVoter(StakeVoteBase::getMyAccount(), stakeVotePtr->getVoterProof(blockNum, phase, baxCount), UranusNode::getInstance()->getNonProducingNode());
#else
        return stakeVotePtr->isVoter(StakeVoteBase::getMyAccount(), phase, baxCount, UranusNode::getInstance()->getNonProducingNode());
#endif
    }

    bool MsgMgr::isProposer(uint32_t blockNum) {
        std::shared_ptr<StakeVoteBase> stakeVotePtr = getVoterSys(blockNum);
        ULTRAIN_ASSERT(stakeVotePtr != nullptr, chain::chain_exception, "not init StakeVote");
#ifdef CONSENSUS_VRF
        return stakeVotePtr->isProposer(StakeVoteBase::getMyAccount(), stakeVotePtr->getProposerProof(blockNum), UranusNode::getInstance()->getNonProducingNode());
#else
        return stakeVotePtr->isProposer(StakeVoteBase::getMyAccount(), UranusNode::getInstance()->getNonProducingNode());
#endif
    }

    void MsgMgr::clearSomeBlockMessage(uint32_t blockNum) {
        for (auto itor = m_blockMsgPoolMap.begin(); itor != m_blockMsgPoolMap.end();) {
            if (blockNum - Config::MAX_LATER_NUMBER > itor->first) {
                ilog("clear block msg for blockNum = ${blockNum}", ("blockNum", itor->first));
                m_blockMsgPoolMap.erase(itor++);
            } else {
                itor++;
            }
        }
    }

    std::shared_ptr<StakeVoteBase> MsgMgr::getVoterSys(uint32_t blockNum) {
        BlockMsgPoolPtr blockMsgPoolPtr = getBlockMsgPool(blockNum);
        return blockMsgPoolPtr->getVoterSys();
    }
}
