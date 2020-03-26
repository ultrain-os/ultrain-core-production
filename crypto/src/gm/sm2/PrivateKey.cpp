#include "gm/sm2/PrivateKey.h"

#include <iostream>
#include <string.h>

#include <openssl/bn.h>
#include <openssl/ec.h>
#include <openssl/evp.h>
#include <openssl/obj_mac.h>
#include <fc/crypto/sha256.hpp>
#include <fc/crypto/base58.hpp>
#include "base/Hex.h"
#include "base/StringUtils.h"
#include "base/StringUtils.h"
#include "gm/sm2/PublicKey.h"
#include "gm/sm3/Digest.h"

// todo workround
#include "sm2.h"

namespace gm {
    namespace sm2 {
        PrivateKey PrivateKey::generate() {
            EC_GROUP* ecGroup = nullptr;
            EC_KEY* ecKey = nullptr;
            char* privateChar = nullptr;
            PrivateKey privateKey;

            ecGroup = EC_GROUP_new_by_curve_name(NID_sm2);
            if (!ecGroup) {
                std::cout << "new sm2 group error!" << std::endl;
                goto cleanup;
            }
            ecKey = EC_KEY_new();
            if (!ecKey) {
                std::cout << "new sm2 key error" << std::endl;
                goto cleanup;
            }

            if (EC_KEY_set_group(ecKey, (const EC_GROUP*)ecGroup) == 0) {
                std::cout << "set group to key error" << std::endl;
                goto cleanup;
            }

            if (EC_KEY_generate_key(ecKey) == 0) {
                std::cout << "generate key error" << std::endl;
                goto cleanup;
            }

            privateChar = BN_bn2hex(EC_KEY_get0_private_key(ecKey));
            if (!privateChar) {
                std::cout << "private key 2 big number error" << std::endl;
                goto cleanup;
            }
            privateKey.reset(PrivateKey::toWif(ultrainio::StringUtils::paddingPrefixZero(std::string(privateChar), PrivateKey::kSm2PrivateKeyLength * 2)));
            // pubHexKey = std::string(EC_POINT_point2hex(ecGroup, EC_KEY_get0_public_key(ecKey), POINT_CONVERSION_COMPRESSED, nullptr));
        cleanup:
            OPENSSL_free(privateChar);
            EC_GROUP_free(ecGroup);
            EC_KEY_free(ecKey);
            return privateKey;
        }

        PrivateKey PrivateKey::regenerate(const fc::sha256& k) {
            std::string hexStr = ultrainio::Hex::toHex<unsigned char>((unsigned char*)k.data(), k.data_size(), false);
            return PrivateKey(toWif(hexStr));
        }

        PrivateKey::PrivateKey(const std::string& wifKey) : m_wifKey(wifKey) {}

        void PrivateKey::reset(const std::string& wifKey) {
            m_wifKey = wifKey;
        }

        PublicKey PrivateKey::get_public_key() const {
            return getPublicKey();
        }

        PublicKey PrivateKey::getPublicKey() const {
            BIGNUM* pPrivateKeyBn = BN_new();
            BN_CTX* ctx = nullptr;
            char* publicKeyChar = nullptr;
            const EC_GROUP* ecGroup = nullptr;
            EC_POINT* ecPoint = nullptr;
            PublicKey publicKey;

            std::string hexKey = fromWif(m_wifKey);
            BN_hex2bn(&pPrivateKeyBn, hexKey.c_str());
            EC_KEY* ecKey = EC_KEY_new_by_curve_name(NID_sm2);
            if (EC_KEY_set_private_key(ecKey, pPrivateKeyBn) == 0) {
                std::cout << "set private key error" << std::endl;
                goto cleanup;
            }

            ecGroup = EC_KEY_get0_group(ecKey);
            ecPoint = EC_POINT_new(ecGroup);
            ctx = BN_CTX_new();
            if (EC_POINT_mul(ecGroup, ecPoint, pPrivateKeyBn, nullptr, nullptr, ctx) == 0) {
                std::cout << "ec point mul error" << std::endl;
                goto cleanup;
            }

            publicKeyChar = EC_POINT_point2hex(ecGroup, ecPoint, POINT_CONVERSION_COMPRESSED, ctx);
            publicKey.reset(std::string(publicKeyChar));
        cleanup:
            BN_free(pPrivateKeyBn);
            OPENSSL_free(publicKeyChar);
            BN_CTX_free(ctx);
            EC_KEY_free(ecKey);
            EC_POINT_free(ecPoint);
            return publicKey;
        }

