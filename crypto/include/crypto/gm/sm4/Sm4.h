#pragma once

#include <stdint.h>
#include <stdio.h>
#include <vector>

namespace gm {
    namespace sm4 {
        class Sm4 {
        public:
            static const int kCipherKeySize = 16;

            static const int kCipherBlockSize = 16;

            static std::vector<char> cbcEncrypt(const uint8_t* key, const uint8_t* plainText,
                    int plainTextLen, bool padding = true);

            static std::vector<char> cbcEncrypt(const uint8_t* key, const uint8_t* iv, const uint8_t* plainText,
                    int plainTextLen, bool padding = true);

            static std::vector<char> cbcDecrypt(const uint8_t* key, const uint8_t* cipherText,
                    int cipherTextLen, bool padding = true);

            static std::vector<char> cbcDecrypt(const uint8_t* key, const uint8_t* iv, const uint8_t* cipherText,
                    int cipherTextLen, bool padding = true);
        };
    }
}
