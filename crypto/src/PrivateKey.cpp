#include "crypto/PrivateKey.h"

#include <base/Hex.h>
#include <crypto/Bls.h>

namespace ultrainio {
    bool PrivateKey::generate(PrivateKey& sk, PublicKey& pk) {
        std::shared_ptr<Bls> blsPtr = Bls::getDefault();
        unsigned char skStr[Bls::BLS_PRI_KEY_LENGTH];
        unsigned char pkStr[Bls::BLS_PUB_KEY_LENGTH];
        blsPtr->keygen(skStr, Bls::BLS_PRI_KEY_LENGTH, pkStr, Bls::BLS_PUB_KEY_LENGTH);
        sk = PrivateKey(skStr, Bls::BLS_PRI_KEY_LENGTH);
        pk = PublicKey(pkStr, Bls::BLS_PUB_KEY_LENGTH);
        return true;
    }

    bool PrivateKey::verifyKeyPair(const PublicKey& publicKey, const PrivateKey& privateKey) {
        if (!privateKey.isValid() || !publicKey.isValid()) {
            return false;
        }
        return privateKey.getPublicKey() == publicKey;
    }

    PrivateKey::PrivateKey(const std::string& key) : m_key(key) {
    }

    PrivateKey::PrivateKey(unsigned char* rawKey, size_t len) : m_key(Hex::toHex<unsigned char>(rawKey, len)) {
    }

    PrivateKey::operator std::string() const {
        return m_key;
    }

    Signature PrivateKey::sign(const Digest& digest) const {
        std::string digestStr = std::string(digest);
        unsigned char rawKey[Bls::BLS_PRI_KEY_LENGTH];
        if (!getRaw(rawKey, Bls::BLS_PRI_KEY_LENGTH)) {
            return Signature();
        }
        unsigned char signature[Bls::BLS_SIGNATURE_LENGTH];
        std::shared_ptr<Bls> blsPtr = Bls::getDefault();
        if (!blsPtr->signature(rawKey, (void*)digestStr.c_str(), digestStr.length(), signature, Bls::BLS_SIGNATURE_LENGTH)) {
            return Signature();
        }
        return Signature(signature, Bls::BLS_SIGNATURE_LENGTH);
    }

    bool PrivateKey::getRaw(unsigned char* rawKey, size_t len) const {
        if (len < Bls::BLS_PRI_KEY_LENGTH) {
            return false;
        }
        return Hex::fromHex<unsigned char>(m_key, rawKey, len) == Bls::BLS_PRI_KEY_LENGTH;
    }

    // maybe more condition check
    bool PrivateKey::isValid() const {
        if (m_key.length() == 2 * Bls::BLS_PRI_KEY_LENGTH) {
            return true;
        }
        return false;
    }

    PublicKey PrivateKey::getPublicKey() const {
        std::shared_ptr<Bls> blsPtr = Bls::getDefault();
        unsigned char sk[Bls::BLS_PRI_KEY_LENGTH];
        if (!getRaw(sk, Bls::BLS_PRI_KEY_LENGTH)) {
            return PublicKey();
        }
        unsigned char pk[Bls::BLS_PUB_KEY_LENGTH];
        if (blsPtr->getPk(pk, Bls::BLS_PUB_KEY_LENGTH, sk, Bls::BLS_PRI_KEY_LENGTH)) {
            return PublicKey(pk, Bls::BLS_PUB_KEY_LENGTH);
        }
        return PublicKey();
    }
}