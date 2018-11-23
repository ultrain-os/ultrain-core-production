#include <rpos/MsgMgr.h>

#include <ultrainio/chain/exceptions.hpp>

#include <rpos/Config.h>
#include <rpos/Node.h>
#include <rpos/StakeVoteBase.h>
#include <rpos/Validator.h>

namespace ultrainio {
    using namespace std;

    // class MessageManager
    shared_ptr<MsgMgr> MsgMgr::s_self = nullptr;

    std::shared_ptr<MsgMgr> MsgMgr::getInstance() {
        if (!s_self) {
            s_self = std::shared_ptr<MsgMgr>(new MsgMgr());
        }
        return s_self;
    }

    void MsgMgr::insert(std::shared_ptr<AggEchoMsg> aggEchoMsgPtr) {
        ULTRAIN_ASSERT(aggEchoMsgPtr, chain::chain_exception, "agg echo msg pointer is null");
        BlockMessagePtr blockMessagePtr = initIfNeed(BlockHeader::num_from_id(aggEchoMsgPtr->blockId));
        blockMessagePtr->m_myAggEchoMsgPtr = aggEchoMsgPtr;
    }

    std::shared_ptr<AggEchoMsg> MsgMgr::getMyAggEchoMsg(uint32_t blockNum) {
        BlockMessagePtr blockMessagePtr = initIfNeed(blockNum);
        if (!blockMessagePtr) {
            return nullptr;
        }
        return blockMessagePtr->m_myAggEchoMsgPtr;
    }

    void MsgMgr::moveToNewStep(uint32_t blockNum, ConsensusPhase phase, int baxCount) {
        ilog("moveToNewStep blockNum = ${blockNum}, phase = ${phase}, baxCount = ${baxCount}",
                ("blockNum", blockNum)("phase", static_cast<int>(phase))("baxCount", baxCount));
        BlockMessagePtr blockMessagePtr = nullptr;
        if (BlockMsg::newRound(phase, baxCount)) {
            clearSomeBlockMessage(blockNum);
        }
        blockMessagePtr = initIfNeed(blockNum);
        blockMessagePtr->moveToNewStep(blockNum, phase, baxCount);
    }

    BlockMessagePtr MsgMgr::initIfNeed(uint32_t blockNum) {
        ULTRAIN_ASSERT(blockNum > 1, chain::chain_exception, "blockNum should > 1");
        auto itor = blockMessageMap.find(blockNum);
        if (itor == blockMessageMap.end()) {
            BlockMessagePtr blockMessagePtr = std::make_shared<BlockMsg>(blockNum);
            blockMessageMap.insert(make_pair(blockNum, blockMessagePtr));
            return blockMessagePtr;
        }
        return itor->second;
    }

    int MsgMgr::handleMessage(const AggEchoMsg& aggEchoMsg) {
        if (!UranusNode::getInstance()->isReady()) {
            ilog("Node is not ready, but we need transfer the msg.");
            return kSuccess;
        }

        uint32_t blockNum = BlockHeader::num_from_id(aggEchoMsg.blockId);
        uint32_t myBlockNum = UranusNode::getInstance()->getBlockNum();
        if (myBlockNum - Config::MAX_LATER_NUMBER > blockNum) {
            return kObsolete;
        }
        if (StakeVoteBase::getMyAccount() == aggEchoMsg.account) {
            ilog("loopback AggEchoMsg");
            return kDuplicate;
        }
        BlockMessagePtr blockMessagePtr = initIfNeed(blockNum);
        for (auto itor = blockMessagePtr->m_aggEchoMsgV.begin(); itor != blockMessagePtr->m_aggEchoMsgV.end(); itor++) {
            if (itor->account == aggEchoMsg.account) {
                ilog("duplicate AggEchoMsg");
                return kDuplicate;
            }
        }
        blockMessagePtr->m_aggEchoMsgV.push_back(aggEchoMsg);
        if (blockNum == myBlockNum) {
            std::shared_ptr<StakeVoteBase> stakeVotePtr = getVoterSys(blockNum);
            PublicKey publicKey = stakeVotePtr->getPublicKey(aggEchoMsg.account);
            ULTRAIN_ASSERT(publicKey.isValid(), chain::chain_exception, "public key is not valid");
            if (!publicKey.isValid()) {
                return kAccountError;
            }
            if (!Validator::verify<UnsignedAggEchoMsg>(Signature(aggEchoMsg.signature), aggEchoMsg, publicKey)) {
                elog("verify AggEchoMsg error. account : ${account}", ("account", std::string(aggEchoMsg.account)));
                return kSignatureError;
            }
            if (!stakeVotePtr->isProposer(aggEchoMsg.account, UranusNode::getInstance()->getNonProducingNode())) {
                elog("is not proposer to send AggEchoMsg. account : ${account}", ("account", std::string(aggEchoMsg.account)));
                return kFaultProposer;
            }
            for (size_t i = 0; i < aggEchoMsg.accountPool.size(); i++) {
                EchoMsg echoMsg;
                echoMsg.blockId = aggEchoMsg.blockId;
                echoMsg.account = aggEchoMsg.accountPool[i];
                echoMsg.signature = aggEchoMsg.sigPool[i];
                echoMsg.timestamp = aggEchoMsg.timePool[i];
                echoMsg.phase = aggEchoMsg.phase;
                echoMsg.baxCount = aggEchoMsg.baxCount;
                UranusNode::getInstance()->handleMessage(echoMsg);
            }
        }
        return kSuccess;
    }

    bool MsgMgr::isVoter(uint32_t blockNum, ConsensusPhase phase, int baxCount) {
        BlockMessagePtr blockMessagePtr = initIfNeed(blockNum);
        return blockMessagePtr->isVoter(phase, baxCount);
    }

    bool MsgMgr::isProposer(uint32_t blockNum) {
        BlockMessagePtr blockMessagePtr = initIfNeed(blockNum);
        return blockMessagePtr->isProposer();
    }

    void MsgMgr::clearSomeBlockMessage(uint32_t blockNum) {
        for (auto itor = blockMessageMap.begin(); itor != blockMessageMap.end();) {
            if (blockNum - Config::MAX_LATER_NUMBER > itor->first) {
                ilog("clear block msg for blockNum = ${blockNum}", ("blockNum", itor->first));
                blockMessageMap.erase(itor++);
            } else {
                itor++;
            }
        }
    }

    std::shared_ptr<StakeVoteBase> MsgMgr::getVoterSys(uint32_t blockNum) {
        BlockMessagePtr blockMessagePtr = initIfNeed(blockNum);
        return blockMessagePtr->getVoterSys();
    }
}
