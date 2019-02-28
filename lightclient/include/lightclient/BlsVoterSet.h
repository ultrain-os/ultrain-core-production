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

        bool empty() const;

        BlsVoterSet();

        BlsVoterSet(const std::string& s);

        BlsVoterSet(const std::vector<char>& vc);

        std::string toString() const;

        std::vector<char> toVectorChar() const;

        bool operator == (const BlsVoterSet&) const;

    private:
        void init(const std::string& s);

        void toVariants(fc::variants&) const;

        void fromVariants(const fc::variants&);
    };
}
