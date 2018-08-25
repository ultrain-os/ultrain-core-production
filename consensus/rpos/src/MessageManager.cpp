#include <rpos/MessageManager.h>

#include <log/Log.h>
#include <rpos/Node.h>
#include <rpos/Proof.h>
#include <rpos/Seed.h>
#include <rpos/Validator.h>
#include <rpos/Vrf.h>

namespace ultrainio {
    using namespace std;

    // class MessageManager
    shared_ptr<MessageManager> MessageManager::s_self = nullptr;

    std::shared_ptr<MessageManager> MessageManager::getInstance() {
        if (!s_self) {
            s_self = std::shared_ptr<MessageManager>(new MessageManager());
        }
        return s_self;
    }

//    void MessageManager::insert(const EchoMsg& echoMsg) {
//        uint32_t blockNum = echoMsg.blockHeader.block_num();
//        BlockMessagePtr blockMessagePtr = initIfNeed(blockNum);
//        assert(blockMessagePtr->blockNum == blockNum);
//        blockMessagePtr->insert(echoMsg);
//    }
//
//    void MessageManager::insert(const ProposeMsg& proposeMsg) {
//        uint32_t blockNum = proposeMsg.block.block_num();
//        BlockMessagePtr blockMessagePtr = initIfNeed(blockNum);
//        assert(blockMessagePtr->blockNum == blockNum);
//        blockMessagePtr->insert(proposeMsg);
//    }

    void MessageManager::insert(std::shared_ptr<AggEchoMsg> aggEchoMsgPtr) {
        BlockMessagePtr blockMessagePtr = initIfNeed(aggEchoMsgPtr->blockHeader.block_num());
        blockMessagePtr->myAggEchoMsgPtr = aggEchoMsgPtr;
    }

    std::shared_ptr<AggEchoMsg> MessageManager::getMyAggEchoMsg(uint32_t blockNum) {
        BlockMessagePtr blockMessagePtr = initIfNeed(blockNum);
        if (!blockMessagePtr) {
            return nullptr;
        }
        return blockMessagePtr->myAggEchoMsgPtr;
    }

    void MessageManager::moveToNewStep(uint32_t blockNum, ConsensusPhase phase, int baxCount) {
        ilog("moveToNewStep blockNum = ${blockNum}, phase = ${phase}, baxCount = ${baxCount}",
                ("blockNum", blockNum)("phase", static_cast<int>(phase))("baxCount", baxCount));
        if (BlockMessage::newRound(phase, baxCount)) {
            clearSomeBlockMessage(blockNum);
            // TODO(init StakeAccountInfo
        }
        BlockMessagePtr blockMessagePtr = initIfNeed(blockNum);
        blockMessagePtr->moveToNewStep(blockNum, phase, baxCount);
    }

    BlockMessagePtr MessageManager::initIfNeed(uint32_t blockNum) {
        if (blockNum < 1) {
            return nullptr;
        }
        auto itor = blockMessageMap.find(blockNum);
        if (itor == blockMessageMap.end()) {
            BlockMessagePtr blockMessagePtr = std::make_shared<BlockMessage>();
            blockMessagePtr->blockNum = blockNum;
            blockMessageMap.insert(make_pair(blockNum, blockMessagePtr));
            return blockMessagePtr;
        }
        return itor->second;
    }

    int MessageManager::handleMessage(const AggEchoMsg& aggEchoMsg) {
        if (!Validator::verify<UnsignedAggEchoMsg>(Signature(aggEchoMsg.signature), aggEchoMsg, PublicKey(aggEchoMsg.pk))) {
            elog("verify AggEchoMsg error. pk : ${pk}", ("pk", aggEchoMsg.pk));
            return kSignatureError;
        }
        uint32_t blockNum = UranusNode::getInstance()->getBlockNum();
        if (blockNum - 1 > aggEchoMsg.blockHeader.block_num()) {
            return kObsolete;
        }
        BlockMessagePtr blockMessagePtr = initIfNeed(aggEchoMsg.blockHeader.block_num());
        if (blockMessagePtr->myAggEchoMsgPtr && blockMessagePtr->myAggEchoMsgPtr->pk == aggEchoMsg.pk) {
            ilog("loopback AggEchoMsg");
            return kDuplicate;
        }
        for (auto itor = blockMessagePtr->aggEchoMsgV.begin(); itor != blockMessagePtr->aggEchoMsgV.end(); itor++) {
            if (itor->pk == aggEchoMsg.pk) {
                ilog("duplicate AggEchoMsg");
                return kDuplicate;
            }
        }
        blockMessagePtr->aggEchoMsgV.push_back(aggEchoMsg);
        for (int i = 0; i < aggEchoMsg.pkPool.size(); i++) {
            EchoMsg echoMsg;
            echoMsg.blockHeader = aggEchoMsg.blockHeader;
            echoMsg.pk = aggEchoMsg.pkPool[i];
            echoMsg.proof = aggEchoMsg.proofPool[i];
            echoMsg.phase = aggEchoMsg.phase;
            echoMsg.baxCount = aggEchoMsg.baxCount;
            UranusNode::getInstance()->handleMessage(echoMsg);
        }
        return kSuccess;
    }

    Proof MessageManager::getVoterProof(uint32_t blockNum, ConsensusPhase phase, int baxCount) {
        BlockMessagePtr blockMessagePtr = initIfNeed(blockNum);
        return blockMessagePtr->getVoterProof(phase, baxCount);
    }

    Proof MessageManager::getProposerProof(uint32_t blockNum) {
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
}
