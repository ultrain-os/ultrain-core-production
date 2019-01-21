#include <rpos/MsgMgr.h>

#include <ultrainio/chain/exceptions.hpp>

#include <rpos/Config.h>
#include <rpos/Node.h>
#include <rpos/Seed.h>
#include <rpos/StakeVoteBase.h>
#include <rpos/Validator.h>
#include <rpos/Vrf.h>

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
        BlockMsgPoolPtr blockMsgPoolPtr = getBlockMsgPool(BlockHeader::num_from_id(aggEchoMsgPtr->commonEchoMsg.blockId));
        blockMsgPoolPtr->m_myAggEchoMsgPtr = aggEchoMsgPtr;
    }

    std::shared_ptr<AggEchoMsg> MsgMgr::getMyAggEchoMsg(uint32_t blockNum) {
        BlockMsgPoolPtr blockMsgPoolPtr = getBlockMsgPool(blockNum);
        if (!blockMsgPoolPtr) {
            return nullptr;
        }
        return blockMsgPoolPtr->m_myAggEchoMsgPtr;
    }

    void MsgMgr::moveToNewStep(uint32_t blockNum, ConsensusPhase phase, int baxCount) {
        ilog("moveToNewStep blockNum = ${blockNum}, phase = ${phase}, baxCount = ${baxCount}",
                ("blockNum", blockNum)("phase", static_cast<int>(phase))("baxCount", baxCount));
        BlockMsgPoolPtr blockMsgPoolPtr = nullptr;
        if (StakeVoteBase::newRound(phase, baxCount)) {
            clearSomeBlockMessage(blockNum);
        }
        std::shared_ptr<StakeVoteBase> stakeVotePtr = getVoterSys(blockNum);
        stakeVotePtr->moveToNewStep(blockNum, phase, baxCount);
    }

    BlockMsgPoolPtr MsgMgr::getBlockMsgPool(uint32_t blockNum) {
        ULTRAIN_ASSERT(blockNum > 1, chain::chain_exception, "blockNum should > 1");
        auto itor = m_blockMsgPoolMap.find(blockNum);
        if (itor == m_blockMsgPoolMap.end()) {
            BlockMsgPoolPtr blockMsgPoolPtr = std::make_shared<BlockMsgPool>(blockNum);
            m_blockMsgPoolMap.insert(make_pair(blockNum, blockMsgPoolPtr));
            return blockMsgPoolPtr;
        }
        return itor->second;
    }

    int MsgMgr::handleMessage(const AggEchoMsg& aggEchoMsg) {
        if (!UranusNode::getInstance()->isReady()) {
            ilog("Node is not ready, but we need transfer the msg.");
            return kSuccess;
        }

        uint32_t blockNum = BlockHeader::num_from_id(aggEchoMsg.commonEchoMsg.blockId);
        uint32_t myBlockNum = UranusNode::getInstance()->getBlockNum();
        if (myBlockNum - Config::MAX_LATER_NUMBER > blockNum) {
            return kObsolete;
        }
        if (StakeVoteBase::getMyAccount() == aggEchoMsg.account) {
            ilog("loopback AggEchoMsg");
            return kDuplicate;
        }
        BlockMsgPoolPtr blockMsgPoolPtr = getBlockMsgPool(blockNum);
        for (auto itor = blockMsgPoolPtr->m_aggEchoMsgV.begin(); itor != blockMsgPoolPtr->m_aggEchoMsgV.end(); itor++) {
            if (itor->account == aggEchoMsg.account) {
                ilog("duplicate AggEchoMsg");
                return kDuplicate;
            }
        }
        blockMsgPoolPtr->m_aggEchoMsgV.push_back(aggEchoMsg);
        if (blockNum == myBlockNum) {
            ilog("handle each echo!");
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
#ifdef CONSENSUS_VRF
            Proof proof(aggEchoMsg.myProposerProof);
            BlockIdType blockId = UranusNode::getInstance()->getPreviousHash();
            std::string previousHash(blockId.data());
            Seed seed(previousHash, blockNum, kPhaseBA0, 0);
            if (!Vrf::verify(publicKey, proof, seed, Vrf::kProposer)) {
                elog("verify AggEchoMsg proof error. account : ${account}", ("account", std::string(aggEchoMsg.account)));
                return kFaultProposer;
            }

            if (!stakeVotePtr->isProposer(aggEchoMsg.account, proof, UranusNode::getInstance()->getNonProducingNode())) {
                elog("send AggEchoMsg by non Proposer. account : ${account}", ("account", std::string(aggEchoMsg.account)));
                return kFaultProposer;
            }
#else
            if (!stakeVotePtr->isProposer(aggEchoMsg.account, UranusNode::getInstance()->getNonProducingNode())) {
                elog("is not proposer to send AggEchoMsg. account : ${account}", ("account", std::string(aggEchoMsg.account)));
                return kFaultProposer;
            }
#endif
            for (size_t i = 0; i < aggEchoMsg.accountPool.size(); i++) {
                EchoMsg echoMsg;
                echoMsg.blockId = aggEchoMsg.commonEchoMsg.blockId;
#ifdef CONSENSUS_VRF
                echoMsg.proposerPriority = aggEchoMsg.proposerPriority;
                echoMsg.proof = aggEchoMsg.proofPool[i];
#else
                echoMsg.proposer = aggEchoMsg.commonEchoMsg.proposer;
#endif
                echoMsg.account = aggEchoMsg.accountPool[i];
                echoMsg.signature = aggEchoMsg.sigPool[i];
                echoMsg.timestamp = aggEchoMsg.timePool[i];
                echoMsg.phase = aggEchoMsg.commonEchoMsg.phase;
                echoMsg.baxCount = aggEchoMsg.commonEchoMsg.baxCount;
                UranusNode::getInstance()->handleMessage(echoMsg);
            }
        }
        return kSuccess;
    }

