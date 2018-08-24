#include "crypto/PublicKey.h"

#include <base/Hex.h>
#include <crypto/Ed25519.h>

namespace ultrainio {
    PublicKey::PublicKey(const std::string& key) : m_key(key) {}

    PublicKey::PublicKey(uint8_t* rawKey, size_t len) : m_key(Hex::toHex(rawKey, len)) {}

    bool PublicKey::operator == (const PublicKey& rhs) {
        return m_key == rhs.m_key;
    }

    PublicKey::operator std::string() const {
        return m_key;
    }

    bool PublicKey::verify(const Signature& signature, const Digest& digest) const {
        if (!isValid()) {
            return false;
        }
        uint8_t pk[Ed25519::PUBLIC_KEY_LEN];
        if (!getRaw(pk, Ed25519::PUBLIC_KEY_LEN)) {
            return false;
        }
        uint8_t sigUint8[Ed25519::SIGNATURE_LEN];
        if (!signature.getRaw(sigUint8, Ed25519::SIGNATURE_LEN)) {
            return false;
        }
        std::string h = std::string(digest);
        if (Ed25519::verify((const uint8_t*)h.c_str(), h.length(), sigUint8, pk)) {
            return true;
        }
        return false;
    }

    bool PublicKey::getRaw(uint8_t* rawKey, size_t len) const {
        return Hex::fromHex(m_key, rawKey, len) == Ed25519::PUBLIC_KEY_LEN;
    }

    bool PublicKey::isValid() const {
        if (m_key.length() == 2 * Ed25519::PUBLIC_KEY_LEN) {
            return true;
        }
        return false;
    }
}