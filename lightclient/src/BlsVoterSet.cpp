#include <lightclient/BlsVoterSet.h>

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
        std::stringstream ss(s);
        if (!fromStringStream(ss)) {
            accountPool.clear();
        }
    }

    std::string BlsVoterSet::toString() const {
        std::stringstream ss;
        toStringStream(ss);
        return ss.str();
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

    void BlsVoterSet::toStringStream(std::stringstream& ss) const {
        commonEchoMsg.toStringStream(ss);
        int n = accountPool.size();
        ss << n << " ";
        for (int i = 0; i < n; i++) {
            ss << std::string(accountPool[i]) << " ";
        }
#ifdef CONSENSUS_VRF
        for (int i = 0; i < n; i++) {
            ss << proofPool[i] << " ";
        }
#endif
        ss << sigX << " ";
    }

    bool BlsVoterSet::fromStringStream(std::stringstream& ss) {
        if (!commonEchoMsg.fromStringStream(ss)) {
            return false;
        }
        int n = 0;
        if (!(ss >> n)) {
            return false;
        }
        std::string item;
        for (int i = 0; i < n; i++) {
            if (!(ss >> item)) {
                return false;
            }
            accountPool.push_back(item);
        }
#ifdef CONSENSUS_VRF
        for (int i = 0; i < n; i++) {
            if (!(ss >> item)) {
                return false;
            }
            proofPool.push_back(item)
        }
#endif
        if (!(ss >> sigX)) {
            return false;
        }
        return true;
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