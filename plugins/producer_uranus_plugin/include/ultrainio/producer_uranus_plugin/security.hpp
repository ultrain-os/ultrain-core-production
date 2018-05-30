#pragma once

#include <stddef.h>
#include <stdint.h>

namespace ultrainio {
    static const size_t VRF_PRIVATE_KEY_LEN    = 64;
    static const size_t VRF_PUBLIC_KEY_LEN     = 32;
    static const size_t VRF_PROOF_LEN          = 64;
    static const size_t SHA256_DIGEST_LEN      = 32;

    /**
     * security class.
     */
    class security {
    public:
        /**
         *
         * @param proof
         * @param message
         * @param message_len
         * @param sk private key
         * @return
         */
        static bool vrf_prove(uint8_t proof[64], const uint8_t* message, size_t message_len, const uint8_t sk[64]);

        /**
         * Verifiable Random Function verify function
         * @param message
         * @param message_len
         * @param proof
         * @param public_key
         * @return
         */
        static bool vrf_verify(const uint8_t* message, size_t message_len, const uint8_t proof[64],
                       const uint8_t public_key[32]);

        /**
         *
         * @param out_public_key
         * @param out_private_key
         * @return
         */
        static bool vrf_keypair(uint8_t out_public_key[32], uint8_t out_private_key[64]);

        static bool sha256(const uint8_t* data, size_t len, uint8_t* out);
    };
} // namespace ultrainio
