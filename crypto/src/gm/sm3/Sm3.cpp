#include "gm/sm3/Sm3.h"

#include <iostream>

#include "base/Hex.h"

namespace gm {
    namespace sm3 {
        bool Sm3::hash(const unsigned char* msg, size_t msgLen, unsigned char* hash, unsigned int* hashLen) {
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

        std::string Sm3::hash(const unsigned char* msg, size_t msgLen) {
            unsigned char hash[Sm3::kHashMaxLen];
            unsigned int hashLen = 0;
            if (!Sm3::hash(msg, msgLen, hash, &hashLen)) {
                return std::string();
            }
            return ultrainio::Hex::toHex<unsigned char>(hash, hashLen);
        }
    }
}
