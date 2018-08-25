#pragma once

#include <fc/crypto/sha256.hpp>
#include <crypto/Digest.h>
#include <crypto/PublicKey.h>
#include <crypto/Signature.h>

namespace ultrainio {
    class Validator {
    public:
        template <class T>
        static bool verify(const Signature& signature, const T& v, const PublicKey& publicKey) {
            fc::sha256 h = fc::sha256::hash(v);
            Digest digest(h.str());
            return publicKey.verify(signature, digest);
        }
    };
}