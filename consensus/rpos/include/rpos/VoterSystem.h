#pragma once

#include <memory>
#include <string>
#include <vector>
#include <appbase/application.hpp>
#include <ultrainio/chain_plugin/chain_plugin.hpp>
using namespace appbase;

namespace ultrainio {
    // forward declare
    class Proof;
    struct get_producers_result;

    class VoterSystem {
    public:
        int getStakes(const std::string &pk);
        double getProposerRatio();
        double getVoterRatio();
        int count(const Proof& proof, int stakes, double p);
        std::shared_ptr<std::vector<fc::variant>> getCommitteeInfoList();
    private:
        int reverseBinoCdf(double rand, int stake, double p);
    };
}
