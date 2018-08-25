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