#pragma once

#include <string>

namespace ed25519 {
    class Signature {
    public:
        Signature();
        explicit Signature(const std::string& s);
        Signature(const uint8_t* s, size_t len);
        Signature(const Signature& rhs) = default;
        Signature&operator = (const Signature& rhs) = default;
        explicit operator std::string() const;
        bool isValid() const;
        bool getRaw(uint8_t* rawKey, size_t len) const;

    private:
        std::string m_sig;
    };
}