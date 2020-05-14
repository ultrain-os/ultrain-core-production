#include "gm/sm4/Sm4.h"

#include <openssl/evp.h>

#include "base/Hex.h"
#include <iostream>

namespace gm {
    namespace sm4 {

        std::vector<char> Sm4::cbcEncrypt(const uint8_t* key, const uint8_t* plainText, int plainTextLen, bool padding) {
            uint8_t iv[EVP_MAX_IV_LENGTH];
            memset(iv, 0x00, EVP_MAX_IV_LENGTH);
            return cbcEncrypt(key, iv, plainText, plainTextLen, padding);
        }

        std::vector<char> Sm4::cbcEncrypt(const uint8_t* key, const uint8_t* iv, const uint8_t* plainText,
                int plainTextLen, bool padding) {
            int len = 0;
            int paddingLen = 0;
            std::vector<char> cipherText(plainTextLen + kCipherBlockSize - 1);
            const EVP_CIPHER* cipher = EVP_sm4_cbc();
            EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
            if (!ctx) {
                std::cout << "no memory to alloc EVP_CIPHER_CTX" << std::endl;
                goto cleanup;
            }
            if (EVP_EncryptInit_ex(ctx, cipher, nullptr, key, iv) != 1) {
                std::cout << "EVP_EncryptInit_ex error." << std::endl;
                goto cleanup;
            }
            if (!padding) {
                EVP_CIPHER_CTX_set_padding(ctx, 0);
            }

            if (EVP_EncryptUpdate(ctx, (unsigned char*)(cipherText.data()), &len, plainText, plainTextLen) != 1) {
                std::cout << "EVP_EncryptUpdate error" << std::endl;
                goto cleanup;
            }
            if (EVP_EncryptFinal_ex(ctx, (unsigned char*)(cipherText.data()) + len, &paddingLen) != 1) {
                std::cout << "EVP_EncryptFinal_ex error" << std::endl;
                goto cleanup;
            }
            //FC_ASSERT(len + paddingLen <= cipherText.size());
            cleanup:
            cipherText.resize(len + paddingLen);
            EVP_CIPHER_CTX_free(ctx);
            return cipherText;
        }

        std::vector<char> Sm4::cbcDecrypt(const uint8_t* key, const uint8_t* cipherText, int cipherTextLen, bool padding) {
            uint8_t iv[EVP_MAX_IV_LENGTH];
            memset(iv, 0x00, EVP_MAX_IV_LENGTH);
            return cbcDecrypt(key, iv, cipherText, cipherTextLen, padding);
        }

        std::vector<char> Sm4::cbcDecrypt(const uint8_t* key, const uint8_t* iv, const uint8_t* cipherText,
                int cipherTextLen, bool padding) {
            int len = 0;
            int paddingLen = 0;
            std::vector<char> plainText(cipherTextLen + kCipherBlockSize);
            const EVP_CIPHER* cipher = EVP_sm4_cbc();
            EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
            if (!ctx) {
                std::cout << "no memory to alloc EVP_CIPHER_CTX" << std::endl;
                goto cleanup;
            }
            if (EVP_DecryptInit_ex(ctx, cipher, nullptr, key, iv) != 1) {
                std::cout << "EVP_DecryptInit_ex error." << std::endl;
                goto cleanup;
            }
            if (!padding) {
                EVP_CIPHER_CTX_set_padding(ctx, 0);
            }

            if (EVP_DecryptUpdate(ctx, (unsigned char *) (plainText.data()), &len, cipherText, cipherTextLen) != 1) {
                std::cout << "EVP_DecryptUpdate error" << std::endl;
                goto cleanup;
            }
            if (EVP_DecryptFinal_ex(ctx, (unsigned char *) (plainText.data()) + len, &paddingLen) != 1) {
                std::cout << "EVP_DecryptFinal_ex error" << std::endl;
                goto cleanup;
            }
            cleanup:
            plainText.resize(len + paddingLen);
            EVP_CIPHER_CTX_free(ctx);
            return plainText;
        }
    }
}
