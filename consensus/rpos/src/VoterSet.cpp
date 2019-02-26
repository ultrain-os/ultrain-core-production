#include <rpos/VoterSet.h>

#include <base/Hex.h>
#include <base/Memory.h>
#include <crypto/Bls.h>

namespace ultrainio {
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