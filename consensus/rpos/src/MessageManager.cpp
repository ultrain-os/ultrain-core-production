#include <rpos/MessageManager.h>

#include <ultrainio/chain/exceptions.hpp>

#include <rpos/Config.h>
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
        BlockMessagePtr blockMessagePtr = initIfNeed(BlockHeader::num_from_id(aggEchoMsgPtr->blockId));
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
            BlockMessagePtr blockMessagePtr = std::make_shared<BlockMessage>(blockNum);
            blockMessageMap.insert(make_pair(blockNum, blockMessagePtr));
            return blockMessagePtr;
        }
        return itor->second;
    }

    int MessageManager::handleMessage(const AggEchoMsg& aggEchoMsg) {
        if (!UranusNode::getInstance()->isReady()) {
            ilog("Node is not ready, but we need transfer the msg.");
            return kSuccess;
        }

        uint32_t blockNum = BlockHeader::num_from_id(aggEchoMsg.blockId);
        uint32_t myBlockNum = UranusNode::getInstance()->getBlockNum();
        if (myBlockNum - Config::MAX_LATER_NUMBER > blockNum) {
            return kObsolete;
        }
        if (blockNum > myBlockNum) {
            return kSuccess;
        }
        BlockMessagePtr blockMessagePtr = initIfNeed(blockNum);
        if (blockMessagePtr->m_myAggEchoMsgPtr && blockMessagePtr->m_myAggEchoMsgPtr->account == aggEchoMsg.account) {
            ilog("loopback AggEchoMsg");
            return kDuplicate;
        }
        for (auto itor = blockMessagePtr->m_aggEchoMsgV.begin(); itor != blockMessagePtr->m_aggEchoMsgV.end(); itor++) {
            if (itor->account == aggEchoMsg.account) {
                ilog("duplicate AggEchoMsg");
                return kDuplicate;
            }
        }
        blockMessagePtr->m_aggEchoMsgV.push_back(aggEchoMsg);
        if (blockNum == myBlockNum) {
            std::shared_ptr<VoterSystem> voterSysPtr = getVoterSys(blockNum);
            PublicKey publicKey = voterSysPtr->getPublicKey(aggEchoMsg.account);
            ULTRAIN_ASSERT(publicKey.isValid(), chain::chain_exception, "public key is not valid");
            if (!publicKey.isValid()) {
                return kAccountError;
            }
            if (!Validator::verify<UnsignedAggEchoMsg>(Signature(aggEchoMsg.signature), aggEchoMsg, publicKey)) {
                elog("verify AggEchoMsg error. account : ${account}", ("account", std::string(aggEchoMsg.account)));
                return kSignatureError;
            }
            Proof proof(aggEchoMsg.myProposerProof);
            ultrainio::chain::block_id_type blockId = UranusNode::getInstance()->getPreviousHash();
            std::string previousHash(blockId.data());
            Seed seed(previousHash, blockNum, kPhaseBA0, 0);
            if (!Vrf::verify(publicKey, proof, seed, Vrf::kProposer)) {
                elog("verify AggEchoMsg proof error. account : ${account}", ("account", std::string(aggEchoMsg.account)));
                return kFaultProposer;
            }

            int stakes = voterSysPtr->getStakes(aggEchoMsg.account, UranusNode::getInstance()->getNonProducingNode());
            double p = voterSysPtr->getProposerRatio();
            if (voterSysPtr->count(proof, stakes, p) <= 0) {
                elog("send AggEchoMsg by non Proposer. account : ${account}", ("account", std::string(aggEchoMsg.account)));
                return kFaultProposer;
            }
            for (size_t i = 0; i < aggEchoMsg.accountPool.size(); i++) {
                EchoMsg echoMsg;
                echoMsg.blockId = aggEchoMsg.blockId;
                echoMsg.proposerPriority = aggEchoMsg.proposerPriority;
                echoMsg.account = aggEchoMsg.accountPool[i];
                echoMsg.proof = aggEchoMsg.proofPool[i];
                echoMsg.signature = aggEchoMsg.sigPool[i];
                echoMsg.timestamp = aggEchoMsg.timePool[i];
                echoMsg.phase = aggEchoMsg.phase;
                echoMsg.baxCount = aggEchoMsg.baxCount;
                UranusNode::getInstance()->handleMessage(echoMsg);
            }
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
            if (blockNum - Config::MAX_LATER_NUMBER > itor->first) {
                ilog("clear block msg for blockNum = ${blockNum}", ("blockNum", itor->first));
                blockMessageMap.erase(itor++);
            } else {
                itor++;
            }
        }
    }

    std::shared_ptr<VoterSystem> MessageManager::getVoterSys(uint32_t blockNum) {
        BlockMessagePtr blockMessagePtr = initIfNeed(blockNum);
        return blockMessagePtr->getVoterSys();
    }
}
