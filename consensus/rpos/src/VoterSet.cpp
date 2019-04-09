#include <rpos/VoterSet.h>

#include <base/Hex.h>
#include <base/Memory.h>
#include <crypto/Bls.h>

namespace ultrainio {
    // class VoterSet
    bool VoterSet::empty() const {
        if (accountPool.empty()) {
            return true;
        }
        if (accountPool.size() != blsSignPool.size()) {
            return true;
        }
        return false;
    }

    BlsVoterSet VoterSet::toBlsVoterSet(int weight) const {
        if (empty()) {
            return BlsVoterSet();
        }
        BlsVoterSet blsVoterSet;
        size_t N = this->accountPool.size() > weight ? weight : this->accountPool.size();
        blsVoterSet.commonEchoMsg = this->commonEchoMsg;
        for (int i = 0; i < N; i++) {
            blsVoterSet.accountPool.push_back(this->accountPool[i]);
        }
#ifdef CONSENSUS_VRF
        for (int i = 0; i < N; i++) {
            blsVoterSet.proofPool.push_back(this->proofPool[i]);
        }
#endif
        std::vector<std::string> tmpBlsSignPool;
        for (int i = 0; i < N; i++) {
            tmpBlsSignPool.push_back(this->blsSignPool[i]);
        }
        blsVoterSet.sigX = generateSigX(tmpBlsSignPool);
        return blsVoterSet;
    }

    std::string VoterSet::generateSigX(const std::vector<std::string>& aggBlsSignPool) const {
        std::shared_ptr<Bls> blsPtr = Bls::getDefault();
        int n = aggBlsSignPool.size();
        unsigned char** blsSignV = (unsigned char**)malloc(n * sizeof(unsigned char*));
        for (int i = 0; i < n; i++) {
            blsSignV[i] = (unsigned char*)malloc(Bls::BLS_SIGNATURE_COMPRESSED_LENGTH);
            Hex::fromHex<unsigned char>(aggBlsSignPool[i], blsSignV[i], Bls::BLS_SIGNATURE_COMPRESSED_LENGTH);
        }
        unsigned char sigX[Bls::BLS_SIGNATURE_COMPRESSED_LENGTH];
        bool res = blsPtr->aggregate(blsSignV, n, sigX, Bls::BLS_SIGNATURE_COMPRESSED_LENGTH);
        Memory::freeMultiDim<unsigned char>(blsSignV, n);
        if (!res) {
            elog("aggregate bls error");
            return std::string();
        }
        return Hex::toHex(sigX, Bls::BLS_SIGNATURE_COMPRESSED_LENGTH);
    }
}