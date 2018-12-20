#pragma once

#include <map>
#include <rpos/StakeVoteBase.h>

namespace ultrainio {
    class RoleRandom;
    class RoleSelection;

    class StakeVoteRandom : public StakeVoteBase {
    public:
        StakeVoteRandom(uint32_t blockNum, std::shared_ptr<CommitteeState> committeeStatePtr, const RoleRandom& rand);

        virtual uint32_t proposerPriority(const AccountName& account, ConsensusPhase phase, int baxCount);

        virtual void moveToNewStep(uint32_t blockNum, ConsensusPhase phase, int baxCount);
    protected:
        virtual bool realIsProposer(const AccountName& account);

        virtual bool realIsVoter(const AccountName& account, ConsensusPhase phase, int baxCount);

        virtual int realGetSendEchoThreshold() const;

        virtual int realGetNextRoundThreshold() const;

        virtual int realGetEmptyBlockThreshold() const;

        virtual int realGetEmptyBlock2Threshold() const;

        virtual uint32_t realGetProposerNumber() const;

    private:
        void initRoleSelection(std::shared_ptr<CommitteeState> committeeStatePtr, const RoleRandom& rand);
        std::shared_ptr<RoleSelection> getRoleSelection(ConsensusPhase phase, int baxCount) const;
        std::shared_ptr<RoleSelection> getRoleSelectionInitIfNull(ConsensusPhase phase, int baxCount);

        std::map<int, std::shared_ptr<RoleSelection> > m_roleSelectionMap; // key = phase + baxCount
        std::vector<std::string> m_committeeV;
    };
}