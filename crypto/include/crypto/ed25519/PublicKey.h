#pragma once

#include <string>

#include <ed25519/Digest.h>
#include <ed25519/Signature.h>

namespace ed25519 {
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

        bool valid() const;

    private:
        bool getRaw(uint8_t* rawKey, size_t len) const;
        std::string m_key;
    };
}