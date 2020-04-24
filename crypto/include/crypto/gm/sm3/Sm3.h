#pragma once

#include <string>
#include <openssl/evp.h>

namespace gm {
    namespace sm3 {
        class Sm3 {
        public:
            static const int kHashMaxLen = EVP_MAX_MD_SIZE;

            static bool hash(const unsigned char* msg, size_t msgLen, unsigned char* hash, unsigned int* hashLen);

            static std::string hash(const unsigned char* msg, size_t msgLen);
        };
    }
}
