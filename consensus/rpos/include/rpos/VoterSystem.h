#pragma once

#include <string>

namespace ultrainio {
    // forward declare
    class Proof;

    class VoterSystem {
    public:
        int getStakes(const std::string &pk);
        double getProposerRatio();
        double getVoterRatio();
        int count(const Proof& proof, int stakes, double p);
    private:
        int reverseBinoCdf(double rand, int stake, double p);
    };
}