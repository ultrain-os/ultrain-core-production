#include <rpos/MessageManager.h>

#include <ultrainio/chain/exceptions.hpp>

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
        ULTRAIN_ASSERT(aggEchoMsgPtr, chain::chain_exception, "agg echo msg pointer is null");
        BlockMessagePtr blockMessagePtr = initIfNeed(aggEchoMsgPtr->blockHeader.block_num());
        blockMessagePtr->m_myAggEchoMsgPtr = aggEchoMsgPtr;
    }

    std::shared_ptr<AggEchoMsg> MessageManager::getMyAggEchoMsg(uint32_t blockNum) {
        BlockMessagePtr blockMessagePtr = initIfNeed(blockNum);
        if (!blockMessagePtr) {
            return nullptr;
        }
        return blockMessagePtr->m_myAggEchoMsgPtr;
    }

    void MessageManager::moveToNewStep(uint32_t blockNum, ConsensusPhase phase, int baxCount) {
        ilog("moveToNewStep blockNum = ${blockNum}, phase = ${phase}, baxCount = ${baxCount}",
                ("blockNum", blockNum)("phase", static_cast<int>(phase))("baxCount", baxCount));
        BlockMessagePtr blockMessagePtr = nullptr;
        if (BlockMessage::newRound(phase, baxCount)) {
            clearSomeBlockMessage(blockNum);
        }
        blockMessagePtr = initIfNeed(blockNum);
        blockMessagePtr->moveToNewStep(blockNum, phase, baxCount);
    }

    BlockMessagePtr MessageManager::initIfNeed(uint32_t blockNum) {
        ULTRAIN_ASSERT(blockNum > 1, chain::chain_exception, "blockNum should > 1");
        auto itor = blockMessageMap.find(blockNum);
        if (itor == blockMessageMap.end()) {
            BlockMessagePtr blockMessagePtr = std::make_shared<BlockMessage>();
            blockMessagePtr->m_blockNum = blockNum;
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
        if (blockMessagePtr->m_myAggEchoMsgPtr && blockMessagePtr->m_myAggEchoMsgPtr->pk == aggEchoMsg.pk) {
            ilog("loopback AggEchoMsg");
            return kDuplicate;
        }
        for (auto itor = blockMessagePtr->m_aggEchoMsgV.begin(); itor != blockMessagePtr->m_aggEchoMsgV.end(); itor++) {
            if (itor->pk == aggEchoMsg.pk) {
                ilog("duplicate AggEchoMsg");
                return kDuplicate;
            }
        }
        blockMessagePtr->m_aggEchoMsgV.push_back(aggEchoMsg);
        for (int i = 0; i < aggEchoMsg.pkPool.size(); i++) {
            EchoMsg echoMsg;
            echoMsg.blockHeader = aggEchoMsg.blockHeader;
            echoMsg.pk = aggEchoMsg.pkPool[i];
            echoMsg.proof = aggEchoMsg.proofPool[i];
            echoMsg.phase = aggEchoMsg.phase;
            echoMsg.baxCount = aggEchoMsg.baxCount;

            //TODO(qinxiaofen) proof check
//            PublicKey publicKey(echoMsg.pk);
//            Proof proposerProof(echoMsg.proof);
//            ultrainio::chain::block_id_type blockId = UranusNode::getInstance()->getPreviousHash();
//            std::string previousHash(blockId.data());
//            Seed seed(previousHash, echoMsg.blockHeader.block_num(), echoMsg.phase, echoMsg.baxCount);
//            if (!Vrf::verify(publicKey, proposerProof, seed, Vrf::kProposer)) {
//                elog("proof verify error. pk : ${pk}", ("pk", propose.block.proposerProof));
//                return false;
//            }
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
        return blockMessagePtr->m_proposerProof;
    }

    int MessageManager::getProposerVoterCount(uint32_t blockNum) {
        BlockMessagePtr blockMessagePtr = initIfNeed(blockNum);
        return blockMessagePtr->m_voterCountAsProposer;
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

    std::shared_ptr<std::vector<CommitteeInfo>> MessageManager::getCommitteeInfoVPtr(uint32_t blockNum) {
        BlockMessagePtr blockMessagePtr = initIfNeed(blockNum);
        return blockMessagePtr->m_committeeInfoVPtr;
    }
}
