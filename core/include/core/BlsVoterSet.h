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

        bool valid() const;

        BlsVoterSet();

        BlsVoterSet(const std::string& s);

        BlsVoterSet(const std::vector<char>& vc);

        std::string toString() const;

        std::vector<char> toVectorChar() const;

        bool operator == (const BlsVoterSet&) const;

    private:
        void init(const std::string& s);

        void toStringStream(std::stringstream&) const;

        bool fromStringStream(std::stringstream&);
    };
}

#ifdef CONSENSUS_VRF
FC_REFLECT( ultrainio::BlsVoterSet, (commonEchoMsg)(accountPool)(proofPool)(sigX))
#else
FC_REFLECT( ultrainio::BlsVoterSet, (commonEchoMsg)(accountPool)(sigX))
#endif
