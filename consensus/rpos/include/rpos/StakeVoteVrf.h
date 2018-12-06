#pragma once

#include <rpos/StakeVoteBase.h>
#include <rpos/Proof.h>

namespace ultrainio {
    class Proof;

    class StakeVoteVrf : public StakeVoteBase {
    public:
        StakeVoteVrf(uint32_t blockNum, std::shared_ptr<CommitteeState> committeeStatePtr);

    protected:
        virtual bool realIsProposer(const AccountName& account, const Proof& proof);

        virtual bool realIsVoter(const AccountName& account, const Proof& proof);

        virtual int calSelectedStake(const Proof& proof);

        virtual void moveToNewStep(uint32_t blockNum, ConsensusPhase phase, int baxCount);

        virtual Proof getVoterProof(uint32_t blockNum, ConsensusPhase phase, int baxCount);

        virtual Proof getProposerProof(uint32_t blockNum);

    private:
        virtual int realGetSendEchoThreshold() const;

        virtual int realGetNextRoundThreshold() const;

        virtual int realGetEmptyBlockThreshold() const;

        virtual int realGetEmptyBlock2Threshold() const;

        bool isCommitteeMember(const AccountName& account) const;

        int commonCalSelectedStake(const Proof& proof, int stakes, double p);

        int reverseBinoCdf(double rand, int stake, double p);

        long getTotalStakes() const;

        void init();

        Proof m_proposerProof;
        std::map<int, Proof> m_phaseProofMap; // key = phase + baxCount
        double m_proposerRatio = 0.0;
        double m_voterRatio = 0.0;
    };
}