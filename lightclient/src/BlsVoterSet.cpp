#include <lightclient/BlsVoterSet.h>

#include <fc/io/json.hpp>

namespace ultrainio {
    // BlsVoterSet
    BlsVoterSet::BlsVoterSet() {}

    BlsVoterSet::BlsVoterSet(const std::string& s) {
        init(s);
    }

    BlsVoterSet::BlsVoterSet(const std::vector<char>& vc) {
        std::string s;
        s.assign(vc.begin(), vc.end());
        init(s);
    }

    void BlsVoterSet::init(const std::string& s) {
        fc::variant v = fc::json::from_string(s);
        fc::variants vs = v.get_array();
        this->fromVariants(vs);
    }

    std::string BlsVoterSet::toString() const {
        fc::variants va;
        this->toVariants(va);
        return fc::json::to_string(va);
    }

    std::vector<char> BlsVoterSet::toVectorChar() const {
        std::string s = this->toString();
        std::vector<char> v(s.size());
        v.assign(s.begin(), s.end());
        return v;
    }

    bool BlsVoterSet::empty() const {
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

    bool BlsVoterSet::operator == (const BlsVoterSet& rhs) const {
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
}