        bool PrivateKey::sign(const char* data, size_t dataSize, Signature& outSignature) const {
            bool ok = false;
            BIGNUM* privBn = BN_new();
            EC_POINT* pubPoint = nullptr;
            ECDSA_SIG* sig = nullptr;
            const BIGNUM* sigR = nullptr;
            const BIGNUM* sigS = nullptr;
            char* sigRChar = nullptr;
            char* sigSChar = nullptr;
            std::string finalR;
            std::string finalS;
            std::string hexKey = fromWif(m_wifKey);
            BN_hex2bn(&privBn, hexKey.c_str());
            EC_GROUP* ecGroup = EC_GROUP_new_by_curve_name(NID_sm2);
            EC_KEY* ecKey = EC_KEY_new_by_curve_name(NID_sm2);
            EC_KEY_set_group(ecKey, ecGroup);
            if (!EC_KEY_set_private_key(ecKey, privBn)) {
                std::cout << "set private key error" << std::endl;
                goto cleanup;
            }
            pubPoint = EC_POINT_new(ecGroup);
            EC_POINT_mul(ecGroup, pubPoint, privBn, nullptr, nullptr, nullptr);
            EC_KEY_set_public_key(ecKey, pubPoint);
            sig = sm2_do_sign(ecKey, EVP_sm3(), (const uint8_t *)SM2_DEFAULT_USERID, strlen(SM2_DEFAULT_USERID),
                    (const uint8_t *)data, dataSize);
            if (!sig) {
                std::cout << "sig error" << std::endl;
                goto cleanup;
            }
            ECDSA_SIG_get0(sig, &sigR, &sigS);
            sigRChar = BN_bn2hex(sigR);
            sigSChar = BN_bn2hex(sigS);
            finalR = ultrainio::StringUtils::paddingPrefixZero(std::string(sigRChar), Signature::kRLength * 2);
            finalS = ultrainio::StringUtils::paddingPrefixZero(std::string(sigSChar), Signature::kSLength * 2);
            outSignature.reset(finalR, finalS);
            ok = true;
        cleanup:
            EC_GROUP_free(ecGroup);
            EC_KEY_free(ecKey);
            BN_free(privBn);
            EC_POINT_free(pubPoint);
            ECDSA_SIG_free(sig);
            OPENSSL_free(sigRChar);
            OPENSSL_free(sigSChar);
            return ok;
        }

        Signature PrivateKey::sign( const fc::sha256& digest, bool require_canonical ) const {
            Signature outSignature;
            sign(digest.data(), digest.data_size(), outSignature);
            return outSignature;
        }

        std::string PrivateKey::toWif(const std::string& hexKey) {
            FC_ASSERT(hexKey.length() == 2 * PrivateKey::kSm2PrivateKeyLength, "hex key is not 64");
            unsigned char rawKey[PrivateKey::kSm2PrivateKeyLength];
            ultrainio::Hex::fromHex<unsigned char>(hexKey, rawKey, PrivateKey::kSm2PrivateKeyLength);
            const size_t sizeOfDataToHash = PrivateKey::kSm2PrivateKeyLength + 1;
            const size_t sizeOfHashBytes = 4;
            char data[sizeOfDataToHash + sizeOfHashBytes];
            data[0] = (char)0x80; // this is the Bitcoin MainNet code
            memcpy(&data[1], rawKey, PrivateKey::kSm2PrivateKeyLength);
            fc::sha256 digest = fc::sha256::hash(data, sizeOfDataToHash);
            digest = fc::sha256::hash(digest);
            memcpy(data + sizeOfDataToHash, (char*)&digest, sizeOfHashBytes);
            return fc::to_base58(data, sizeof(data));
        }

        std::string PrivateKey::fromWif(const std::string& wif) {
            auto wifBytes = fc::from_base58(wif);
            FC_ASSERT(wifBytes.size() >= 37); // 1 + 32 + 4
            fc::sha256 check = fc::sha256::hash(wifBytes.data(), wifBytes.size() - 4);
            fc::sha256 check2 = fc::sha256::hash(check);

            FC_ASSERT(memcmp( (char*)&check, wifBytes.data() + wifBytes.size() - 4, 4 ) == 0 ||
                      memcmp( (char*)&check2, wifBytes.data() + wifBytes.size() - 4, 4 ) == 0 );
            unsigned char rawKey[PrivateKey::kSm2PrivateKeyLength];
            memcpy(rawKey, wifBytes.data() + 1, PrivateKey::kSm2PrivateKeyLength);
            return ultrainio::Hex::toHex<unsigned char>(rawKey, PrivateKey::kSm2PrivateKeyLength, false);
        }

        PrivateKey::operator std::string() const {
            return m_wifKey;
        }

        bool PrivateKey::operator == (const PrivateKey& rhs) const {
            if (this == &rhs) {
                return true;
            }
            return this->m_wifKey == rhs.m_wifKey;
        }
    }
}

namespace fc
{
    void to_variant(const gm::sm2::PrivateKey& var, fc::variant& vo)
    {
        vo = std::string(var);
    }

    void from_variant(const fc::variant& var, gm::sm2::PrivateKey& vo)
    {
        vo = gm::sm2::PrivateKey(var.as_string());
    }

} // fc
