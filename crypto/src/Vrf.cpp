#include "crypto/Vrf.h"

#include <boringssl/curve25519.h>

namespace ultrainio {
    bool Vrf::keypair(uint8_t out_public_key[32], uint8_t out_private_key[64]) {
        ED25519_keypair(out_public_key, out_private_key);
        return true;
    }

    bool Vrf::prove(uint8_t proof[64], const uint8_t* message, size_t message_len, const uint8_t sk[64]) {
        if (ED25519_sign(proof, message, message_len, sk)) {
            return true;
        }
        return false;
    }

    bool Vrf::verify(const uint8_t* message, size_t message_len, const uint8_t proof[64],
                   const uint8_t public_key[32]) {
        if (ED25519_verify(message, message_len, proof, public_key)) {
            return true;
        }
        return false;
    }
}
