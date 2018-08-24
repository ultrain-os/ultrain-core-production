#pragma once

#include <stddef.h>
#include <stdint.h>

namespace ultrainio {

    /**
     * Wrapper class for ed25519
     */
    class Ed25519 {
    public:
        static const int PRIVATE_KEY_LEN;

        static const int PUBLIC_KEY_LEN;

        static const int SIGNATURE_LEN;

        static bool keypair(uint8_t* outPublicKey, uint8_t* outPrivateKey);

        static bool sign(uint8_t* outSign, const uint8_t* message, size_t messageLen, const uint8_t* privateKey);

        static bool verify(const uint8_t* message, size_t messageLen, const uint8_t* outSign, const uint8_t* publicKey);
    };
} // namespace ultrainio
