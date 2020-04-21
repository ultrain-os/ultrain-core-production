#include "ed25519/PublicKey.h"

#include <base/Hex.h>
#include <ed25519/Ed25519.h>

namespace ed25519 {
    PublicKey::PublicKey(const std::string& key) : m_key(key) {}

    PublicKey::PublicKey(uint8_t* rawKey, size_t len) : m_key(ultrainio::Hex::toHex<uint8_t>(rawKey, len)) {}

    bool operator == (const PublicKey& lhs, const PublicKey& rhs) {
        return lhs.m_key == rhs.m_key;
    }

    bool operator != (const PublicKey& lhs, const PublicKey& rhs) {
        return lhs.m_key != rhs.m_key;
    }

    PublicKey::operator std::string() const {
        return m_key;
    }

    bool PublicKey::verify(const Signature& signature, const Digest& digest) const {
        if (!valid()) {
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
        return ultrainio::Hex::fromHex<uint8_t>(m_key, rawKey, len) == Ed25519::PUBLIC_KEY_LEN;
    }

    bool PublicKey::valid() const {
        if (m_key.length() == Ed25519::PUBLIC_KEY_HEX_LEN) {
            return true;
        }
        return false;
    }
}