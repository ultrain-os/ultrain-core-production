#include "rpos/BlockMessage.h"

#include <crypto/PrivateKey.h>
#include <rpos/Node.h>
#include <rpos/Seed.h>
#include <rpos/VoterSystem.h>
#include <rpos/Vrf.h>

namespace ultrainio {
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
        assert(phaseMessagePtr->m_phase == phase && phaseMessagePtr->m_baxCount == baxCount);
        phaseMessagePtr->insert(echoMsg);
    }

    void BlockMessage::insert(const ProposeMsg& proposeMsg) {
        m_proposeMsgList.push_back(proposeMsg);
    }

    void BlockMessage::moveToNewStep(uint32_t blockNum, ConsensusPhase phase, int baxCount) {
        if (newRound(phase, baxCount)) {
            ultrainio::chain::block_id_type blockId = UranusNode::getInstance()->getPreviousHash();
            std::string previousHash(blockId.data());
            VoterSystem voterSystem;
            // #### This line has to run first, all the following caculation depends on this line. ###
            m_committeeInfoVPtr = voterSystem.getCommitteeInfoList();
            int stakes = voterSystem.getStakes(std::string(UranusNode::getInstance()->getSignaturePublic()));
            double proposerRatio = voterSystem.getProposerRatio();
            m_voterCountAsProposer = voterSystem.count(m_proposerProof, stakes, proposerRatio);
            Seed proposerSeed(previousHash, blockNum, phase, baxCount);
            PrivateKey privateKey = UranusNode::getInstance()->getSignaturePrivate();
            m_proposerProof = Vrf::vrf(privateKey, proposerSeed, Vrf::kProposer);

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
