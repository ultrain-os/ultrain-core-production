#include "rpos/PhaseMessage.h"

#include <crypto/PrivateKey.h>
#include <rpos/MessageManager.h>
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
        std::shared_ptr<VoterSystem> voterSysPtr = MessageManager::getInstance()->getVoterSys(echoMsg.blockHeader.block_num());
        int stakes = voterSysPtr->getStakes(echoMsg.account, UranusNode::getInstance()->getNonProducingNode());
        double voterRatio = voterSysPtr->getVoterRatio();
        int voterCount = voterSysPtr->count(proof, stakes, voterRatio);
        if (itor == m_echoMsgSetMap.end()) {
            EchoMsgSet echoMsgSet;
            echoMsgSet.echoMsgV.push_back(echoMsg);
            echoMsgSet.accountPool.push_back(echoMsg.account);
            echoMsgSet.blockHeader = echoMsg.blockHeader;
            echoMsgSet.totalVoterCount = voterCount;
            m_echoMsgSetMap.insert(std::make_pair(blockId, echoMsgSet));
        } else {
            itor->second.echoMsgV.push_back(echoMsg);
            itor->second.accountPool.push_back(echoMsg.account);
            itor->second.totalVoterCount += voterCount;
        }
    }

    void PhaseMessage::moveToNewStep(uint32_t blockNum, ConsensusPhase phase, int baxCount) {
        ultrainio::chain::block_id_type blockId = UranusNode::getInstance()->getPreviousHash();
        std::string previousHash(blockId.data());
        std::shared_ptr<VoterSystem> voterSysPtr = MessageManager::getInstance()->getVoterSys(blockNum);
        ULTRAIN_ASSERT(voterSysPtr != nullptr, chain::chain_exception, "voterSystemPtr is nullptr");
        AccountName workingAccount = voterSysPtr->getMyWorkingAccount();
        Seed voterSeed(previousHash, blockNum, phase, baxCount);
        PrivateKey privateKey = voterSysPtr->getPrivateKey(workingAccount);
        if (!privateKey.isValid()) {
            return;
        }
        m_proof = Vrf::vrf(privateKey, voterSeed, Vrf::kVoter);
        int stakes = voterSysPtr->getStakes(workingAccount, UranusNode::getInstance()->getNonProducingNode());
        if (stakes == 0) {
            m_voterCountAsVoter = 0;
            return;
        }
        double voterRatio = voterSysPtr->getVoterRatio();
        m_voterCountAsVoter = voterSysPtr->count(m_proof, stakes, voterRatio);
    }
}