#include "crypto/Digest.h"

namespace ultrainio {
    Digest::Digest(const std::string& h) : m_h(h) {}

    Digest::operator std::string() const {
        return m_h;
    }
}