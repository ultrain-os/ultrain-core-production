#include <rpos/StakeVoteBase.h>

#include <limits>

#include <boost/math/distributions/binomial.hpp>

#include <base/Hex.h>
#include <rpos/Config.h>
#include <rpos/Genesis.h>
#include <rpos/Proof.h>
#include <rpos/NodeInfo.h>
#include <rpos/RoleSelection.h>

#include <appbase/application.hpp>
#include <ultrainio/chain_plugin/chain_plugin.hpp>
#include <crypto/Bls.h>
#include <lightclient/LightClientProducer.h>

using std::string;
using namespace appbase;

namespace ultrainio {

#define GENESIS_ROLE_CHECK(account,isNonProducer)                                           \
            AccountName myAccount = getMyAccount();                                         \
            if (isNonProducer && account == myAccount) {                                    \
                return false;                                                               \
            }                                                                               \
            if (isGenesisPeriod()) {                                                        \
                if (isGenesisLeader(account)) {                                             \
                    return true;                                                            \
                } else {                                                                    \
                    return false;                                                           \
                }                                                                           \
            }

    std::shared_ptr<NodeInfo> StakeVoteBase::s_keyKeeper = std::make_shared<NodeInfo>();

    StakeVoteBase::StakeVoteBase(uint32_t blockNum, std::shared_ptr<CommitteeState> committeeStatePtr)
            : m_committeeStatePtr(committeeStatePtr), m_blockNum(blockNum) {
        if (!m_committeeStatePtr) {
            m_committeeStatePtr = getCommitteeState(chain::master_chain_name);
        }
        ULTRAIN_ASSERT(getCommitteeMemberNumber() != 0, chain::chain_exception, "totalStake is 0");
        const auto &ro_api = appbase::app().get_plugin<chain_plugin>().get_read_only_api();
        chain_apis::read_only::get_confirm_point_interval_result result = ro_api.get_confirm_point_interval(chain_apis::read_only::get_confirm_point_interval_params());
        LightClientProducer::setConfirmPointInterval(result.confirm_point_interval);
    }

    StakeVoteBase::~StakeVoteBase() {
    }

    std::shared_ptr<NodeInfo> StakeVoteBase::getKeyKeeper() {
        return s_keyKeeper;
    }

    AccountName StakeVoteBase::getMyAccount() {
        return s_keyKeeper->getMyAccount();
    }

    PrivateKey StakeVoteBase::getMyPrivateKey() {
        return s_keyKeeper->getPrivateKey();
    }

    bool StakeVoteBase::getMyBlsPrivateKey(unsigned char* sk, int skSize) {
        return s_keyKeeper->getMyBlsPrivateKey(sk, skSize);
    }

    bool StakeVoteBase::newRound(ConsensusPhase phase, int baxCount) {
        if (kPhaseBA0 == phase && 0 == baxCount) {
            return true;
        }
        return false;
    }

    SHA256 StakeVoteBase::getCommitteeMroot() const {
        if (m_committeeStatePtr && m_committeeStatePtr->chainStateNormal && m_committeeStatePtr->cinfo.size() > 0) {
            return CommitteeSet(m_committeeStatePtr->cinfo).committeeMroot();
        }
        return SHA256();
    }

    CommitteeSet StakeVoteBase::getCommitteeSet() const {
        if (!m_committeeStatePtr) {
            return CommitteeSet();
        }
        return CommitteeSet(m_committeeStatePtr->cinfo);
    }

    // static
    bool StakeVoteBase::committeeHasWorked() {
        static const auto &ro_api = appbase::app().get_plugin<chain_plugin>().get_read_only_api();
        return ro_api.is_genesis_finished();
    }

    // static
    bool StakeVoteBase::isGenesisLeaderAndInGenesisPeriod() {
        fc::time_point current_time = fc::time_point::now();
        int pass_time_to_genesis_min = (current_time - Genesis::s_time).to_seconds() / 60;
        if (pass_time_to_genesis_min < Genesis::s_genesisStartupTime
            && StakeVoteBase::getMyAccount() == AccountName(Genesis::kGenesisAccount)
            && !committeeHasWorked()) {
            return true;
        }
        return false;
    }

    bool StakeVoteBase::committeeHasWorked2() const {
        if (m_committeeStatePtr && m_committeeStatePtr->chainStateNormal) {
            return true;
        }
        return false;
    }

