#pragma once

#include <string>

namespace ultrainio {
    class Signature {
    public:
        Signature();
        explicit Signature(const std::string& s);
        Signature(const unsigned char* s, size_t len);
        Signature(const Signature& rhs) = default;
        Signature&operator = (const Signature& rhs) = default;
        explicit operator std::string() const;
        bool isValid() const;

    private:
        bool getRaw(unsigned char* rawKey, size_t len) const;
        std::string m_sig;

        friend class PublicKey;
        friend class Proof;
    };
}