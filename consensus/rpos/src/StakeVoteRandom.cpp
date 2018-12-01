#include "rpos/StakeVoteRandom.h"

#include <rpos/Config.h>
#include <rpos/RoleRandom.h>
#include <rpos/RoleSelection.h>

namespace ultrainio {
    StakeVoteRandom::StakeVoteRandom(uint32_t blockNum, std::shared_ptr<CommitteeState> committeeStatePtr, const RoleRandom& rand)
            : StakeVoteBase(blockNum, committeeStatePtr) {
        if (!isGenesisPeriod()) {
            // double check committee is work. And m_committeeStatePtr may be null when new node join network
            if (committeeHasWorked2()) {
                initRoleSelection(m_committeeStatePtr, rand);
            }
        }
    }

    void StakeVoteRandom::initRoleSelection(std::shared_ptr<CommitteeState> committeeStatePtr, const RoleRandom& rand) {
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
        ULTRAIN_ASSERT(m_roleSelectionPtr != nullptr, chain::chain_exception, "m_roleSelectionPtr is null");
        return m_roleSelectionPtr->proposerPriority(std::string(account));
    }

    bool StakeVoteRandom::realIsProposer(const AccountName& account) {
        if (!m_roleSelectionPtr) {
            return false;
        }
        return m_roleSelectionPtr->isProposer(std::string(account));
    }

    bool StakeVoteRandom::realIsVoter(const AccountName& account) {
        if (!m_roleSelectionPtr) {
            return false;
        }
        return m_roleSelectionPtr->isVoter(std::string(account));
    }

    int StakeVoteRandom::realGetSendEchoThreshold() const {
        ULTRAIN_ASSERT(m_roleSelectionPtr != nullptr, chain::chain_exception, "m_roleSelectionPtr is null");
        return Config::kSendEchoThresholdRatio * m_roleSelectionPtr->voterNumber() + 1;
    }

    int StakeVoteRandom::realGetNextRoundThreshold() const {
        ULTRAIN_ASSERT(m_roleSelectionPtr != nullptr, chain::chain_exception, "m_roleSelectionPtr is null");
        return Config::kNextRoundThresholdRatio * m_roleSelectionPtr->voterNumber() + 1;
    }

    int StakeVoteRandom::realGetEmptyBlockThreshold() const {
        ULTRAIN_ASSERT(m_roleSelectionPtr != nullptr, chain::chain_exception, "m_roleSelectionPtr is null");
        return Config::kEmptyBlockThresholdRatio * m_roleSelectionPtr->voterNumber() + 1;
    }

    int StakeVoteRandom::realGetEmptyBlock2Threshold() const {
        ULTRAIN_ASSERT(m_roleSelectionPtr != nullptr, chain::chain_exception, "m_roleSelectionPtr is null");
        return Config::kEmptyBlock2ThresholdRatio * m_roleSelectionPtr->voterNumber() + 1;
    }

    uint32_t StakeVoteRandom::realGetProposerNumber() const {
        ULTRAIN_ASSERT(m_roleSelectionPtr != nullptr, chain::chain_exception, "m_roleSelectionPtr is null");
        return m_roleSelectionPtr->proposerNumber();
    }
}