    bool StakeVoteBase::isGenesisPeriod() const {
        fc::time_point current_time = fc::time_point::now();
        int pass_time_to_genesis_min = (current_time - Genesis::s_time).to_seconds() / 60;
        if (!committeeHasWorked2() && (pass_time_to_genesis_min < Genesis::s_genesisStartupTime)) {
            return true;
        }
        return false;
    }

    int StakeVoteBase::getCommitteeMemberNumber() const {
        if (isGenesisPeriod()) {
            return 1; // genesis leader only
        }
        if (!committeeHasWorked2()) { // pass genesis startup , but still has not world state, that is fresh node join
            return 1;
        }
        if (!m_committeeStatePtr) {
            if (m_blockNum > Genesis::s_genesisStartupBlockNum) {
                ULTRAIN_ASSERT(m_committeeStatePtr != nullptr, chain::chain_exception, "DO YOU HAVE STAKES");
            }
            // may be a bundle of node join
            return 1;
        }

        return m_committeeStatePtr->cinfo.size();
    }

    bool StakeVoteBase::isProposer(const AccountName& account, bool myIsNonProducingNode) {
        GENESIS_ROLE_CHECK(account, myIsNonProducingNode);
        return realIsProposer(account);
    }

    bool StakeVoteBase::isVoter(const AccountName& account, ConsensusPhase phase, int baxCount, bool myIsNonProducingNode) {
        GENESIS_ROLE_CHECK(account, myIsNonProducingNode);
        return realIsVoter(account, phase, baxCount);
    }

    bool StakeVoteBase::isProposer(const AccountName& account, const Proof& proof, bool myIsNonProducingNode) {
        GENESIS_ROLE_CHECK(account, myIsNonProducingNode);
        return realIsProposer(account, proof);
    }

    bool StakeVoteBase::isVoter(const AccountName& account, const Proof& proof, bool myIsNonProducingNode) {
        GENESIS_ROLE_CHECK(account, myIsNonProducingNode);
        return realIsVoter(account, proof);
    }

    int StakeVoteBase::calSelectedStake(const Proof& proof) {
        ULTRAIN_ASSERT(false, chain::chain_exception, "should be implemented by subclass");
    }

    void StakeVoteBase::moveToNewStep(uint32_t blockNum, ConsensusPhase phase, int baxCount) {
        ULTRAIN_ASSERT(false, chain::chain_exception, "should be implemented by subclass");
    }

    uint32_t StakeVoteBase::getProposerNumber() const {
        if (isGenesisPeriod()) {
            return 1;
        }
        return realGetProposerNumber();
    }

    int StakeVoteBase::getSendEchoThreshold() const {
        if (isGenesisPeriod()) {
            return 1;
        }
        return realGetSendEchoThreshold();
    }

    int StakeVoteBase::getNextRoundThreshold() const {
        if (isGenesisPeriod()) {
            return 1;
        }
        return realGetNextRoundThreshold();
    }

    int StakeVoteBase::getEmptyBlockThreshold() const {
        if (isGenesisPeriod()) {
            return 1;
        }
        return realGetEmptyBlockThreshold();
    }

    int StakeVoteBase::getEmptyBlock2Threshold() const {
        if (isGenesisPeriod()) {
            return 1;
        }
        return realGetEmptyBlock2Threshold();
    }

    CommitteeInfo StakeVoteBase::findInCommitteeMemberList(const AccountName& account) const {
        if (m_committeeStatePtr) {
            for (auto& v : m_committeeStatePtr->cinfo) {
                ULTRAIN_ASSERT(!v.accountName.empty(), chain::chain_exception, "account name is empty");
                if (account == AccountName(v.accountName)) {
                    return v;
                }
            }
        }
        return CommitteeInfo();
    }

    PublicKey StakeVoteBase::getPublicKey(const AccountName& account) const {
        if (account == s_keyKeeper->getMyAccount()) {
            return s_keyKeeper->getPrivateKey().getPublicKey();
        } else if (account == AccountName(Genesis::kGenesisAccount)) {
            return PublicKey(Genesis::s_genesisPk);
        } else {
            CommitteeInfo c = findInCommitteeMemberList(account);
            return PublicKey(c.pk);
        }
    }

