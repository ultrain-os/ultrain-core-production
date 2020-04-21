#pragma once

#include <fc/crypto/sha256.hpp>

#include <base/Hex.h>
#include <core/types.h>
#include <crypto/Bls.h>
#include <ed25519/Digest.h>

namespace ultrainio {
    class Signer {
    public:
        template <class T>
        static consensus::SignatureType sign(const T& v, const consensus::PrivateKeyType& privateKey) {
            fc::sha256 h = fc::sha256::hash(v);
#ifdef ULTRAIN_CONSENSUS_SUPPORT_GM
            return privateKey.sign(h);
#else
            ed25519::Digest digest(h.str());
            return privateKey.sign(digest);
#endif
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