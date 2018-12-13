#include "crypto/Ed25519.h"

#include <boringssl/curve25519.h>

namespace ultrainio {
    const int Ed25519::PRIVATE_KEY_LEN = ED25519_PRIVATE_KEY_LEN;

    const int Ed25519::PUBLIC_KEY_LEN = ED25519_PUBLIC_KEY_LEN;

    const int Ed25519::SIGNATURE_LEN = ED25519_SIGNATURE_LEN;

    const int Ed25519::PRIVATE_KEY_HEX_LEN = 2 * ED25519_PRIVATE_KEY_LEN;

    const int Ed25519::PUBLIC_KEY_HEX_LEN = 2 * ED25519_PUBLIC_KEY_LEN;

    const int Ed25519::SIGNATURE_HEX_LEN = 2 * ED25519_SIGNATURE_LEN;

    bool Ed25519::keypair(uint8_t* outPublicKey, uint8_t* outPrivateKey) {
        ED25519_keypair_u(outPublicKey, outPrivateKey);
        return true;
    }

    bool Ed25519::sign(uint8_t* outSign, const uint8_t* message, size_t messageLen, const uint8_t* privateKey) {
        if (ED25519_sign_u(outSign, message, messageLen, privateKey)) {
            return true;
        }
        return false;
    }

    bool Ed25519::verify(const uint8_t* message, size_t messageLen, const uint8_t* outSign, const uint8_t* publicKey) {
        if (ED25519_verify_u(message, messageLen, outSign, publicKey)) {
            return true;
        }
        return false;
    }
}
