#include "rpos/PhaseMessage.h"

#include <crypto/PrivateKey.h>
#include <rpos/Node.h>
#include <rpos/Proof.h>
#include <rpos/Seed.h>
#include <rpos/VoterSystem.h>
#include <rpos/Vrf.h>

namespace ultrainio {
    void PhaseMessage::insert(const EchoMsg& echoMsg) {
        chain::block_id_type blockId = echoMsg.blockHeader.id();
        auto itor = m_echoMsgSetMap.find(blockId);
        Proof proof(echoMsg.proof);
        VoterSystem voterSystem;
        int stakes = voterSystem.getStakes(echoMsg.pk);
        double voterRatio = voterSystem.getVoterRatio();
        int voterCount = voterSystem.count(proof, stakes, voterRatio);
        if (itor == m_echoMsgSetMap.end()) {
            EchoMsgSet echoMsgSet;
            echoMsgSet.echoMsgV.push_back(echoMsg);
            echoMsgSet.pkPool.push_back(echoMsg.pk);
            echoMsgSet.blockHeader = echoMsg.blockHeader;
            echoMsgSet.totalVoterCount = voterCount;
            m_echoMsgSetMap.insert(std::make_pair(blockId, echoMsgSet));
        } else {
            itor->second.echoMsgV.push_back(echoMsg);
            itor->second.pkPool.push_back(echoMsg.pk);
            itor->second.totalVoterCount += voterCount;
        }
    }

    void PhaseMessage::moveToNewStep(uint32_t blockNum, ConsensusPhase phase, int baxCount) {
        ultrainio::chain::block_id_type blockId = UranusNode::getInstance()->getPreviousHash();
        std::string previousHash(blockId.data());
        Seed voterSeed(previousHash, blockNum, phase, baxCount);
        PrivateKey privateKey = UranusNode::getInstance()->getPrivateKey();
        m_proof = Vrf::vrf(privateKey, voterSeed, Vrf::kVoter);
        VoterSystem voterSystem;
        int stakes = voterSystem.getStakes(std::string(UranusNode::getInstance()->getPublicKey()));
        double voterRatio = voterSystem.getVoterRatio();
        m_voterCountAsVoter = voterSystem.count(m_proof, stakes, voterRatio);
        //ilog("blockNum = ${blockNum} phase = ${phase} baxCount = ${baxCount} voterCountAsVoter = ${voterCountAsVoter} proof = ${proof}",
                //("blockNum", blockNum)("phase", static_cast<int>(phase))("baxCount", baxCount)("voterCountAsVoter", voterCountAsVoter)("proof", std::string(proof)));
    }
}