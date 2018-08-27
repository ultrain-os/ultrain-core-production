#pragma once

#include <memory>
#include <vector>

#include <core/Message.h>
#include <crypto/Ed25519.h>
#include <rpos/BlockMessage.h>
#include <rpos/Proof.h>
#include <rpos/VoterSystem.h>

namespace ultrainio {
    enum MessageStatus {
        kSuccess = 0,
        kDuplicate,
        kSignatureError,
        kObsolete,
        kFaultProposer,
        kFaultVoter,
        kTheSameRoundButPhase,
        kLaterRound
    };

    class MessageManager {
    public:
        static std::shared_ptr<MessageManager> getInstance();
        MessageManager& operator = (const MessageManager&) = delete;
        MessageManager(const MessageManager&) = delete;

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

        std::shared_ptr<std::vector<CommitteeInfo>> getCommitteeInfoVPtr(uint32_t blockNum);
    private:
        MessageManager() = default;

        int getProposerVoterCount(uint32_t blockNum);

        int getVoterVoterCount(uint32_t blockNum, ConsensusPhase phase, int baxCount);

        BlockMessagePtr initIfNeed(uint32_t blockNum);

        void clearSomeBlockMessage(uint32_t blockNum);

        static std::shared_ptr<MessageManager> s_self;
        std::map<int, BlockMessagePtr> blockMessageMap; // key - blockNum
    };
}