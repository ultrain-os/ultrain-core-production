#include "gm/sm2/PublicKey.h"

#include <iostream>
#include <string.h>

#include <openssl/bn.h>
#include <openssl/ec.h>
#include <openssl/evp.h>
#include <openssl/obj_mac.h>
#include <fc/crypto/ripemd160.hpp>
#include <fc/crypto/base58.hpp>
#include <fc/crypto/sha256.hpp>
#include "base/Hex.h"
#include "base/StringUtils.h"
#include "sm2.h"

namespace gm {
    namespace sm2 {
        const char* PublicKey::UTR = "UTR";

        PublicKey::PublicKey(const std::string& wifKey) : m_binKey(wif2Bin(wifKey)) {}

        void PublicKey::reset(const std::string& hexKey) {
            m_binKey = hex2Bin(hexKey);
        }

        bool PublicKey::valid() const {
            return true; //TODO(xiaofen.qin@gmail.com
        }

        PublicKey::operator std::string() const {
            return bin2Wif(m_binKey);
        }

        bool operator < ( const PublicKey& p1, const PublicKey& p2) {
            return p1.m_binKey.get<PublicKey::PublicKeyData>() < p2.m_binKey.get<PublicKey::PublicKeyData>();
        }

        bool operator == (const PublicKey& p1, const PublicKey& p2) {
            if (&p1 == &p2) {
                return true;
            }
            return p1.m_binKey.get<PublicKey::PublicKeyData>() == p2.m_binKey.get<PublicKey::PublicKeyData>();
        }

        bool operator != ( const PublicKey& p1, const PublicKey& p2) {
            return !(p1 == p2);
        }

        std::string PublicKey::base58_to_hex(const std::string& base58str) {
            FC_ASSERT(base58str.length() >= 53, "public key less than 3");
            auto subStr = base58str.substr(strlen(UTR));
            auto bin = fc::from_base58(subStr);
            // do checksum
            uint32_t checksum = calcChecksum(bin.data(), PublicKey::kSm2PublicKeyCompressedLength);
            size_t sizeOfHashBytes = 4;
            FC_ASSERT(memcmp((char*)&checksum, bin.data() + PublicKey::kSm2PublicKeyCompressedLength, sizeOfHashBytes) == 0, "wif ${wif} to public key checksum error", ("wif", base58str));
            return ultrainio::Hex::toHex<unsigned char>((unsigned char*)bin.data(), PublicKey::kSm2PublicKeyCompressedLength, false);
        }

        PublicKey::DataType PublicKey::wif2Bin(const std::string& wif) {
            std::string hexStr = base58_to_hex(wif);
            return hex2Bin(hexStr);
        }

        std::string PublicKey::bin2Hex(const PublicKey::DataType& data) {
            return ultrainio::Hex::toHex<unsigned char>((unsigned char*)data.get<PublicKeyData>().begin(), PublicKey::kSm2PublicKeyCompressedLength, false);
        }

        PublicKey::DataType PublicKey::hex2Bin(const std::string& hexKey) {
            PublicKey::PublicKeyData data;
            FC_ASSERT(hexKey.length() / 2 == PublicKey::kSm2PublicKeyCompressedLength, "public key ${key} not correct", ("key", hexKey));
            ultrainio::Hex::fromHex<unsigned char>(hexKey, (unsigned char*)data.begin(), PublicKey::kSm2PublicKeyCompressedLength);
            return PublicKey::DataType(data);
        }

        std::string PublicKey::bin2Wif(const PublicKey::DataType& data) {
            std::string hexKey = bin2Hex(data);
            return hex2Wif(hexKey);
        }

