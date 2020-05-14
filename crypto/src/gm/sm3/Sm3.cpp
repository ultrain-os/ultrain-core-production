#include "gm/sm3/Sm3.h"

#include <iostream>

#include "base/Hex.h"

namespace gm {
    namespace sm3 {
        bool Sm3::hash(const uint8_t* msg, size_t msgLen, uint8_t* hash, unsigned int* hashLen) {
            const EVP_MD* md = EVP_sm3();
            if (!md) {
                std::cout << "sm3Hash error, md is nullptr" << std::endl;
                return false;
            }
            EVP_MD_CTX* mdCtx = EVP_MD_CTX_new();
            EVP_DigestInit_ex(mdCtx, md, nullptr);
            EVP_DigestUpdate(mdCtx, msg, msgLen);
            EVP_DigestFinal_ex(mdCtx, hash, hashLen);
            // TODO any memory leak EVP_MD_CTX_cleanup
            EVP_MD_CTX_free(mdCtx);
            return true;
        }

        Sm3 Sm3::hash(const uint8_t* msg, size_t msgLen) {
            Sm3 sm3;
            Sm3::hash(msg, msgLen, sm3.data(), &sm3.m_size);
            return sm3;
        }

        Sm3::Sm3() {
            memset(m_hash, 0x00, kHashMaxLen);
        }

        Sm3::operator std::string() const {
            return ultrainio::Hex::toHex<uint8_t>(m_hash, m_size);
        }

        uint8_t* Sm3::data() {
            return m_hash;
        }

        const uint8_t* Sm3::data() const {
            return m_hash;
        }

        int Sm3::dataSize() const {
            return m_size;
        }

        bool operator == (const gm::sm3::Sm3& lhs, const gm::sm3::Sm3& rhs) {
            if (&lhs == &rhs) {
                return true;
            }
            return lhs.dataSize() == rhs.dataSize() && !memcmp(lhs.data(), rhs.data(), lhs.dataSize());
        }
    }
}
