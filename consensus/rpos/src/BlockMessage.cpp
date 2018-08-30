#include "rpos/BlockMessage.h"

#include <crypto/PrivateKey.h>
#include <rpos/Node.h>
#include <rpos/Seed.h>
#include <rpos/VoterSystem.h>
#include <rpos/Vrf.h>

namespace ultrainio {
    BlockMessage::BlockMessage(uint32_t blockNum) : m_blockNum(blockNum) {}

    PhaseMessagePtr BlockMessage::initIfNeed(ConsensusPhase phase, int baxCount) {
        int key = phase + baxCount;
        auto itor = m_phaseMessageMap.find(key);
        if (itor == m_phaseMessageMap.end()) {
            PhaseMessagePtr phaseMessagePtr = std::make_shared<PhaseMessage>();
            phaseMessagePtr->m_phase = phase;
            phaseMessagePtr->m_baxCount = baxCount;
            m_phaseMessageMap.insert(make_pair(key, phaseMessagePtr));
            return phaseMessagePtr;
        }
        return itor->second;
    }

    void BlockMessage::insert(const EchoMsg& echoMsg) {
        ConsensusPhase phase = echoMsg.phase;
        uint32_t baxCount = echoMsg.baxCount;
        PhaseMessagePtr phaseMessagePtr = initIfNeed(phase, baxCount);
        ULTRAIN_ASSERT(phaseMessagePtr->m_phase == phase && phaseMessagePtr->m_baxCount == baxCount,
                chain::chain_exception, "phase or baxCount not equal");
        phaseMessagePtr->insert(echoMsg);
    }

    void BlockMessage::insert(const ProposeMsg& proposeMsg) {
        m_proposeMsgList.push_back(proposeMsg);
    }

    void BlockMessage::moveToNewStep(uint32_t blockNum, ConsensusPhase phase, int baxCount) {
        ULTRAIN_ASSERT(blockNum == m_blockNum, chain::chain_exception, "blockNum is equal");
        if (!m_proposerProof.isValid()) {
            ultrainio::chain::block_id_type blockId = UranusNode::getInstance()->getPreviousHash();
            std::string previousHash(blockId.data());
            // #### This line has to run first, all the following caculation depends on this line. ###
            m_voterSystem = VoterSystem::create(blockNum, nullptr);
            Seed proposerSeed(previousHash, blockNum, kPhaseBA0, 0);
            PrivateKey privateKey = m_voterSystem->getMyWorkingPrivateKey();
            m_proposerProof = Vrf::vrf(privateKey, proposerSeed, Vrf::kProposer);
            int stakes = m_voterSystem->getStakes(m_voterSystem->getMyWorkingAccount(), UranusNode::getInstance()->getNonProducingNode());
            double proposerRatio = m_voterSystem->getProposerRatio();
            m_voterCountAsProposer = m_voterSystem->count(m_proposerProof, stakes, proposerRatio);
            //ilog("blockNum = ${blockNum} voterCountAsProposer = ${voterCountAsProposer} proposerProof = ${proposerProof}",
                    //("blockNum", blockNum)("voterCountAsProposer", voterCountAsProposer)("proposerProof", std::string(proposerProof)));
        }
        PhaseMessagePtr phaseMessagePtr = initIfNeed(phase, baxCount);
        phaseMessagePtr->moveToNewStep(blockNum, phase, baxCount);
    }

    Proof BlockMessage::getVoterProof(ConsensusPhase phase, int baxCount) {
        PhaseMessagePtr phaseMessagePtr = initIfNeed(phase, baxCount);
        return phaseMessagePtr->m_proof;
    }

    int BlockMessage::getVoterVoterCount(ConsensusPhase phase, int baxCount) {
        PhaseMessagePtr phaseMessagePtr = initIfNeed(phase, baxCount);
        return phaseMessagePtr->m_voterCountAsVoter;
    }

    bool BlockMessage::newRound(ConsensusPhase phase, int baxCount) {
        if (kPhaseBA0 == phase && 0 == baxCount) {
            return true;
        }
        return false;
    }
}
