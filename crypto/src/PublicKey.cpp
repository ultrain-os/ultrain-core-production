#include <crypto/PublicKey.h>

#include <base/Hex.h>
#include <crypto/Bls.h>

namespace ultrainio {
    PublicKey::PublicKey(const std::string& key) : m_key(key) {}

    PublicKey::PublicKey(unsigned char* rawKey, size_t len) : m_key(Hex::toHex<unsigned char>(rawKey, len)) {}

    bool PublicKey::operator == (const PublicKey& rhs) const {
        return m_key == rhs.m_key;
    }

    PublicKey::operator std::string() const {
        return m_key;
    }

    bool PublicKey::verify(const Signature& signature, const Digest& digest) const {
        if (!isValid()) {
            return false;
        }
        unsigned char pk[Bls::BLS_PUB_KEY_LENGTH];
        if (!getRaw(pk, Bls::BLS_PUB_KEY_LENGTH)) {
            return false;
        }
        unsigned char sigStr[Bls::BLS_SIGNATURE_LENGTH];
        if (!signature.getRaw(sigStr, Bls::BLS_SIGNATURE_LENGTH)) {
            return false;
        }
        std::string h = std::string(digest);
        if (Bls::getDefault()->verify(pk, sigStr, (void*)h.c_str(), h.length())) {
            return true;
        }
        return false;
    }

    bool PublicKey::getRaw(unsigned char* rawKey, size_t len) const {
        if (len < Bls::BLS_PUB_KEY_LENGTH) {
            return false;
        }
        return Hex::fromHex<unsigned char>(m_key, rawKey, len) == Bls::BLS_PUB_KEY_LENGTH;
    }

    bool PublicKey::isValid() const {
        if (m_key.length() == 2 * Bls::BLS_PUB_KEY_LENGTH) {
            return true;
        }
        return false;
    }
}