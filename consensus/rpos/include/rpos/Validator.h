#pragma once

#include <fc/crypto/sha256.hpp>

#include <base/Hex.h>
#include <crypto/Bls.h>
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

        template <class T>
        static bool verify(const std::string& signature, const T& v, unsigned char* pk) {
            fc::sha256 h = fc::sha256::hash(v);
            std::shared_ptr<Bls> blsPtr = Bls::getDefault();
            unsigned char sign[Bls::BLS_SIGNATURE_COMPRESSED_LENGTH];
            Hex::fromHex<unsigned char>(signature, sign, Bls::BLS_SIGNATURE_COMPRESSED_LENGTH);
            return blsPtr->verify(pk, sign, (void*)h.str().c_str(), h.str().length());
        }

        template <class T>
        static bool verify(const std::string& sig, const T& v, unsigned char** pks, int size) {
            fc::sha256 h = fc::sha256::hash(v);
            std::shared_ptr<Bls> blsPtr = Bls::getDefault();
            unsigned char sigX[Bls::BLS_SIGNATURE_COMPRESSED_LENGTH];
            Hex::fromHex<unsigned char>(sig, sigX, Bls::BLS_SIGNATURE_COMPRESSED_LENGTH);
            return blsPtr->verifyAggregate(pks, size, sigX, Bls::BLS_SIGNATURE_COMPRESSED_LENGTH, (void*)h.str().c_str(), h.str().length());
        }
    };
}