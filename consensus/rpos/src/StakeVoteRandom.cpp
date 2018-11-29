#include "rpos/StakeVoteRandom.h"

#include <rpos/Config.h>
#include <rpos/RoleRandom.h>
#include <rpos/RoleSelection.h>

namespace ultrainio {
    StakeVoteRandom::StakeVoteRandom(uint32_t blockNum, std::shared_ptr<CommitteeState> committeeStatePtr, const RoleRandom& rand)
            : StakeVoteBase(blockNum, committeeStatePtr) {
        if (!isGenesisPeriod()) {
            // init with new committeeState
            initRoleSelection(m_committeeStatePtr, rand);
        }
    }

    void StakeVoteRandom::initRoleSelection(std::shared_ptr<CommitteeState> committeeStatePtr, const RoleRandom& rand) {
        ULTRAIN_ASSERT(committeeStatePtr != nullptr, chain::chain_exception, "committeeStatePtr is null");
        std::vector<std::string> committeeV;
        for (auto committeeInfo : committeeStatePtr->cinfo) {
            committeeV.push_back(committeeInfo.accountName);
        }
        m_roleSelectionPtr = std::make_shared<RoleSelection>(committeeV, rand);
    }

    uint32_t StakeVoteRandom::proposerPriority(const AccountName& account) {
        if (isGenesisPeriod()) {
            if (isGenesisLeader(account)) {
                return 0;
            }
            ULTRAIN_ASSERT(false, chain::chain_exception, "handle no proposer message at genesis period.");
        }
        return m_roleSelectionPtr->proposerPriority(std::string(account));
    }

    bool StakeVoteRandom::realIsProposer(const AccountName& account) {
        return m_roleSelectionPtr->isProposer(std::string(account));
    }

    bool StakeVoteRandom::realIsVoter(const AccountName& account) {
        return m_roleSelectionPtr->isVoter(std::string(account));
    }

    int StakeVoteRandom::realGetSendEchoThreshold() const {
        return Config::kSendEchoThresholdRatio * m_roleSelectionPtr->voterNumber() + 1;
    }

    int StakeVoteRandom::realGetNextRoundThreshold() const {
        return Config::kNextRoundThresholdRatio * m_roleSelectionPtr->voterNumber() + 1;
    }

    int StakeVoteRandom::realGetEmptyBlockThreshold() const {
        return Config::kEmptyBlockThresholdRatio * m_roleSelectionPtr->voterNumber() + 1;
    }

    int StakeVoteRandom::realGetEmptyBlock2Threshold() const {
        return Config::kEmptyBlock2ThresholdRatio * m_roleSelectionPtr->voterNumber() + 1;
    }

    uint32_t StakeVoteRandom::realGetProposerNumber() const {
        return m_roleSelectionPtr->proposerNumber();
    }
}