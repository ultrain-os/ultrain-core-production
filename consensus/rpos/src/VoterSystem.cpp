#include "rpos/VoterSystem.h"

#include <limits>

#include <boost/math/distributions/binomial.hpp>

#include <crypto/Ed25519.h>
#include <rpos/Config.h>
#include <rpos/Node.h>
#include <rpos/Proof.h>

using boost::math::binomial;
using std::string;

namespace ultrainio {
    double VoterSystem::getProposerRatio() {
        int stakeNodeNumber = UranusNode::getInstance()->getGlobalProducingNodeNumber();
        long totalStake = stakeNodeNumber * Config::DEFAULT_THRESHOLD;
        return Config::PROPOSER_STAKES_NUMBER / totalStake;
    }

    double VoterSystem::getVoterRatio() {
        int stakeNodeNumber = UranusNode::getInstance()->getGlobalProducingNodeNumber();
        long totalStake = stakeNodeNumber * Config::DEFAULT_THRESHOLD;
        return Config::VOTER_STAKES_NUMBER / totalStake;
    }
    int VoterSystem::getStakes(const std::string &pk) {
        bool isNonProducingNode = UranusNode::getInstance()->getNonProducingNode();
        string myPk(UranusNode::getInstance()->getPublicKey());
        // (shenyufeng)always be no listener
        if (isNonProducingNode && pk == myPk)
            return 0;
        return Config::DEFAULT_THRESHOLD;
    }

    std::shared_ptr<std::vector<fc::variant>> VoterSystem::getCommitteeInfoList() {
        static const auto &ro_api = appbase::app().get_plugin<chain_plugin>().get_read_only_api();
        static struct chain_apis::read_only::get_producers_params params;
        params.json = false;
        params.lower_bound = "0";
        params.limit = 50;
        auto result = ro_api.get_producers(params, true);
//        auto result = rawResult.as<ultrainio::chain_apis::read_only::get_producers_result>();
        if (!result.rows.empty()) {
            for (const auto& r : result.rows ) {
               ilog("********************token ${token}, pk ${pk}", ("token", r)("pk", r["producer_key"]));
            }
        }
        return nullptr;
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
