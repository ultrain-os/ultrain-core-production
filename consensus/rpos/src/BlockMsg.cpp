#include "rpos/BlockMsg.h"

#include <crypto/PrivateKey.h>
#include <rpos/Node.h>
#include <rpos/Seed.h>
#include <rpos/StakeVote.h>
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

    void BlockMsg::insert(const EchoMsg& echoMsg) {
        ConsensusPhase phase = echoMsg.phase;
        uint32_t baxCount = echoMsg.baxCount;
        PhaseMsgPtr phaseMessagePtr = initIfNeed(phase, baxCount);
        ULTRAIN_ASSERT(phaseMessagePtr->m_phase == phase && phaseMessagePtr->m_baxCount == baxCount,
                chain::chain_exception, "phase or baxCount not equal");
        phaseMessagePtr->insert(echoMsg);
    }

    void BlockMsg::insert(const ProposeMsg& proposeMsg) {
        m_proposeMsgList.push_back(proposeMsg);
    }

    void BlockMsg::moveToNewStep(uint32_t blockNum, ConsensusPhase phase, int baxCount) {
        ULTRAIN_ASSERT(blockNum == m_blockNum, chain::chain_exception, "blockNum is equal");
        if (!m_proposerProof.isValid()) {
            ultrainio::chain::block_id_type blockId = UranusNode::getInstance()->getPreviousHash();
            std::string previousHash(blockId.data());
            // #### This line has to run first, all the following caculation depends on this line. ###
            m_voterSystem = StakeVote::create(blockNum, nullptr);
            Seed proposerSeed(previousHash, blockNum, kPhaseBA0, 0);
            PrivateKey privateKey = StakeVote::getMyPrivateKey();
            m_proposerProof = Vrf::vrf(privateKey, proposerSeed, Vrf::kProposer);
            int stakes = m_voterSystem->getStakes(StakeVote::getMyAccount(), UranusNode::getInstance()->getNonProducingNode());
            double proposerRatio = m_voterSystem->getProposerRatio();
            m_voterCountAsProposer = m_voterSystem->count(m_proposerProof, stakes, proposerRatio);
        }
        PhaseMsgPtr phaseMessagePtr = initIfNeed(phase, baxCount);
        phaseMessagePtr->moveToNewStep(blockNum, phase, baxCount);
    }

    Proof BlockMsg::getVoterProof(ConsensusPhase phase, int baxCount) {
        PhaseMsgPtr phaseMessagePtr = initIfNeed(phase, baxCount);
        return phaseMessagePtr->m_proof;
    }

    int BlockMsg::getVoterVoterCount(ConsensusPhase phase, int baxCount) {
        PhaseMsgPtr phaseMessagePtr = initIfNeed(phase, baxCount);
        return phaseMessagePtr->m_voterCountAsVoter;
    }

    bool BlockMsg::newRound(ConsensusPhase phase, int baxCount) {
        if (kPhaseBA0 == phase && 0 == baxCount) {
            return true;
        }
        return false;
    }

    std::shared_ptr<StakeVote> BlockMsg::getVoterSys() {
        ULTRAIN_ASSERT(m_voterSystem, chain::chain_exception, "m_voterSystem is nullptr");
        return m_voterSystem;
    }
}
