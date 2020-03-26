#include "gm/sm3/Digest.h"

#include <openssl/evp.h>

#include <iostream>

#include "base/Hex.h"

namespace gm {
    namespace sm3 {
        const int Digest::kDigestMaxLen = EVP_MAX_MD_SIZE;

        bool Digest::sm3Hash(const unsigned char* msg, size_t msgLen, unsigned char* hash, unsigned int* hashLen) {
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

        bool Digest::sm3Hash(const unsigned char* msg, size_t msgLen, Digest& outDigest) {
            unsigned char hash[Digest::kDigestMaxLen];
            unsigned int hashLen = 0;
            bool res = Digest::sm3Hash(msg, msgLen, hash, &hashLen);
            if (!res) {
                return res;
            }
            outDigest.reset(ultrainio::Hex::toHex<unsigned char>(hash, hashLen));
            return res;
        }

        Digest::Digest() {}

        Digest::Digest(const std::string& digest) : m_digest(digest) {}

        Digest::operator std::string() const {
            return m_digest;
        }

        void Digest::reset(const std::string& digest) {
            m_digest = digest;
        }
    }
}
