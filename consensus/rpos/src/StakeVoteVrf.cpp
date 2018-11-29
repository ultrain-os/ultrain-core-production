#include <rpos/StakeVoteVrf.h>

namespace ultrainio {
    StakeVoteVrf::StakeVoteVrf(uint32_t blockNum, std::shared_ptr<CommitteeState> committeeStatePtr)
            : StakeVoteBase(blockNum, committeeStatePtr) {
        init();
    }

    void StakeVoteVrf::init() {
        long totalStake = getTotalStakes();
        ULTRAIN_ASSERT(totalStake != 0, chain::chain_exception, "totalStake is 0");
        m_proposerRatio = Config::kProposerStakeNumber / totalStake;
        m_voterRatio = Config::kVoterStakeNumber / totalStake;
    }

    long StakeVoteVrf::getTotalStakes() const {
        // todo(qinxiaofen)
        return getCommitteeMemberNumber() * 6000;
    }

    bool StakeVoteVrf::realIsProposer(const AccountName& account) {

    }

    bool StakeVoteVrf::realIsVoter(const AccountName& account);

    int StakeVoteVrf::realGetSendEchoThreshold() const;

    int StakeVoteVrf::realGetNextRoundThreshold() const;

    int StakeVoteVrf::realGetEmptyBlockThreshold() const;

    int StakeVoteVrf::realGetEmptyBlock2Threshold() const;

    uint32_t StakeVoteVrf::realGetProposerNumber() const;
}