    bool StakeVoteBase::getCommitteeBlsPublicKey(const AccountName& account, unsigned char* blsPublicKey, int pkSize) const {
        if (account == AccountName(Genesis::kGenesisAccount)) {
            Hex::fromHex<unsigned char>(Genesis::s_genesisBlsPk, blsPublicKey, Bls::BLS_PUB_KEY_COMPRESSED_LENGTH);
            return true;
        } else {
            CommitteeInfo c = findInCommitteeMemberList(account);
            if (c.isEmpty()) {
                return false;
            }
            Hex::fromHex<unsigned char>(c.blsPk, blsPublicKey, pkSize);
            return true;
        }
        return false;
    }

    bool StakeVoteBase::isGenesisLeader(const AccountName& account) const {
        return account.good() && account == AccountName(Genesis::kGenesisAccount);
    }

    // static
    std::shared_ptr<CommitteeState> StakeVoteBase::getCommitteeState(uint64_t chainNum) {
        const auto &ro_api = appbase::app().get_plugin<chain_plugin>().get_read_only_api();
        struct chain_apis::read_only::get_producers_params params;
        CommitteeInfo cinfo;
        auto statePtr(std::make_shared<CommitteeState>());
        params.json=true;
        params.lower_bound="";
        params.all_chain = false;
        params.chain_name = chainNum;
        try {
            auto result = ro_api.get_producers(params);
            if(!result.rows.empty()) {
                for( const auto& r : result.rows ) {
                    cinfo.accountName = r.prod_detail["owner"].as_string();
                    cinfo.pk = r.prod_detail["producer_key"].as_string();
                    cinfo.blsPk = r.prod_detail["bls_key"].as_string();
                    //cinfo.stakesCount = r.prod_detail["total_cons_staked"].as_int64();
                    statePtr->cinfo.push_back(cinfo);
                }
                ilog("#########################there are ${p} committee member enabled", ("p", result.rows.size()));
            }
            statePtr->chainStateNormal = ro_api.is_genesis_finished();
        }
        catch (fc::exception& e) {
            ilog("there may be no producer registered: ${e}", ("e", e.to_string()));
            return nullptr;
        } catch (...) {
            elog("error thrown from getCommitteeState ");
            return nullptr;
         }
        return statePtr;
    }

    uint32_t StakeVoteBase::proposerPriority(const AccountName& account, ConsensusPhase phase, int baxCount) {
        ULTRAIN_ASSERT(false, chain::chain_exception, "should be implemented by subclass");
    }

    Proof StakeVoteBase::getVoterProof(uint32_t blockNum, ConsensusPhase phase, int baxCount) {
        ULTRAIN_ASSERT(false, chain::chain_exception, "should be implemented by subclass");
    }

    Proof StakeVoteBase::getProposerProof(uint32_t blockNum) {
        ULTRAIN_ASSERT(false, chain::chain_exception, "should be implemented by subclass");
    }

    bool StakeVoteBase::realIsProposer(const AccountName& account) {
        ULTRAIN_ASSERT(false, chain::chain_exception, "should be implemented by subclass");
    }

    bool StakeVoteBase::realIsVoter(const AccountName& account, ConsensusPhase phase, int baxCount) {
        ULTRAIN_ASSERT(false, chain::chain_exception, "should be implemented by subclass");
    }

    int StakeVoteBase::realGetSendEchoThreshold() const {
        ULTRAIN_ASSERT(false, chain::chain_exception, "should be implemented by subclass");
    }

    int StakeVoteBase::realGetNextRoundThreshold() const {
        ULTRAIN_ASSERT(false, chain::chain_exception, "should be implemented by subclass");
    }

    int StakeVoteBase::realGetEmptyBlockThreshold() const {
        ULTRAIN_ASSERT(false, chain::chain_exception, "should be implemented by subclass");
    }

    int StakeVoteBase::realGetEmptyBlock2Threshold() const {
        ULTRAIN_ASSERT(false, chain::chain_exception, "should be implemented by subclass");
    }

    uint32_t StakeVoteBase::realGetProposerNumber() const {
        ULTRAIN_ASSERT(false, chain::chain_exception, "should be implemented by subclass");
    }

    bool StakeVoteBase::realIsProposer(const AccountName& account, const Proof& proof) {
        ULTRAIN_ASSERT(false, chain::chain_exception, "should be implemented by subclass");
    }

    bool StakeVoteBase::realIsVoter(const AccountName& account, const Proof& proof) {
        ULTRAIN_ASSERT(false, chain::chain_exception, "should be implemented by subclass");
    }
}
