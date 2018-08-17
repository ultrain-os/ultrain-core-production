#include <uranus/MessageManager.h>

#include <log/Log.h>
#include <uranus/Node.h>

namespace ultrainio {
    using namespace std;

    static bool newRound(ConsensusPhase phase, int baxCount);

    // class MessageManager
    shared_ptr<MessageManager> MessageManager::s_self = nullptr;

    std::shared_ptr<MessageManager> MessageManager::getInstance() {
        if (!s_self) {
            s_self = std::shared_ptr<MessageManager>(new MessageManager());
        }
        return s_self;
    }

    void MessageManager::insert(const EchoMsg& echoMsg) {
        uint32_t blockNum = echoMsg.blockHeader.block_num();
        BlockMessagePtr blockMessagePtr = initIfNeed(blockNum);
        assert(blockMessagePtr->blockNum == blockNum);
        blockMessagePtr->insert(echoMsg);
    }

    void MessageManager::insert(const ProposeMsg& proposeMsg) {
        uint32_t blockNum = proposeMsg.block.block_num();
        BlockMessagePtr blockMessagePtr = initIfNeed(blockNum);
        assert(blockMessagePtr->blockNum == blockNum);
        blockMessagePtr->insert(proposeMsg);
    }

    void MessageManager::moveToNewStep(uint32_t blockNum, ConsensusPhase phase, int baxCount) {
        ilog("moveToNewStep blockNum = ${blockNum}, phase = ${phase}, baxCount = ${baxCount}",
                ("blockNum", blockNum)("phase", static_cast<int>(phase))("baxCount", baxCount));
        if (newRound(phase, baxCount)) {
            static int cycle = 0;
            if (cycle == 10) { //
                cycle = 0;
                clearSomeBlockMessage(blockNum);
            }
            cycle++;
        }
        BlockMessagePtr blockMessagePtr = initIfNeed(blockNum);
        blockMessagePtr->moveToNewStep(blockNum, phase, baxCount);
    }

    BlockMessagePtr MessageManager::initIfNeed(uint32_t blockNum) {
        auto itor = blockMessageMap.find(blockNum);
        if (itor == blockMessageMap.end()) {
            BlockMessagePtr blockMessagePtr = std::make_shared<BlockMessage>();
            blockMessagePtr->blockNum = blockNum;
            blockMessageMap.insert(make_pair(blockNum, blockMessagePtr));
            return blockMessagePtr;
        }
        return itor->second;
    }

    const uint8_t* MessageManager::getVoterProof(uint32_t blockNum, ConsensusPhase phase, int baxCount) {
        BlockMessagePtr blockMessagePtr = initIfNeed(blockNum);
        return blockMessagePtr->getVoterProof(phase, baxCount);
    }

    const uint8_t* MessageManager::getProposerProof(uint32_t blockNum) {
        BlockMessagePtr blockMessagePtr = initIfNeed(blockNum);
        return blockMessagePtr->proposerProof;
    }

    int MessageManager::getProposerVoterCount(uint32_t blockNum) {
        BlockMessagePtr blockMessagePtr = initIfNeed(blockNum);
        return blockMessagePtr->voterCountAsProposer;
    }

    int MessageManager::getVoterVoterCount(uint32_t blockNum, ConsensusPhase phase, int baxCount) {
        BlockMessagePtr blockMessagePtr = initIfNeed(blockNum);
        return blockMessagePtr->getVoterVoterCount(phase, baxCount);
    }

    bool MessageManager::isVoter(uint32_t blockNum, ConsensusPhase phase, int baxCount) {
        return getVoterVoterCount(blockNum, phase, baxCount) > 0;
    }

    bool MessageManager::isProposer(uint32_t blockNum) {
        return getProposerVoterCount(blockNum);
    }

    void MessageManager::clearSomeBlockMessage(uint32_t blockNum) {
        for (auto itor = blockMessageMap.begin(); itor != blockMessageMap.end();) {
            ilog("has blockNum = ${blockNum}", ("blockNum", blockNum));
            if (blockNum - itor->first > 3) { // keep 3 block behind this one
                blockMessageMap.erase(itor++);
            } else {
                itor++;
            }
        }
    }

    // class BlockMessage
    PhaseMessagePtr BlockMessage::initIfNeed(ConsensusPhase phase, int baxCount) {
        int key = phase + baxCount;
        auto itor = phaseMessageMap.find(key);
        if (itor == phaseMessageMap.end()) {
            PhaseMessagePtr phaseMessagePtr = std::make_shared<PhaseMessage>();
            phaseMessagePtr->phase = phase;
            phaseMessagePtr->baxCount = baxCount;
            phaseMessageMap.insert(make_pair(key, phaseMessagePtr));
            return phaseMessagePtr;
        }
        return itor->second;
    }

