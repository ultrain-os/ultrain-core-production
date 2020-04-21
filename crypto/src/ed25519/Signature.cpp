#include "ed25519/Signature.h"

#include <base/Hex.h>

namespace ed25519 {
    Signature::Signature() {}

    Signature::Signature(const std::string& sig) : m_sig(sig) {}

    Signature::Signature(const uint8_t* sig, size_t len) : m_sig(ultrainio::Hex::toHex(sig, len)) {}

    Signature::operator std::string() const {
        return m_sig;
    }

    bool Signature::isValid() const {
        // loose check
        return m_sig.length() != 0;
    }

    bool Signature::getRaw(uint8_t* rawKey, size_t len) const {
        if (rawKey == nullptr || len <= 0) {
            return false;
        }
        ultrainio::Hex::fromHex<uint8_t>(m_sig, rawKey, len);
        return true;
    }
}