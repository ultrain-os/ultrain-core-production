#pragma once

#include <map>
#include <memory>
#include <vector>

#include <core/Message.h>
#include <rpos/CommitteeInfo.h>
#include <rpos/PhaseMsg.h>
#include <rpos/Proof.h>

namespace ultrainio {
    class StakeVoteBase;

    class BlockMsg {
    public:
        static bool newRound(ConsensusPhase phase, int baxCount);
        BlockMsg(uint32_t blockNum);
        void insert(const ProposeMsg& proposeMsg);
        void moveToNewStep(uint32_t blockNum, ConsensusPhase phase, int baxCount);
        bool isProposer() const;
        bool isVoter(ConsensusPhase phase, int baxCount);
        std::shared_ptr<StakeVoteBase> getVoterSys();
    private:
        PhaseMsgPtr initIfNeed(ConsensusPhase phase, int baxCount);
        std::vector<ProposeMsg> m_proposeMsgList;
        std::map<int, PhaseMsgPtr> m_phaseMessageMap; // key = phase + bax_count
        uint32_t m_blockNum = 0;
        bool m_isProposer = false;
        std::shared_ptr<AggEchoMsg> m_myAggEchoMsgPtr;
        std::vector<AggEchoMsg> m_aggEchoMsgV;
        std::shared_ptr<StakeVoteBase> m_stakeVote = nullptr;

        friend class MsgMgr;

        // Only for debug purpose.
        friend class Scheduler;
    };

    typedef std::shared_ptr<BlockMsg> BlockMessagePtr;
}
