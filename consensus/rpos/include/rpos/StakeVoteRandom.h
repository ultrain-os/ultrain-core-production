#pragma once

#include <rpos/StakeVoteBase.h>

namespace ultrainio {
    class RoleRandom;
    class RoleSelection;

    class StakeVoteRandom : public StakeVoteBase {
    public:
        StakeVoteRandom(uint32_t blockNum, std::shared_ptr<CommitteeState> committeeStatePtr, const RoleRandom& rand);

        virtual uint32_t proposerPriority(const AccountName& account);
    protected:
        virtual bool realIsProposer(const AccountName& account);

        virtual bool realIsVoter(const AccountName& account);

        virtual int realGetSendEchoThreshold() const;

        virtual int realGetNextRoundThreshold() const;

        virtual int realGetEmptyBlockThreshold() const;

        virtual int realGetEmptyBlock2Threshold() const;

        virtual uint32_t realGetProposerNumber() const;

    private:
        void initRoleSelection(std::shared_ptr<CommitteeState> committeeStatePtr, const RoleRandom& rand);
        std::shared_ptr<RoleSelection> m_roleSelectionPtr = nullptr;
    };
}