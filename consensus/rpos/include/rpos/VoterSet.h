#pragma once

#include <vector>
#include <core/types.h>
#include <lightclient/BlsVoterSet.h>

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

        BlsVoterSet toBlsVoterSet() const;
    };
}