#pragma once

#include <rpos/StakeVoteBase.h>

namespace ultrainio {
    class StakeVoteVrf : public StakeVoteBase {
    public:
        StakeVoteVrf(uint32_t blockNum, std::shared_ptr<CommitteeState> committeeStatePtr);

    protected:
        virtual bool realIsProposer(const AccountName& account);

        virtual bool realIsVoter(const AccountName& account);

        virtual int realGetSendEchoThreshold() const;

        virtual int realGetNextRoundThreshold() const;

        virtual int realGetEmptyBlockThreshold() const;

        virtual int realGetEmptyBlock2Threshold() const;

        virtual int realGetProposerNumber() const;

    private:
        long getTotalStakes() const;
        void init();

        double m_proposerRatio = 0.0;
        double m_voterRatio = 0.0;
    };
}