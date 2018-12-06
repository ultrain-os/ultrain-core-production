#pragma once

#include <string>

#include <crypto/Digest.h>
#include <crypto/Signature.h>

namespace ultrainio {
    class PublicKey {
    public:
        PublicKey() = default;

        explicit PublicKey(const std::string& key);

        PublicKey(uint8_t* rawKey, size_t len);

        PublicKey(const PublicKey& rhs) = default;

        PublicKey&operator = (const PublicKey& rhs) = default;

        friend bool operator == (const PublicKey& lhs, const PublicKey& rhs);

        friend bool operator != (const PublicKey& lhs, const PublicKey& rhs);

        explicit operator std::string() const;

        bool verify(const Signature& signature, const Digest& digest) const;

        bool isValid() const;

    private:
        bool getRaw(uint8_t* rawKey, size_t len) const;
        std::string m_key;
    };
}