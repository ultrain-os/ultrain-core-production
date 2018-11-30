#include "crypto/Signature.h"

#include <base/Hex.h>
#include <crypto/Bls.h>

namespace ultrainio {
    Signature::Signature() {}

    Signature::Signature(const std::string& sig) : m_sig(sig) {}

    Signature::Signature(const unsigned char* sig, size_t len) : m_sig(Hex::toHex<unsigned char>(sig, len)) {}

    Signature::operator std::string() const {
        return m_sig;
    }

    bool Signature::isValid() const {
        return m_sig.length() == 2 * Bls::BLS_SIGNATURE_LENGTH;
    }

    bool Signature::getRaw(unsigned char* rawKey, size_t len) const {
        if (len < Bls::BLS_SIGNATURE_LENGTH) {
            return false;
        }
        return Hex::fromHex<unsigned char>(m_sig, rawKey, len) == Bls::BLS_SIGNATURE_LENGTH;
    }
}