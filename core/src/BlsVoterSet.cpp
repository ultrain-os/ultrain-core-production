#include <core/BlsVoterSet.h>

#include <base/Hex.h>
#include <base/Memory.h>
#include <crypto/Bls.h>

namespace ultrainio {
    template <class T>
    static bool verify(const std::string& sig, const T& v, unsigned char** pks, int size) {
        fc::sha256 h = fc::sha256::hash(v);
        std::shared_ptr<Bls> blsPtr = Bls::getDefault();
        unsigned char sigX[Bls::BLS_SIGNATURE_COMPRESSED_LENGTH];
        Hex::fromHex<unsigned char>(sig, sigX, Bls::BLS_SIGNATURE_COMPRESSED_LENGTH);
        return blsPtr->verifyAggregate(pks, size, sigX, Bls::BLS_SIGNATURE_COMPRESSED_LENGTH, (void*)h.str().c_str(), h.str().length());
    }

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

    bool BlsVoterSet::valid() const {
        return !accountPool.empty();
    }

    bool BlsVoterSet::verifyBls(std::vector<std::string> blsPkVector) const {
        if (blsPkVector.size() != accountPool.size()) {
            ilog("size not equal");
            return false;
        }
        unsigned char** pks = (unsigned char**)malloc(sizeof(unsigned char*) * accountPool.size());
        for (int i = 0; i < accountPool.size(); i++) {
            pks[i] = (unsigned char*)malloc(sizeof(unsigned char) * Bls::BLS_PUB_KEY_COMPRESSED_LENGTH);
            Hex::fromHex<unsigned char>(blsPkVector[i], pks[i], Bls::BLS_PUB_KEY_COMPRESSED_LENGTH);
        }
        bool res = verify(sigX, commonEchoMsg, pks, accountPool.size());
        Memory::freeMultiDim<unsigned char>(pks, accountPool.size());
        return res;
    }

    void BlsVoterSet::toStringStream(std::stringstream& ss) const {
        if (!valid()) {
            ss << std::string();
            return;
        }
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