#pragma once

#include <memory>
#include <vector>

#include <core/Message.h>
#include <core/Redefined.h>
#include <crypto/Ed25519.h>
#include <rpos/BlockMsg.h>
#include <rpos/Proof.h>
#include <rpos/StakeVote.h>

namespace ultrainio {
    enum MessageStatus {
        kSuccess = 0,
        kDuplicate,
        kSignatureError,
        kObsolete,
        kAccountError,
        kFaultProposer,
        kFaultVoter,
        kTheSameRoundButPhase,
        kLaterRound,
        kUnknownError
    };

    class MsgMgr {
    public:
        static std::shared_ptr<MsgMgr> getInstance();
        MsgMgr& operator = (const MsgMgr&) = delete;
        MsgMgr(const MsgMgr&) = delete;

        //void insert(const EchoMsg& echoMsg);

        //void insert(const ProposeMsg& proposeMsg);

        int handleMessage(const AggEchoMsg& aggEchoMsg);

        void insert(std::shared_ptr<AggEchoMsg> aggEchoMsgPtr);

        std::shared_ptr<AggEchoMsg> getMyAggEchoMsg(uint32_t blockNum);

        void moveToNewStep(uint32_t blockNum, ConsensusPhase phase, int baxCount);

        Proof getVoterProof(uint32_t blockNum, ConsensusPhase phase, int baxCount);

        Proof getProposerProof(uint32_t blockNum);

        bool isVoter(uint32_t blockNum, ConsensusPhase phase, int baxCount);

        bool isProposer(uint32_t blockNum);

        std::shared_ptr<StakeVote> getVoterSys(uint32_t blockNum);

    private:
        MsgMgr() = default;

        int getProposerVoterCount(uint32_t blockNum);

        int getVoterVoterCount(uint32_t blockNum, ConsensusPhase phase, int baxCount);

        BlockMessagePtr initIfNeed(uint32_t blockNum);

        void clearSomeBlockMessage(uint32_t blockNum);

        static std::shared_ptr<MsgMgr> s_self;
        std::map<int, BlockMessagePtr> blockMessageMap; // key - blockNum

        // Only for debug purpose.
        friend class Scheduler;
    };
}
