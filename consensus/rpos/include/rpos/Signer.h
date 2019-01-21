#pragma once

#include <fc/crypto/sha256.hpp>

#include <base/Hex.h>
#include <crypto/Bls.h>
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

        template <class T>
        static std::string sign(const T& v, unsigned char* sk) {
            fc::sha256 h = fc::sha256::hash(v);
            std::shared_ptr<Bls> blsPtr = Bls::getDefault();
            unsigned char signature[Bls::BLS_SIGNATURE_COMPRESSED_LENGTH];
            if (blsPtr->sign(sk, (void*)h.str().c_str(), h.str().length(), signature, Bls::BLS_SIGNATURE_COMPRESSED_LENGTH)) {
                return Hex::toHex<unsigned char>(signature, Bls::BLS_SIGNATURE_COMPRESSED_LENGTH);
            }
            return std::string();
        }
    };
}