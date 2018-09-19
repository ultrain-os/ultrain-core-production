#include "rpos/VoterSystem.h"

#include <limits>

#include <boost/math/distributions/binomial.hpp>

#include <rpos/Config.h>
#include <rpos/Genesis.h>
#include <rpos/Proof.h>
#include <rpos/KeyKeeper.h>

#include <appbase/application.hpp>
#include <ultrainio/chain_plugin/chain_plugin.hpp>

using boost::math::binomial;
using std::string;
using namespace appbase;

namespace ultrainio {
    std::shared_ptr<KeyKeeper> VoterSystem::s_keyKeeper = std::make_shared<KeyKeeper>();

    std::shared_ptr<VoterSystem> VoterSystem::create(uint32_t blockNum, std::shared_ptr<CommitteeState> committeeStatePtr) {
        VoterSystem* voterSysPtr = new VoterSystem(blockNum, committeeStatePtr);
        return std::shared_ptr<VoterSystem>(voterSysPtr);
    }

    VoterSystem::VoterSystem(uint32_t blockNum, std::shared_ptr<CommitteeState> committeeStatePtr)
            : m_blockNum(blockNum), m_committeeStatePtr(committeeStatePtr) {
        if (!m_committeeStatePtr) {
            m_committeeStatePtr = getCommitteeState();
        }
        long totalStake = getTotalStakes();
        ULTRAIN_ASSERT(totalStake != 0, chain::chain_exception, "totalStake is 0");
        m_proposerRatio = Config::kProposerStakeNumber / totalStake;
        m_voterRatio = Config::VOTER_STAKES_NUMBER / totalStake;
    }

    std::shared_ptr<KeyKeeper> VoterSystem::getKeyKeeper() {
        return s_keyKeeper;
    }

    AccountName VoterSystem::getMyAccount() {
        return s_keyKeeper->getMyAccount();
    }

    PrivateKey VoterSystem::getMyPrivateKey() {
        return s_keyKeeper->getPrivateKey();
    }

    bool VoterSystem::committeeHasWorked() {
        wlog("be caution to call this");
        static const auto &ro_api = appbase::app().get_plugin<chain_plugin>().get_read_only_api();
        return ro_api.is_genesis_finished();
    }

    bool VoterSystem::committeeHasWorked2() const {
        if (m_committeeStatePtr && m_committeeStatePtr->chainStateNormal) {
            return true;
        }
        return false;
    }

    double VoterSystem::getProposerRatio() {
        return m_proposerRatio;
    }

    double VoterSystem::getVoterRatio() {
        return m_voterRatio;
    }

    bool VoterSystem::isGenesisPeriod() const {
        boost::chrono::minutes genesisElapsed
                = boost::chrono::duration_cast<boost::chrono::minutes>(boost::chrono::system_clock::now() - Genesis::s_time);
        if (!committeeHasWorked2() && (genesisElapsed < boost::chrono::minutes(Genesis::s_genesisStartupTime))) {
            return true;
        }
        return false;
    }

    long VoterSystem::getTotalStakes() const {
        return getCommitteeMemberNumber() * Config::DEFAULT_THRESHOLD;
    }

    int VoterSystem::getStakes(const AccountName& account, bool isNonProducingNode) {
        AccountName myAccount = getMyAccount();
        // (shenyufeng)always be no listener
        if (isNonProducingNode && account == myAccount) {
            return 0;
        } else if (isCommitteeMember(account)) {
            return Config::DEFAULT_THRESHOLD;
        }
        return 0;
    }

    int VoterSystem::getCommitteeMemberNumber() const {
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

    bool VoterSystem::isCommitteeMember(const AccountName& account) const {
        bool genesis = isGenesisPeriod();
        if ((genesis && isGenesisLeader(account))
            || (!genesis && findInCommitteeMemberList(account).isValid())) {
            return true;
        }
        return false;
    }

    PublicKey VoterSystem::findInCommitteeMemberList(const AccountName& account) const {
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

    PublicKey VoterSystem::getPublicKey(const AccountName& account) const {
        if (account == s_keyKeeper->getMyAccount()) {
            return s_keyKeeper->getPrivateKey().getPublicKey();
        } else if (account == AccountName(Genesis::kGenesisAccount)) {
            return PublicKey(Genesis::s_genesisPk);
        }
        return findInCommitteeMemberList(account);
    }

    bool VoterSystem::isGenesisLeader(const AccountName& account) const {
        return account.good() && account == AccountName(Genesis::kGenesisAccount);
    }

    std::shared_ptr<CommitteeState> VoterSystem::getCommitteeState() {
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
                    cinfo.stakesCount = r["total_votes"].as_int64();
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

    int VoterSystem::count(const Proof& proof, int stakes, double p) {
        double rand = proof.getRand();
        return reverseBinoCdf(rand, stakes, p);
    }

    int VoterSystem::reverseBinoCdf(double rand, int stake, double p) {
        int k = 0;
        binomial b(stake, p);

        while (rand > boost::math::cdf(b, k)) {
            k++;
        }
        return k;
    }
}
