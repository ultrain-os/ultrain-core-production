#pragma once

#include <string>

namespace ultrainio {
    class VoterSystem {
    public:
        static const int TOTAL_STAKES;
        static const int PROPOSER_STAKES;
        static const int VOTER_STAKES;
        static const double PROPOSER_RATIO;
        static const double VOTER_RATIO;
        int vote(const std::string& seed, const uint8_t* sk, int stake, double p, uint8_t* proof);
        int vote(const uint8_t* proof, int stakes, double p);
        uint32_t proof2Priority(const uint8_t* proof);
    private:
        int reverseBinoCdf(double rand, int stake, double p);
    };
}