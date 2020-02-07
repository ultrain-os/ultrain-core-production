#include <rpos/MsgMgr.h>

#include <ultrainio/chain/exceptions.hpp>

#include <rpos/Config.h>
#include <rpos/Node.h>
#include <rpos/NodeInfo.h>
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
        std::shared_ptr<StakeVoteBase> stakeVotePtr = getStakeVote(blockNum);
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

    bool MsgMgr::hasProposer(uint32_t blockNum, size_t& outIndex) {
        std::shared_ptr<StakeVoteBase> stakeVotePtr = getStakeVote(blockNum);
        ULTRAIN_ASSERT(stakeVotePtr != nullptr, chain::chain_exception, "not init StakeVote");
        bool has = false;
        const std::vector<std::string>& accountList = NodeInfo::getInstance()->getAccountList();
        outIndex = accountList.size();
        uint32_t currPriority = Config::kDesiredProposerNumber;
        for (size_t i = 0; i < accountList.size(); i++) {
            if (stakeVotePtr->isProposer(accountList[i], Node::getInstance()->getNonProducingNode())) {
                has = true;
                uint32_t priority = stakeVotePtr->proposerPriority(accountList[i], kPhaseBA0, 0);
                if (priority < currPriority) {
                    currPriority = priority;
                    outIndex = i;
                }
            }
        }
        return has;
    }

    bool MsgMgr::hasProposer(uint32_t blockNum) {
        size_t index;
        return hasProposer(blockNum, index);
    }

    bool MsgMgr::hasVoter(uint32_t blockNum, ConsensusPhase phase, int baxCount, std::vector<size_t>& outV) {
        std::shared_ptr<StakeVoteBase> stakeVotePtr = getStakeVote(blockNum);
        ULTRAIN_ASSERT(stakeVotePtr != nullptr, chain::chain_exception, "not init StakeVote");
        bool has = false;
        const std::vector<std::string>& accountList = NodeInfo::getInstance()->getAccountList();
        for (size_t i = 0; i < accountList.size(); i++) {
            if (stakeVotePtr->isVoter(accountList[i], phase, baxCount, Node::getInstance()->getNonProducingNode())) {
                has = true;
                outV.push_back(i);
            }
        }
        return has;
    }

    bool MsgMgr::hasVoter(uint32_t blockNum, ConsensusPhase phase, int baxCount) {
        std::vector<size_t> v;
        return hasVoter(blockNum, phase, baxCount, v);
    }

    void MsgMgr::clearSomeBlockMessage(uint32_t blockNum) {
        for (auto itor = m_blockMsgPoolMap.begin(); itor != m_blockMsgPoolMap.end();) {
            if (blockNum - Config::kMaxLaterNumber > itor->first) {
                ilog("clear block msg for blockNum = ${blockNum}", ("blockNum", itor->first));
                m_blockMsgPoolMap.erase(itor++);
            } else {
                itor++;
            }
        }
    }

    std::shared_ptr<StakeVoteBase> MsgMgr::getStakeVote(uint32_t blockNum) {
        BlockMsgPoolPtr blockMsgPoolPtr = getBlockMsgPool(blockNum);
        return blockMsgPoolPtr->getStakeVote();
    }
}
