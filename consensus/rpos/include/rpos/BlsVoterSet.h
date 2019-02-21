#pragma once

#include <string>
#include <core/Message.h>

namespace ultrainio {
    struct BlsVoterSet {
        CommonEchoMsg commonEchoMsg;
        std::vector<AccountName> accountPool;
#ifdef CONSENSUS_VRF
        std::vector<std::string> proofPool;
#endif
        std::string sigX;

        bool empty();

        BlsVoterSet();

        BlsVoterSet(const std::string& s);

        //BlsVoterSet(const std::vector<char>& vc);

        std::string toString() const;

        std::vector<char> toVectorChar() const;

        bool operator == (const BlsVoterSet&) const;

    private:
        void toVariants(fc::variants&) const;

        void fromVariants(const fc::variants&);
    };

    struct VoterSet {
        CommonEchoMsg commonEchoMsg;
        std::vector<AccountName> accountPool;
        std::vector<std::string> sigPool;
        std::vector<std::string> blsSignPool;
        std::vector<uint32_t>    timePool;
#ifdef CONSENSUS_VRF
        std::vector<std::string> proofPool;
#endif
        bool empty();
    };
}
