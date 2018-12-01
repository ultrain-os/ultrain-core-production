#include "crypto/PrivateKey.h"

#include <base/Hex.h>
#include <crypto/Ed25519.h>

namespace ultrainio {
    PrivateKey PrivateKey::generate() {
        uint8_t pk[Ed25519::PUBLIC_KEY_LEN];
        uint8_t sk[Ed25519::PRIVATE_KEY_LEN];
        Ed25519::keypair(pk, sk);
        return PrivateKey(sk, Ed25519::PRIVATE_KEY_LEN);
    }

    bool PrivateKey::verifyKeyPair(const PublicKey& publicKey, const PrivateKey& privateKey) {
        Digest digest("");
        if (!privateKey.isValid() || !publicKey.isValid()) {
            return false;
        }
        return publicKey.verify(privateKey.sign(digest), digest);
    }

    PrivateKey::PrivateKey(const std::string& key) : m_key(key) {
        m_publicKey = PublicKey(std::string(m_key, Ed25519::PRIVATE_KEY_HEX_LEN - Ed25519::PUBLIC_KEY_HEX_LEN));
    }

    PrivateKey::PrivateKey(uint8_t* rawKey, size_t len) : m_key(Hex::toHex(rawKey, len)) {
        m_publicKey = PublicKey(std::string(m_key, Ed25519::PRIVATE_KEY_HEX_LEN - Ed25519::PUBLIC_KEY_HEX_LEN));
    }

    PrivateKey::operator std::string() const {
        return m_key;
    }

    Signature PrivateKey::sign(const Digest& digest) const {
        std::string digestStr = std::string(digest);
        uint8_t rawKey[Ed25519::PRIVATE_KEY_LEN];
        if (!getRaw(rawKey, Ed25519::SIGNATURE_LEN)) {
            return Signature();
        }
        uint8_t sig[Ed25519::SIGNATURE_LEN];
        if (!Ed25519::sign(sig, (const uint8_t*)digestStr.c_str(), digestStr.length(), rawKey)) {
            return Signature();
        }
        return Signature(sig, Ed25519::SIGNATURE_LEN);
    }

    bool PrivateKey::getRaw(uint8_t* rawKey, size_t len) const {
        return Hex::fromHex(m_key, rawKey, len) == Ed25519::PRIVATE_KEY_LEN;
    }

    // maybe more condition check
    bool PrivateKey::isValid() const {
        if (m_key.length() == Ed25519::PRIVATE_KEY_HEX_LEN && m_publicKey.isValid()) {
            return true;
        }
        return false;
    }

    PublicKey PrivateKey::getPublicKey() const {
        return m_publicKey;
    }
}