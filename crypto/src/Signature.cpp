#include "crypto/Signature.h"

#include <base/Hex.h>
#include <crypto/Ed25519.h>

namespace ultrainio {
    Signature::Signature() {}

    Signature::Signature(const std::string& sig) : m_sig(sig) {}

    Signature::Signature(const uint8_t* sig, size_t len) : m_sig(Hex::toHex(sig, len)) {}

    Signature::operator std::string() const {
        return m_sig;
    }

    bool Signature::isValid() const {
        return m_sig.length() == Ed25519::SIGNATURE_HEX_LEN;
    }

    bool Signature::getRaw(uint8_t* rawKey, size_t len) const {
        return Hex::fromHex(m_sig, rawKey, len) == Ed25519::SIGNATURE_LEN;
    }
}