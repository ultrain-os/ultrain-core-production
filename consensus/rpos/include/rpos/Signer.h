#pragma once

#include <fc/crypto/sha256.hpp>
#include <crypto/Digest.h>
#include <crypto/PrivateKey.h>
#include <crypto/Signature.h>

namespace ultrainio {
    class Signer {
    public:
        template <class T>
        static Signature sign(const T& v, const PrivateKey& privateKey) {
            fc::sha256 h = fc::sha256::hash(v);
            Digest digest(h.str());
            return privateKey.sign(digest);
        }
    };
}