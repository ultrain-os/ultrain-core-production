#include <rpos/StakeVoteBase.h>

#include <limits>

#include <boost/math/distributions/binomial.hpp>

#include <rpos/Config.h>
#include <rpos/Genesis.h>
#include <rpos/Proof.h>
#include <rpos/NodeInfo.h>
#include <rpos/RoleSelection.h>

#include <appbase/application.hpp>
#include <ultrainio/chain_plugin/chain_plugin.hpp>

using std::string;
using namespace appbase;

namespace ultrainio {
    std::shared_ptr<NodeInfo> StakeVoteBase::s_keyKeeper = std::make_shared<NodeInfo>();

    StakeVoteBase::StakeVoteBase(uint32_t blockNum, std::shared_ptr<CommitteeState> committeeStatePtr)
            : m_committeeStatePtr(committeeStatePtr), m_blockNum(blockNum) {
        if (!m_committeeStatePtr) {
            m_committeeStatePtr = getCommitteeState();
        }
        computeCommitteeMroot();
        ULTRAIN_ASSERT(getCommitteeMemberNumber() != 0, chain::chain_exception, "totalStake is 0");
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

    void StakeVoteBase::computeCommitteeMroot() {
        if (!m_committeeStatePtr) {
            m_committeeMroot = chain::checksum256_type();
            return;
        }
        vector<string> accounts;
        for (const auto& c : m_committeeStatePtr->cinfo) {
            accounts.push_back(c.accountName);
        }
        std::sort(accounts.begin(), accounts.end());
        m_committeeMroot = chain::digest_type::hash(accounts);
        dlog("--------- committee mroot is ${mroot}", ("mroot", m_committeeMroot));
    }

    bool StakeVoteBase::committeeHasWorked() {
        wlog("be caution to call this");
        static const auto &ro_api = appbase::app().get_plugin<chain_plugin>().get_read_only_api();
        return ro_api.is_genesis_finished();
    }

    bool StakeVoteBase::committeeHasWorked2() const {
        if (m_committeeStatePtr && m_committeeStatePtr->chainStateNormal) {
            return true;
        }
        return false;
    }

    bool StakeVoteBase::isGenesisPeriod() const {
        boost::chrono::minutes genesisElapsed
                = boost::chrono::duration_cast<boost::chrono::minutes>(boost::chrono::system_clock::now() - Genesis::s_time);
        if (!committeeHasWorked2() && (genesisElapsed < boost::chrono::minutes(Genesis::s_genesisStartupTime))) {
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
        AccountName myAccount = getMyAccount();
        if (myIsNonProducingNode && account == myAccount) { // non producer
            return false;
        }
        if (isGenesisPeriod()) {
            if (isGenesisLeader(account)) {
                return true; // genesis node
            } else {
                return false;
            }
        }
        return realIsProposer(account);
    }

    bool StakeVoteBase::isVoter(const AccountName& account, bool myIsNonProducingNode) {
        AccountName myAccount = getMyAccount();
        if (myIsNonProducingNode && account == myAccount) { // non producer
            return false;
        }
        if (isGenesisPeriod()) {
            if (isGenesisLeader(account)) {
                return true; // genesis node
            } else {
                return false;
            }
        }
        return realIsVoter(account);
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

    PublicKey StakeVoteBase::findInCommitteeMemberList(const AccountName& account) const {
        if (m_committeeStatePtr) {
            for (auto& v : m_committeeStatePtr->cinfo) {
                ULTRAIN_ASSERT(!v.accountName.empty(), chain::chain_exception, "account name is empty");
                if (account == AccountName(v.accountName)) {
                    return PublicKey(v.pk);
                }
            }
        }
        return PublicKey();
    }

    PublicKey StakeVoteBase::getPublicKey(const AccountName& account) const {
        if (account == s_keyKeeper->getMyAccount()) {
            return s_keyKeeper->getPrivateKey().getPublicKey();
        } else if (account == AccountName(Genesis::kGenesisAccount)) {
            return PublicKey(Genesis::s_genesisPk);
        }
        return findInCommitteeMemberList(account);
    }

    bool StakeVoteBase::isGenesisLeader(const AccountName& account) const {
        return account.good() && account == AccountName(Genesis::kGenesisAccount);
    }

    std::shared_ptr<CommitteeState> StakeVoteBase::getCommitteeState() {
        static const auto &ro_api = appbase::app().get_plugin<chain_plugin>().get_read_only_api();
        static struct chain_apis::read_only::get_producers_params params;
        CommitteeInfo cinfo;
        auto statePtr(std::make_shared<CommitteeState>());
        params.json=true;
        params.lower_bound="";
        try {
            auto result = ro_api.get_producers(params, true);
            if(!result.rows.empty()) {
                for( const auto& r : result.rows ) {
                    cinfo.accountName = r["owner"].as_string();
                    cinfo.pk = r["producer_key"].as_string();
                    cinfo.stakesCount = r["total_cons_staked"].as_int64();
                    statePtr->cinfo.push_back(cinfo);
                }
                ilog("#########################there are ${p} committee member enabled", ("p", result.rows.size()));
            }
            statePtr->chainStateNormal = ro_api.is_genesis_finished();
            statePtr->chainMinStakeThresh = result.min_stake_thresh;
        }
        catch (fc::exception& e) {
            ilog("there may be no producer registered: ${e}", ("e", e.to_string()));
            return nullptr;
        }
        return statePtr;
    }

    uint32_t StakeVoteBase::proposerPriority(const AccountName& account) {
        ULTRAIN_ASSERT(false, chain::chain_exception, "no be there");
    }

    bool StakeVoteBase::realIsProposer(const AccountName& account) {
        ULTRAIN_ASSERT(false, chain::chain_exception, "no be there");
    }

    bool StakeVoteBase::realIsVoter(const AccountName& account) {
        ULTRAIN_ASSERT(false, chain::chain_exception, "no be there");
    }

    int StakeVoteBase::realGetSendEchoThreshold() const {
        ULTRAIN_ASSERT(false, chain::chain_exception, "no be there");
    }

    int StakeVoteBase::realGetNextRoundThreshold() const {
        ULTRAIN_ASSERT(false, chain::chain_exception, "no be there");
    }

    int StakeVoteBase::realGetEmptyBlockThreshold() const {
        ULTRAIN_ASSERT(false, chain::chain_exception, "no be there");
    }

    int StakeVoteBase::realGetEmptyBlock2Threshold() const {
        ULTRAIN_ASSERT(false, chain::chain_exception, "no be there");
    }

    uint32_t StakeVoteBase::realGetProposerNumber() const {
        ULTRAIN_ASSERT(false, chain::chain_exception, "no be there");
    }
}
