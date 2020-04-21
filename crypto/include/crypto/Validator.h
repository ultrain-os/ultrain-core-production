#pragma once

#include <fc/crypto/sha256.hpp>

#include <base/Hex.h>
#include <core/types.h>
#include <crypto/Bls.h>
#include <ed25519/Digest.h>

namespace ultrainio {
    class Validator {
    public:
        template <class T>
        static bool verify(const consensus::SignatureType& signature, const T& v, const consensus::PublicKeyType& publicKey) {
            fc::sha256 h = fc::sha256::hash(v);
#ifdef ULTRAIN_CONSENSUS_SUPPORT_GM
            return publicKey.verify(h.data(), h.data_size(), signature);
#else
            ed25519::Digest digest(h.str());
            return publicKey.verify(signature, digest);
#endif
        }

        template <class T>
        static bool verify(const std::string& signature, const T& v, unsigned char* pk) {
            fc::sha256 h = fc::sha256::hash(v);
            std::shared_ptr<Bls> blsPtr = Bls::getDefault();
            unsigned char sign[Bls::BLS_SIGNATURE_COMPRESSED_LENGTH];
            Hex::fromHex<unsigned char>(signature, sign, Bls::BLS_SIGNATURE_COMPRESSED_LENGTH);
            return blsPtr->verify(pk, sign, (void*)h.str().c_str(), h.str().length());
        }
    };
}