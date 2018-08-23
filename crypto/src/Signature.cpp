#include "crypto/Signature.h"

#include <boringssl/curve25519.h>
#include <base/Hex.h>

namespace ultrainio {
    Signature::Signature() {}

    Signature::Signature(const std::string& sig) : m_sig(sig) {}

    Signature::Signature(const uint8_t* sig, size_t len) : m_sig(Hex::toHex(sig, len)) {}

    Signature::operator std::string() const {
        return m_sig;
    }

    bool Signature::getRaw(uint8_t* rawKey, size_t len) const {
        return Hex::fromHex(m_sig, rawKey, len) == ED25519_SIGNATURE_LEN;
    }
}