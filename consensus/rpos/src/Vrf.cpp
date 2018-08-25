#include "rpos/Vrf.h"

#include <crypto/PrivateKey.h>
#include <crypto/PublicKey.h>
#include <rpos/Proof.h>
#include <rpos/Seed.h>

namespace ultrainio {
    const int Vrf::kProposer = 1;

    const int Vrf::kVoter = 2;

    Proof Vrf::vrf(const PrivateKey& privateKey, const Seed& seed, int role) {
        Digest digest(std::string(seed) + std::to_string(role));
        if (!privateKey.isValid()) {
            return Proof();
        }
        return Proof(privateKey.sign(digest));
    }

    bool Vrf::verify(const PublicKey& publicKey, const Proof& proof, const Seed& seed, int role) {
        Digest digest(std::string(seed) + std::to_string(role));
        if (!publicKey.isValid()) {
            return false;
        }
        Signature signature = proof.getSignature();
        if (!signature.isValid()) {
            return false;
        }
        return publicKey.verify(signature, digest);
    }
}