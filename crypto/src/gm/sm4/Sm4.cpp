#include "gm/sm4/Sm4.h"

#include <openssl/evp.h>

#include "base/Hex.h"
#include <iostream>

namespace gm {
    namespace sm4 {

        bool Sm4::cbcEncrypt(const uint8_t* key, const uint8_t* plainText, int plainTextLen,
                uint8_t** cipherText, int* cipherTextLen) {
            bool ok = false;
            int len = 0;
            int paddingLen = 0;
            uint8_t* buffer = nullptr;
            uint8_t ivec[EVP_MAX_IV_LENGTH];
            memset(ivec, 0x00, EVP_MAX_IV_LENGTH);
            const EVP_CIPHER* cipher = EVP_sm4_cbc();
            EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
            if (!ctx) {
                std::cout << "no memory to alloc EVP_CIPHER_CTX" << std::endl;
                goto cleanup;
            }
            if (EVP_EncryptInit_ex(ctx, cipher, nullptr, key, ivec) != 1) {
                std::cout << "EVP_EncryptInit_ex error." << std::endl;
                goto cleanup;
            }
            EVP_CIPHER_CTX_set_padding(ctx, 0);
            buffer = (uint8_t*)malloc(plainTextLen);
            if (!buffer) {
                std::cout << "no memory to alloc buffer" << std::endl;
                goto cleanup;
            }
            if (EVP_EncryptUpdate(ctx, buffer, &len, plainText, plainTextLen) != 1) {
                std::cout << "EVP_EncryptUpdate error" << std::endl;
                goto cleanup;
            }
            if (EVP_EncryptFinal_ex(ctx, buffer + len, &paddingLen) != 1) {
                std::cout << "EVP_EncryptFinal_ex error" << std::endl;
                goto cleanup;
            }
            if (cipherText) {
                *cipherText = buffer;
            }
            if (cipherTextLen) {
                *cipherTextLen = len + paddingLen;
            }
            ok = true;
        cleanup:
            EVP_CIPHER_CTX_free(ctx);
            return ok;
        }

        bool Sm4::cbcDecrypt(const uint8_t* key, const uint8_t* cipherText, int cipherTextLen,
                               uint8_t** plainText, int* plainTextLen) {
            bool ok = false;
            int len = 0;
            int paddingLen = 0;
            uint8_t* buffer = nullptr;
            uint8_t ivec[EVP_MAX_IV_LENGTH];
            memset(ivec, 0x00, EVP_MAX_IV_LENGTH);
            const EVP_CIPHER* cipher = EVP_sm4_cbc();
            EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
            if (!ctx) {
                std::cout << "no memory to alloc EVP_CIPHER_CTX" << std::endl;
                goto cleanup;
            }
            if (EVP_DecryptInit_ex(ctx, cipher, nullptr, key, ivec) != 1) {
                std::cout << "EVP_DecryptInit_ex error." << std::endl;
                goto cleanup;
            }
            EVP_CIPHER_CTX_set_padding(ctx, 0);
            buffer = (uint8_t*)malloc(cipherTextLen);
            if (!buffer) {
                std::cout << "no memory to alloc buffer" << std::endl;
                goto cleanup;
            }
            if (EVP_DecryptUpdate(ctx, buffer, &len, cipherText, cipherTextLen) != 1) {
                std::cout << "EVP_DecryptUpdate error" << std::endl;
                goto cleanup;
            }
            if (EVP_DecryptFinal_ex(ctx, buffer + len, &paddingLen) != 1) {
                std::cout << "EVP_DecryptFinal_ex error" << std::endl;
                goto cleanup;
            }
            if (plainText) {
                *plainText = buffer;
            }
            if (plainTextLen) {
                *plainTextLen = len + paddingLen;
            }
            ok = true;
        cleanup:
            EVP_CIPHER_CTX_free(ctx);
            return ok;
        }
    }
}
