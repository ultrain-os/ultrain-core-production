//
// Created by 秦晓分 on 2018/4/28.
//

#include "security.hpp"

#include <boringssl/curve25519.h>
#include <openssl/sha.h>

namespace ultrainio {
    bool security::vrf_keypair(uint8_t out_public_key[32], uint8_t out_private_key[64]) {
        ED25519_keypair(out_public_key, out_private_key);
        return true;
    }

    bool security::vrf_prove(uint8_t proof[64], const uint8_t* message, size_t message_len, const uint8_t sk[64]) {
        if (ED25519_sign(proof, message, message_len, sk)) {
            return true;
        }
        return false;
    }

    bool security::vrf_verify(const uint8_t* message, size_t message_len, const uint8_t proof[64],
                   const uint8_t public_key[32]) {
        if (ED25519_verify(message, message_len, proof, public_key)) {
            return true;
        }
        return false;
    }

    bool security::sha256(const uint8_t* data, size_t len, uint8_t* out) {
        SHA256(data, len, out);
        return true;
    }
}
