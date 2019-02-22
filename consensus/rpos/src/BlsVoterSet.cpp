#include <rpos/BlsVoterSet.h>

#include <fc/io/json.hpp>
#include <base/Hex.h>
#include <base/Memory.h>
#include <crypto/Bls.h>

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

    // class VoterSet
    bool VoterSet::empty() {
        return accountPool.empty();
    }

    BlsVoterSet VoterSet::toBlsVoterSet() const {
        BlsVoterSet blsVoterSet;
        blsVoterSet.commonEchoMsg = this->commonEchoMsg;
        blsVoterSet.accountPool = this->accountPool;
#ifdef CONSENSUS_VRF
        blsVoterSet.proofPool = this->proofPool;
#endif
        std::shared_ptr<Bls> blsPtr = Bls::getDefault();
        int n = blsVoterSet.accountPool.size();
        unsigned char** blsSignV = (unsigned char**)malloc(n * sizeof(unsigned char*));
        for (int i = 0; i < n; i++) {
            blsSignV[i] = (unsigned char*)malloc(Bls::BLS_SIGNATURE_COMPRESSED_LENGTH);
            Hex::fromHex(this->blsSignPool[i], blsSignV[i], Bls::BLS_SIGNATURE_COMPRESSED_LENGTH);
        }
        unsigned char sigX[Bls::BLS_SIGNATURE_COMPRESSED_LENGTH];
        bool res = blsPtr->aggregate(blsSignV, n, sigX, Bls::BLS_SIGNATURE_COMPRESSED_LENGTH);
        Memory::freeMultiDim<unsigned char>(blsSignV, n);
        if (!res) {
            elog("aggregate bls error");
            return BlsVoterSet();
        }
        blsVoterSet.sigX = Hex::toHex(sigX, Bls::BLS_SIGNATURE_COMPRESSED_LENGTH);
        return blsVoterSet;
    }
}