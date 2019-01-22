#include <rpos/StakeVoteVrf.h>

#include <boost/math/distributions/binomial.hpp>
#include <rpos/Config.h>
#include <rpos/Node.h>
#include <rpos/Proof.h>
#include <rpos/Seed.h>
#include <rpos/Vrf.h>

namespace ultrainio {
#define THRESHOLD_STAKE 6000
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
        return getCommitteeMemberNumber() * THRESHOLD_STAKE;
    }

    bool StakeVoteVrf::realIsProposer(const AccountName& account, const Proof& proof) {
        if (!isCommitteeMember(account)) {
            return false;
        }

        if (commonCalSelectedStake(proof, THRESHOLD_STAKE, m_proposerRatio) > 0) {
            return true;
        }
        return false;
    }

    bool StakeVoteVrf::realIsVoter(const AccountName& account, const Proof& proof) {
        if (!isCommitteeMember(account)) {
            return false;
        }

        if (calSelectedStake(proof) > 0) {
            return true;
        }
        return false;
    }

    bool StakeVoteVrf::isCommitteeMember(const AccountName& account) const {
        CommitteeInfo c = findInCommitteeMemberList(account);
        return !c.isEmpty();
    }

    int StakeVoteVrf::calSelectedStake(const Proof& proof) {
        return commonCalSelectedStake(proof, THRESHOLD_STAKE, m_voterRatio);
    }

    int StakeVoteVrf::commonCalSelectedStake(const Proof& proof, int stakes, double p) {
        double rand = proof.getRand();
        return reverseBinoCdf(rand, stakes, p);
    }

    int StakeVoteVrf::reverseBinoCdf(double rand, int stake, double p) {
        int k = 0;
        boost::math::binomial b(stake, p);

        while (rand > boost::math::cdf(b, k)) {
            k++;
        }
        return k;
    }

    int StakeVoteVrf::realGetSendEchoThreshold() const {
        return Config::kVoterStakeNumber * Config::kSendEchoThresholdRatio + 1;
    }

    int StakeVoteVrf::realGetNextRoundThreshold() const {
        return Config::kVoterStakeNumber * Config::kNextRoundThresholdRatio + 1;
    }

    int StakeVoteVrf::realGetEmptyBlockThreshold() const {
        return Config::kVoterStakeNumber * Config::kEmptyBlockThresholdRatio + 1;
    }

    int StakeVoteVrf::realGetEmptyBlock2Threshold() const {
        return Config::kVoterStakeNumber * Config::kEmptyBlock2ThresholdRatio + 1;
    }

    void StakeVoteVrf::moveToNewStep(uint32_t blockNum, ConsensusPhase phase, int baxCount) {
        ULTRAIN_ASSERT(blockNum == m_blockNum, chain::chain_exception, "blockNum is not equal");
        BlockIdType blockId = UranusNode::getInstance()->getPreviousHash();
        std::string previousHash(blockId.data());
        if (!m_proposerProof.isValid()) {
            Seed seed(previousHash, blockNum, kPhaseBA0, 0);
            m_proposerProof = Vrf::vrf(StakeVoteBase::getMyPrivateKey(), seed, Vrf::kProposer);
        }
        auto itor = m_phaseProofMap.find(phase + baxCount);
        if (itor == m_phaseProofMap.end()) {
            Seed seed(previousHash, blockNum, phase, baxCount);
            Proof proof = Vrf::vrf(StakeVoteBase::getMyPrivateKey(), seed, Vrf::kVoter);
            m_phaseProofMap.insert(std::make_pair(phase + baxCount, proof));
        }
    }

    Proof StakeVoteVrf::getVoterProof(uint32_t blockNum, ConsensusPhase phase, int baxCount) {
        ULTRAIN_ASSERT(blockNum == m_blockNum, chain::chain_exception, "blockNum is not equal");
        ULTRAIN_ASSERT(m_phaseProofMap.find(phase + baxCount) != m_phaseProofMap.end(), chain::chain_exception, "should be implemented by subclass");
        return m_phaseProofMap[phase + baxCount];
    }

    Proof StakeVoteVrf::getProposerProof(uint32_t blockNum) {
        ULTRAIN_ASSERT(blockNum == m_blockNum, chain::chain_exception, "blockNum is not equal");
        ULTRAIN_ASSERT(m_proposerProof.isValid(), chain::chain_exception, "should be implemented by subclass");
        return m_proposerProof;
    }
}