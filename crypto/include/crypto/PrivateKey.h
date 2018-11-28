#pragma once

#include <string>

#include <crypto/Digest.h>
#include <crypto/PublicKey.h>
#include <crypto/Signature.h>

namespace ultrainio {
    class PrivateKey {
    public:
        static bool generate(PrivateKey& sk, PublicKey& pk);

        static bool verifyKeyPair(const PublicKey& publicKey, const PrivateKey& privateKey);

        PrivateKey() = default;
        // hex string
        explicit PrivateKey(const std::string& key);

        PrivateKey(unsigned char* rawKey, size_t len);

        PrivateKey(const PrivateKey& rhs) = default;

        PrivateKey&operator = (const PrivateKey& rhs) = default;

        explicit operator std::string() const;

        Signature sign(const Digest& digest) const;

        PublicKey getPublicKey() const;

        bool isValid() const;

    private:
        bool getRaw(unsigned char* rawKey, size_t len) const;

        std::string m_key;
    };
}