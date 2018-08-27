#pragma once

#include <memory>
#include <string>
#include <vector>

namespace ultrainio {
    // forward declare
    class CommitteeInfo;
    class Proof;

    class VoterSystem {
    public:
        int getStakes(const std::string &pk);
        double getProposerRatio();
        double getVoterRatio();
        int count(const Proof& proof, int stakes, double p);
        std::shared_ptr<std::vector<CommitteeInfo>> getCommitteeInfoList();
    private:
        int reverseBinoCdf(double rand, int stake, double p);
    };
}