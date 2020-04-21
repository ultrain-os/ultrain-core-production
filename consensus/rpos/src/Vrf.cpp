#include "rpos/Vrf.h"

#include <rpos/Proof.h>
#include <rpos/Seed.h>

namespace ultrainio {
    const int Vrf::kProposer = 1;

    const int Vrf::kVoter = 2;

    Proof Vrf::vrf(const consensus::PrivateKeyType& privateKey, const Seed& seed, int role) {
        //TODO(xiaofen.qin@gmailcom
        return Proof();
//        Digest digest(std::string(seed) + std::to_string(role));
//        if (!privateKey.isValid()) {
//            return Proof();
//        }
//        return Proof(privateKey.sign(digest));
    }

    bool Vrf::verify(const consensus::PublicKeyType& publicKey, const Proof& proof, const Seed& seed, int role) {
        //TODO(xiaofen.qin@gmailcom
        return true;
//        Digest digest(std::string(seed) + std::to_string(role));
//        if (!publicKey.isValid()) {
//            return false;
//        }
//        SignatureType signature = proof.getSignature();
//        if (!signature.isValid()) {
//            return false;
//        }
//        return publicKey.verify(signature, digest);
    }
}