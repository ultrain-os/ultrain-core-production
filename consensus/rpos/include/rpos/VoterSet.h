#pragma once

#include <vector>
#include <core/BlsVoterSet.h>
#include <core/types.h>

namespace ultrainio {
    struct VoterSet {
        CommonEchoMsg commonEchoMsg;
        std::vector<AccountName> accountPool;
        std::vector<std::string> sigPool;
        std::vector<std::string> blsSignPool;
        std::vector<uint32_t>    timePool;
#ifdef CONSENSUS_VRF
        std::vector<std::string> proofPool;
#endif
        bool empty() const;

        BlsVoterSet toBlsVoterSet(int weight) const;

        std::string generateSigX(const std::vector<std::string>& blsSignPool) const;
    };
}