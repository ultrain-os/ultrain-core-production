#include "crypto/PrivateKey.h"

#include <boringssl/curve25519.h>
#include <base/Hex.h>

namespace ultrainio {
    PrivateKey PrivateKey::generate() {
        uint8_t pk[ED25519_PUBLIC_KEY_LEN];
        uint8_t sk[ED25519_PRIVATE_KEY_LEN];
        ED25519_keypair(pk, sk);
        PublicKey publicKey(pk, ED25519_PUBLIC_KEY_LEN);
        return PrivateKey(sk, ED25519_PRIVATE_KEY_LEN, publicKey);
    }

    // TODO(qinxiaofen) to generator public key when pass an default public key
    PrivateKey::PrivateKey(const std::string& key, const PublicKey& publicKey) : m_key(key), m_publicKey(publicKey) {}

    // TODO(qinxiaofen) to generator public key when pass an default public key
    PrivateKey::PrivateKey(uint8_t* rawKey, size_t len, const PublicKey& publicKey) : m_key(Hex::toHex(rawKey, len)), m_publicKey(publicKey) {}

    PrivateKey::operator std::string() const {
        return m_key;
    }

    Signature PrivateKey::sign(const Digest& digest) const {
        std::string digestStr = std::string(digest);
        uint8_t rawKey[ED25519_PRIVATE_KEY_LEN];
        if (!getRaw(rawKey, ED25519_SIGNATURE_LEN)) {
            return Signature();
        }
        uint8_t sig[ED25519_SIGNATURE_LEN];
        if (!ED25519_sign(sig, (const uint8_t*)digestStr.c_str(), digestStr.length(), rawKey)) {
            return Signature();
        }
        return Signature(sig, ED25519_SIGNATURE_LEN);
    }

    bool PrivateKey::getRaw(uint8_t* rawKey, size_t len) const {
        return Hex::fromHex(m_key, rawKey, len) == ED25519_PRIVATE_KEY_LEN;
    }

    // maybe more condition check
    bool PrivateKey::isValid() {
        if (m_key.length() == 2 * ED25519_PRIVATE_KEY_LEN && m_publicKey.isValid()) {
            return true;
        }
        return false;
    }

    PublicKey PrivateKey::getPublicKey() const {
        return m_publicKey;
    }
}