        std::string PublicKey::hex2Wif(const std::string& hexKey) {
            FC_ASSERT(hexKey.length() / 2 == PublicKey::kSm2PublicKeyCompressedLength, "public key ${key} not correct", ("key", hexKey));
            size_t sizeOfHashBytes = 4;
            unsigned char rawKey[PublicKey::kSm2PublicKeyCompressedLength + sizeOfHashBytes]; // 4 Bytes for checksum
            ultrainio::Hex::fromHex<unsigned char>(hexKey, rawKey, PublicKey::kSm2PublicKeyCompressedLength);
            uint32_t checksum = calcChecksum((char*)rawKey, PublicKey::kSm2PublicKeyCompressedLength);
            memcpy(rawKey + PublicKey::kSm2PublicKeyCompressedLength, (char*)&checksum, sizeOfHashBytes);
            return std::string(UTR) + fc::to_base58((char*)rawKey, PublicKey::kSm2PublicKeyCompressedLength + sizeOfHashBytes);
        }

        uint32_t PublicKey::calcChecksum(const char* data, size_t dataLen) {
            auto encoder = fc::ripemd160::encoder();
            encoder.write(data, dataLen);
            return encoder.result()._hash[0];
        }

        bool PublicKey::verify(const char* data, size_t dataSize, const Signature& signature) const {
            int ok = false;
            EC_KEY* ecKey = nullptr;
            EC_GROUP* ecGroup = nullptr;
            BIGNUM* r = nullptr;
            BIGNUM* s = nullptr;
            ECDSA_SIG* sig = nullptr;
            BN_CTX* bnCtx = nullptr;
            EC_POINT* pkEcPoint = nullptr;
            std::string hexKey;
            if (BN_hex2bn(&r, signature.r().c_str()) == 0) {
                std::cout << "BN_hex2bn error. r = " << signature.r() << std::endl;
                goto cleanup;
            }
            if (BN_hex2bn(&s, signature.s().c_str()) == 0) {
                std::cout << "BN_hex2bn error. s = " << signature.s() << std::endl;
                goto cleanup;
            }
            sig = ECDSA_SIG_new();
            if (!sig) {
                std::cout << "ECDSA_SIG_new error " << std::endl;
                goto cleanup;
            }
            ECDSA_SIG_set0(sig, r, s);
            ecKey = EC_KEY_new_by_curve_name(NID_sm2);
            if (!ecKey) {
                std::cout << " new ecKey error " << std::endl;
                goto cleanup;
            }
            ecGroup = EC_GROUP_new_by_curve_name(NID_sm2);
            if (!ecGroup) {
                std::cout << "new ec group error" << std::endl;
                goto cleanup;
            }
            EC_KEY_set_group(ecKey, ecGroup);
            bnCtx = BN_CTX_new();
            if (!bnCtx) {
                std::cout << " new BN ctx error" << std::endl;
                goto cleanup;
            }
            pkEcPoint = EC_POINT_new(ecGroup);
            if (!pkEcPoint) {
                std::cout << " new ec point error" << std::endl;
                goto cleanup;
            }
            hexKey = bin2Hex(m_binKey);
            EC_POINT_hex2point(ecGroup, hexKey.c_str(), pkEcPoint, bnCtx);
            EC_KEY_set_public_key(ecKey, pkEcPoint);
            if (sm2_do_verify(ecKey, EVP_sm3(), sig, (const uint8_t *)SM2_DEFAULT_USERID, strlen(SM2_DEFAULT_USERID), (const uint8_t *)data, dataSize) == 1) {
                ok = true;
            }
        cleanup:
            ECDSA_SIG_free(sig);
            EC_KEY_free(ecKey);
            EC_GROUP_free(ecGroup);
            BN_CTX_free(bnCtx);
            EC_POINT_free(pkEcPoint);
            return ok;
        }
    }
}

namespace fc {
    void to_variant(const gm::sm2::PublicKey& var,  variant& vo) {
        vo = std::string(var);
    }

    void from_variant(const variant& var, gm::sm2::PublicKey& vo) {
        vo = gm::sm2::PublicKey(var.as_string());
    }
} // namespace fc
