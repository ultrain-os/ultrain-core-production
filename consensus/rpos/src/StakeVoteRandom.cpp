#include "rpos/StakeVoteRandom.h"

#include <rpos/Config.h>
#include <rpos/Node.h>
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
        for (auto committeeInfo : committeeStatePtr->cinfo) {
            m_committeeV.push_back(committeeInfo.accountName);
        }
        std::shared_ptr<RoleSelection> roleSelectionPtr = std::make_shared<RoleSelection>(m_committeeV, rand);
        m_roleSelectionMap.insert(std::make_pair(kPhaseBA0 + 0, roleSelectionPtr));
    }

    uint32_t StakeVoteRandom::proposerPriority(const AccountName& account, ConsensusPhase phase, int baxCount) {
        if (isGenesisPeriod()) {
            if (isGenesisLeader(account)) {
                return 0;
            }
            ULTRAIN_ASSERT(false, chain::chain_exception, "handle no proposer message at genesis period. account : ${account}", ("account", std::string(account)));
        }
        std::shared_ptr<RoleSelection> roleSelectionPtr = getRoleSelectionInitIfNull(phase, baxCount);
        return roleSelectionPtr->proposerPriority(std::string(account));
    }

    std::shared_ptr<RoleSelection> StakeVoteRandom::getRoleSelection(ConsensusPhase phase, int baxCount) const {
        auto itor = m_roleSelectionMap.find(phase + baxCount);
        if (itor != m_roleSelectionMap.end()) {
            return itor->second;
        }
        return nullptr;
    }

    std::shared_ptr<RoleSelection> StakeVoteRandom::getRoleSelectionInitIfNull(ConsensusPhase phase, int baxCount) {
        std::shared_ptr<RoleSelection> roleSelectionPtr = getRoleSelection(phase, baxCount);
        if (!roleSelectionPtr) {
            RoleRandom rand(UranusNode::getInstance()->getPreviousHash(), m_blockNum, phase, baxCount);
            roleSelectionPtr = std::make_shared<RoleSelection>(m_committeeV, rand);
            m_roleSelectionMap.insert(std::make_pair(phase + baxCount, roleSelectionPtr));
        }
        return roleSelectionPtr;
    }

    void StakeVoteRandom::moveToNewStep(uint32_t blockNum, ConsensusPhase phase, int baxCount) {
        ULTRAIN_ASSERT(m_blockNum == blockNum, chain::chain_exception, "blockNum not equal");
        if (m_committeeV.size() > 0) {
            getRoleSelectionInitIfNull(phase, baxCount); // do init
        }
    }

    bool StakeVoteRandom::realIsProposer(const AccountName& account) {
        std::shared_ptr<RoleSelection> roleSelectionPtr = getRoleSelectionInitIfNull(kPhaseBA0, 0);
        ULTRAIN_ASSERT(roleSelectionPtr, chain::chain_exception, "RoleSelection is nullptr");
        return roleSelectionPtr->isProposer(std::string(account));
    }

    bool StakeVoteRandom::realIsVoter(const AccountName& account, ConsensusPhase phase, int baxCount) {
        std::shared_ptr<RoleSelection> roleSelectionPtr = getRoleSelectionInitIfNull(phase, baxCount);
        ULTRAIN_ASSERT(roleSelectionPtr, chain::chain_exception, "RoleSelection is nullptr");
        return roleSelectionPtr->isVoter(std::string(account));
    }

    int StakeVoteRandom::realGetSendEchoThreshold() const {
        const std::shared_ptr<RoleSelection> roleSelectionPtr = getRoleSelection(kPhaseBA0, 0);
        return Config::kSendEchoThresholdRatio * roleSelectionPtr->voterNumber() + 1;
    }

    int StakeVoteRandom::realGetNextRoundThreshold() const {
        const std::shared_ptr<RoleSelection> roleSelectionPtr = getRoleSelection(kPhaseBA0, 0);
        return Config::kNextRoundThresholdRatio * roleSelectionPtr->voterNumber() + 1;
    }

    int StakeVoteRandom::realGetEmptyBlockThreshold() const {
        const std::shared_ptr<RoleSelection> roleSelectionPtr = getRoleSelection(kPhaseBA0, 0);
        return Config::kEmptyBlockThresholdRatio * roleSelectionPtr->voterNumber() + 1;
    }

    int StakeVoteRandom::realGetEmptyBlock2Threshold() const {
        const std::shared_ptr<RoleSelection> roleSelectionPtr = getRoleSelection(kPhaseBA0, 0);
        return Config::kEmptyBlock2ThresholdRatio * roleSelectionPtr->voterNumber() + 1;
    }

    uint32_t StakeVoteRandom::realGetProposerNumber() const {
        const std::shared_ptr<RoleSelection> roleSelectionPtr = getRoleSelection(kPhaseBA0, 0);
        return roleSelectionPtr->proposerNumber();
    }
}