#include <rpos/BlsVoterSet.h>

namespace ultrainio {
    // BlsVoterSet
    bool BlsVoterSet::empty() {
        return accountPool.empty();
    }

    void BlsVoterSet::toVariants(fc::variants& v) const {
        commonEchoMsg.toVariants(v);
        uint32_t n = accountPool.size();
        v.push_back(fc::variant(n));
        for (int i = 0; i < n; i++) {
            v.push_back(fc::variant(std::string(accountPool[i])));
        }
#ifdef CONSENSUS_VRF
        for (int i = 0; i < n; i++) {
            v.push_back(fc::variant(std::string(proofPool[i])));
        }
#endif
        v.push_back(fc::variant(sigX));
    }

    void BlsVoterSet::fromVariants(const fc::variants& v) {
        int nextIndex = commonEchoMsg.fromVariants(v);
        uint32_t n = v[nextIndex++].as<uint32_t>();
        for (int i = 0; i < n; i++) {
            accountPool.push_back(v[nextIndex++].as_string());
        }
#ifdef CONSENSUS_VRF
        for (int i = 0; i < n; i++) {
            proofPool.push_back(v[nextIndex++].as_string());
        }
#endif
        // sigX
        sigX = v[nextIndex].as_string();
    }

    bool BlsVoterSet::operator == (const BlsVoterSet& rhs) {
        if (this == &rhs) {
            return true;
        }
        if (commonEchoMsg == rhs.commonEchoMsg
            && accountPool == rhs.accountPool
            #ifdef CONSENSUS_VRF
            && proofPool == rhs.proofPool
            #endif
            && sigX == rhs.sigX) {
            return true;
        }
        return false;
    }

    // class VoterSet
    bool VoterSet::empty() {
        return accountPool.empty();
    }
}