    void BlockMessage::insert(const EchoMsg& echoMsg) {
        ConsensusPhase phase = echoMsg.phase;
        uint32_t baxCount = echoMsg.baxCount;
        PhaseMessagePtr phaseMessagePtr = initIfNeed(phase, baxCount);
        assert(phaseMessagePtr->phase == phase && phaseMessagePtr->baxCount == baxCount);
        phaseMessagePtr->insert(echoMsg);
    }

    void BlockMessage::insert(const ProposeMsg& proposeMsg) {
        proposeMsgList.push_back(proposeMsg);
    }

    void BlockMessage::moveToNewStep(uint32_t blockNum, ConsensusPhase phase, int baxCount) {
        if (newRound(phase, baxCount)) {
            ultrainio::chain::block_id_type blockId = UranusNode::getInstance()->getPreviousHash();
            std::string previousHash(blockId.data());
            std::string proposerSeed = previousHash + std::to_string(blockNum) + std::to_string(phase) + std::to_string(baxCount) + std::to_string(kProposer);
            int stakes = UranusNode::getInstance()->getStakes(std::string((char*)UranusNode::URANUS_PUBLIC_KEY));
            VoterSystem voterSystem;
            voterCountAsProposer = voterSystem.vote(proposerSeed, UranusNode::URANUS_PRIVATE_KEY, stakes, VoterSystem::PROPOSER_RATIO, proposerProof);
            //ilog("blockNum = ${blockNum} voterCountAsProposer = ${voterCountAsProposer} proposerProof = ${proposerProof}",
            //        ("blockNum", blockNum)("voterCountAsProposer", voterCountAsProposer)("proposerProof", UltrainLog::convert2Hex(std::string((char*)proposerProof, VRF_PROOF_LEN))));
        }
        PhaseMessagePtr phaseMessagePtr = initIfNeed(phase, baxCount);
        phaseMessagePtr->moveToNewStep(blockNum, phase, baxCount);
    }

    const uint8_t* BlockMessage::getVoterProof(ConsensusPhase phase, int baxCount) {
        PhaseMessagePtr phaseMessagePtr = initIfNeed(phase, baxCount);
        return phaseMessagePtr->proof;
    }

    int BlockMessage::getVoterVoterCount(ConsensusPhase phase, int baxCount) {
        PhaseMessagePtr phaseMessagePtr = initIfNeed(phase, baxCount);
        return phaseMessagePtr->voterCountAsVoter;
    }

    // class PhaseMessage
    void PhaseMessage::insert(const EchoMsg& echoMsg) {
        chain::block_id_type blockId = echoMsg.blockHeader.id();
        auto itor = echoMsgSetMap.find(blockId);
        VoterSystem voter;
        int stakes = UranusNode::getInstance()->getStakes(echoMsg.pk);
        int voterCount = voter.vote((uint8_t*)echoMsg.proof.data(), stakes, VoterSystem::VOTER_RATIO);
        if (itor == echoMsgSetMap.end()) {
            EchoMsgSet echoMsgSet;
            echoMsgSet.echoMsgV.push_back(echoMsg);
            echoMsgSet.pkPool.push_back(echoMsg.pk);
            echoMsgSet.blockHeader = echoMsg.blockHeader;
            echoMsgSet.totalVoterCount = voterCount;
            echoMsgSetMap.insert(make_pair(blockId, echoMsgSet));
        } else {
            itor->second.echoMsgV.push_back(echoMsg);
            itor->second.pkPool.push_back(echoMsg.pk);
            itor->second.totalVoterCount += voterCount;
        }
    }

    void PhaseMessage::moveToNewStep(uint32_t blockNum, ConsensusPhase phase, int baxCount) {
        ultrainio::chain::block_id_type blockId = UranusNode::getInstance()->getPreviousHash();
        std::string previousHash(blockId.data());
        std::string voterSeed = previousHash + std::to_string(blockNum) + std::to_string(static_cast<int>(phase))
                + std::to_string(baxCount) + std::to_string(kVoter);
        int stakes = UranusNode::getInstance()->getStakes(std::string((char*)UranusNode::URANUS_PUBLIC_KEY));
        VoterSystem voterSystem;
        voterCountAsVoter = voterSystem.vote(voterSeed, UranusNode::URANUS_PRIVATE_KEY, stakes, VoterSystem::VOTER_RATIO, proof);
        //ilog("blockNum = ${blockNum} phase = ${phase} baxCount = ${baxCount} voterCountAsVoter = ${voterCountAsVoter} proof = ${proof}",
        //        ("blockNum", blockNum)("phase", static_cast<int>(phase))("baxCount", baxCount)("voterCountAsVoter", voterCountAsVoter)("proof", UltrainLog::convert2Hex(std::string((char*)proof, VRF_PROOF_LEN))));
    }

    bool newRound(ConsensusPhase phase, int baxCount) {
        if (kPhaseBA0 == phase && 0 == baxCount) {
            return true;
        }
        return false;
    }
}
