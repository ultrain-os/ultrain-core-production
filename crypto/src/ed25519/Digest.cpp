#include "ed25519/Digest.h"

namespace ed25519 {
    Digest::Digest(const std::string& h) : m_h(h) {}

    Digest::operator std::string() const {
        return m_h;
    }
}