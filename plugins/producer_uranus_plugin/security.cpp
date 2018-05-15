//
// Created by 秦晓分 on 2018/4/28.
//

#include <ultrainio/producer_uranus_plugin/security.hpp>

#include <boringssl/curve25519.h>
#include <openssl/sha.h>

namespace ultrain {
    const size_t security::VRF_PRIVATE_KEY_LEN = 64;

    const size_t security::VRF_PUBLIC_KEY_LEN = 32;

    const size_t security::VRF_PROOF_LEN = 64;

    const size_t security::SHA256_DIGEST_LEN = 32;

    int security::vrf_keypair(uint8_t out_public_key[32], uint8_t out_private_key[64]) {
        ED25519_keypair(out_public_key, out_private_key);
        return 0;
    }

    int security::vrf_prove(uint8_t proof[64], const uint8_t* message, size_t message_len, const uint8_t sk[64]) {
        return !ED25519_sign(proof, message, message_len, sk);
    }

    int security::vrf_proof2priority(uint64_t* priority, const uint8_t proof[64]) {
        // 193:256 bit
        if (priority == nullptr || proof == nullptr) {
            return 1;
        }
        *priority = 0;
        size_t start_index = 24;
        size_t byte_num = 8;
        for (size_t i = 0; i < byte_num; i++) {
            *priority += proof[start_index + i];
            if (i != byte_num - 1) {
                *priority = *priority << 8;
            }
        }
        return 0;
    }

    int security::vrf_verify(const uint8_t* message, size_t message_len, const uint8_t proof[64],
                   const uint8_t public_key[32]) {
        return !ED25519_verify(message, message_len, proof, public_key);
    }

    int security::sha256(const uint8_t* data, size_t len, uint8_t* out) {
        SHA256(data, len, out);
        return 0;
    }
}