#ifdef CONSENSUS_VRF
    Proof MsgMgr::getVoterProof(uint32_t blockNum, ConsensusPhase phase, int baxCount) {
        std::shared_ptr<StakeVoteBase> stakeVotePtr = getVoterSys(blockNum);
        ULTRAIN_ASSERT(stakeVotePtr != nullptr, chain::chain_exception, "not init StakeVote");
        return stakeVotePtr->getVoterProof(blockNum, phase, baxCount);
    }

    Proof MsgMgr::getProposerProof(uint32_t blockNum) {
        std::shared_ptr<StakeVoteBase> stakeVotePtr = getVoterSys(blockNum);
        ULTRAIN_ASSERT(stakeVotePtr != nullptr, chain::chain_exception, "not init StakeVote");
        return stakeVotePtr->getProposerProof(blockNum);
    }
#endif

    bool MsgMgr::isVoter(uint32_t blockNum, ConsensusPhase phase, int baxCount) {
        std::shared_ptr<StakeVoteBase> stakeVotePtr = getVoterSys(blockNum);
        ULTRAIN_ASSERT(stakeVotePtr != nullptr, chain::chain_exception, "not init StakeVote");
#ifdef CONSENSUS_VRF
        return stakeVotePtr->isVoter(StakeVoteBase::getMyAccount(), stakeVotePtr->getVoterProof(blockNum, phase, baxCount), UranusNode::getInstance()->getNonProducingNode());
#else
        return stakeVotePtr->isVoter(StakeVoteBase::getMyAccount(), phase, baxCount, UranusNode::getInstance()->getNonProducingNode());
#endif
    }

    bool MsgMgr::isProposer(uint32_t blockNum) {
        std::shared_ptr<StakeVoteBase> stakeVotePtr = getVoterSys(blockNum);
        ULTRAIN_ASSERT(stakeVotePtr != nullptr, chain::chain_exception, "not init StakeVote");
#ifdef CONSENSUS_VRF
        return stakeVotePtr->isProposer(StakeVoteBase::getMyAccount(), stakeVotePtr->getProposerProof(blockNum), UranusNode::getInstance()->getNonProducingNode());
#else
        return stakeVotePtr->isProposer(StakeVoteBase::getMyAccount(), UranusNode::getInstance()->getNonProducingNode());
#endif
    }

    void MsgMgr::clearSomeBlockMessage(uint32_t blockNum) {
        for (auto itor = m_blockMsgPoolMap.begin(); itor != m_blockMsgPoolMap.end();) {
            if (blockNum - Config::MAX_LATER_NUMBER > itor->first) {
                ilog("clear block msg for blockNum = ${blockNum}", ("blockNum", itor->first));
                m_blockMsgPoolMap.erase(itor++);
            } else {
                itor++;
            }
        }
    }

    std::shared_ptr<StakeVoteBase> MsgMgr::getVoterSys(uint32_t blockNum) {
        BlockMsgPoolPtr blockMsgPoolPtr = getBlockMsgPool(blockNum);
        return blockMsgPoolPtr->getVoterSys();
    }
}
