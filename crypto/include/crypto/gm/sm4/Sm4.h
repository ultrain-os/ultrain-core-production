#pragma once

#include <stdint.h>
#include <stdio.h>

namespace gm {
    namespace sm4 {
        class Sm4 {
        public:
            static bool cbcEncrypt(const uint8_t* key, const uint8_t* plainText, int plainTextLen,
                    uint8_t** cipherText, int* cipherTextLen);

            static bool cbcDecrypt(const uint8_t* key, const uint8_t* cipherText, int cipherTextLen,
                    uint8_t** plainText, int* plainTextLen);
        };
    }
}
