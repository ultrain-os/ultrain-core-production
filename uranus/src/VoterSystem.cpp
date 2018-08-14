#include "uranus/VoterSystem.h"

#include <limits>

#include <boost/math/distributions/binomial.hpp>

#include <crypto/Vrf.h>
#include <iostream>

using boost::math::binomial;
using std::string;

namespace ultrainio {
    const int VoterSystem::TOTAL_STAKES           = 10000;
    const int VoterSystem::PROPOSER_STAKES        = 20;
    const int VoterSystem::VOTER_STAKES           = 300;
    const double VoterSystem::PROPOSER_RATIO      = static_cast<double>(PROPOSER_STAKES) / TOTAL_STAKES;
    const double VoterSystem::VOTER_RATIO         = static_cast<double>(VOTER_STAKES) / TOTAL_STAKES;
    /*
     * seed - preHash + blockNum + phase + 01|02. 01 for proposer, 02 for voter
     */
    int VoterSystem::vote(const std::string& seed, const uint8_t* sk, int stakes, double p, uint8_t* proof) {
        Vrf::prove(proof, (uint8_t*)seed.data(), seed.length(), sk);
        return vote(proof, stakes, p);
    }

    int VoterSystem::vote(const uint8_t* proof, int stakes, double p) {
        uint32_t priority = proof2Priority(proof);
        double rand = static_cast<double>(priority) / std::numeric_limits<uint32_t>::max();
        return reverseBinoCdf(rand, stakes, p);
    }

    uint32_t VoterSystem::proof2Priority(const uint8_t* proof) {
        // 193:224 bit
        uint32_t priority = 0;
        size_t startIndex = 24; // 24 * 8 = 192
        size_t byteNum = 4;

        for (size_t i = 0; i < byteNum; i++) {
            priority += proof[startIndex + i];
            if (i != byteNum - 1) {
                priority = priority << 8;
            }
        }
        return priority;
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