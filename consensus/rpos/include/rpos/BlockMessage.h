#pragma once

#include <map>
#include <memory>
#include <vector>

#include <core/Message.h>
#include <rpos/CommitteeInfo.h>
#include <rpos/PhaseMessage.h>
#include <rpos/Proof.h>

namespace ultrainio {

    class BlockMessage {
    public:
        static bool newRound(ConsensusPhase phase, int baxCount);
        void insert(const EchoMsg& echoMsg);
        void insert(const ProposeMsg& proposeMsg);
        void moveToNewStep(uint32_t blockNum, ConsensusPhase phase, int baxCount);
        Proof getVoterProof(ConsensusPhase phase, int baxCount);
        int getVoterVoterCount(ConsensusPhase phase, int baxCount);
    private:
        PhaseMessagePtr initIfNeed(ConsensusPhase phase, int baxCount);
        std::vector<ProposeMsg> m_proposeMsgList;
        std::map<int, PhaseMessagePtr> m_phaseMessageMap; // key = phase + bax_count
        uint32_t m_blockNum = 0;
        int m_voterCountAsProposer = 0;
        Proof m_proposerProof;
        std::shared_ptr<AggEchoMsg> m_myAggEchoMsgPtr;
        std::vector<AggEchoMsg> m_aggEchoMsgV;
        std::shared_ptr<CommitteeState> m_committeeStatePtr = nullptr;

        friend class MessageManager;
    };

    typedef std::shared_ptr<BlockMessage> BlockMessagePtr;
}