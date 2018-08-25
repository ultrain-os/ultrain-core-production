#pragma once

#include <map>
#include <memory>
#include <vector>

#include <core/Message.h>
#include <rpos/PhaseMessage.h>
#include <rpos/Proof.h>
#include <rpos/StakeAccountInfo.h>

namespace ultrainio {
    struct StakeAccountInfo;

    class BlockMessage {
    public:
        static bool newRound(ConsensusPhase phase, int baxCount);
        void insert(const EchoMsg& echoMsg);
        void insert(const ProposeMsg& proposeMsg);
        void moveToNewStep(uint32_t blockNum, ConsensusPhase phase, int baxCount);
        Proof getVoterProof(ConsensusPhase phase, int baxCount);
        int getVoterVoterCount(ConsensusPhase phase, int baxCount);

        uint32_t blockNum = 0;
        int voterCountAsProposer = 0;
        Proof proposerProof;
        std::shared_ptr<AggEchoMsg> myAggEchoMsgPtr;
        std::vector<AggEchoMsg> aggEchoMsgV;
        std::vector<StakeAccountInfo> stakeAccountInfoV;
    private:
        PhaseMessagePtr initIfNeed(ConsensusPhase phase, int baxCount);
        std::vector<ProposeMsg> proposeMsgList;
        std::map<int, PhaseMessagePtr> phaseMessageMap; // key = phase + bax_count
    };

    typedef std::shared_ptr<BlockMessage> BlockMessagePtr;
}