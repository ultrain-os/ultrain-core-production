#include "rpos/BlockMsg.h"

#include <crypto/PrivateKey.h>
#include <rpos/Node.h>
#include <rpos/Seed.h>
#include <rpos/StakeVoteBase.h>
#include <rpos/StakeVoteFactory.h>
#include <rpos/Vrf.h>

namespace ultrainio {
    BlockMsg::BlockMsg(uint32_t blockNum) : m_blockNum(blockNum) {}

    PhaseMsgPtr BlockMsg::initIfNeed(ConsensusPhase phase, int baxCount) {
        int key = phase + baxCount;
        auto itor = m_phaseMessageMap.find(key);
        if (itor == m_phaseMessageMap.end()) {
            PhaseMsgPtr phaseMessagePtr = std::make_shared<PhaseMsg>();
            phaseMessagePtr->m_phase = phase;
            phaseMessagePtr->m_baxCount = baxCount;
            m_phaseMessageMap.insert(make_pair(key, phaseMessagePtr));
            return phaseMessagePtr;
        }
        return itor->second;
    }

    void BlockMsg::insert(const ProposeMsg& proposeMsg) {
        m_proposeMsgList.push_back(proposeMsg);
    }

    void BlockMsg::moveToNewStep(uint32_t blockNum, ConsensusPhase phase, int baxCount) {
        ULTRAIN_ASSERT(blockNum == m_blockNum, chain::chain_exception, "blockNum is equal");
        if (!m_stakeVote) {
            ultrainio::chain::block_id_type blockId = UranusNode::getInstance()->getPreviousHash();
            std::string previousHash(blockId.data());
            // #### This line has to run first, all the following caculation depends on this line. ###
            m_stakeVote = StakeVoteFactory::createRandom(blockNum, nullptr, blockId);
            m_isProposer = m_stakeVote->isProposer(StakeVoteBase::getMyAccount(), UranusNode::getInstance()->getNonProducingNode());
        }
        PhaseMsgPtr phaseMessagePtr = initIfNeed(phase, baxCount);
        phaseMessagePtr->moveToNewStep(blockNum, phase, baxCount);
    }

    bool BlockMsg::isProposer() const {
        return m_isProposer;
    }

    bool BlockMsg::isVoter(ConsensusPhase phase, int baxCount) {
        PhaseMsgPtr phaseMessagePtr = initIfNeed(phase, baxCount);
        return phaseMessagePtr->isVoter();
    }

    bool BlockMsg::newRound(ConsensusPhase phase, int baxCount) {
        if (kPhaseBA0 == phase && 0 == baxCount) {
            return true;
        }
        return false;
    }

    std::shared_ptr<StakeVoteBase> BlockMsg::getVoterSys() {
        ULTRAIN_ASSERT(m_stakeVote, chain::chain_exception, "m_stakeVote is nullptr");
        return m_stakeVote;
    }
}
