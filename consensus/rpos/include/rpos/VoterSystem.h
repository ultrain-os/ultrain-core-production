#pragma once
#include <rpos/CommitteeInfo.h>
#include <memory>
#include <string>
#include <vector>
#include <appbase/application.hpp>
#include <ultrainio/chain_plugin/chain_plugin.hpp>
using namespace appbase;

namespace ultrainio {
    // forward declare
    class PublicKey;
    class Proof;
    struct CommitteeInfo;

    class VoterSystem {
    public:
        int getStakes(const std::string &pk);
        double getProposerRatio();
        double getVoterRatio();
        int getCommitteeMemberNumber(uint32_t blockNum) const;
        bool isCommitteeMember(const PublicKey& publicKey) const;
        int count(const Proof& proof, int stakes, double p);
        std::shared_ptr<std::vector<CommitteeInfo>> getCommitteeInfoList();
    private:
        bool isStillGenesis(uint32_t blockNum) const;
        long getTotalStakes(uint32_t blockNum) const;
        int reverseBinoCdf(double rand, int stake, double p);
    };
}
