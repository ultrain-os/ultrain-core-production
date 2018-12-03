#include <crypto/Bls.h>

#include <cstring>
#include <chrono>
#include <iostream>

using namespace ultrainio;

int main(int argc, char* argv[]) {
    unsigned char** sk;
    unsigned char** pk;
    unsigned char** signature;
    char* hmsg = "ultrain";
    if (argc < 3) {
        std::cout << "argument count < 3";
        return 0;
    }
    int n = atoi(argv[2]);
    std::shared_ptr<Bls> blsPtr = Bls::getDefault();
    // alloc
    sk = (unsigned char**)malloc(sizeof(unsigned char*) * n);
    pk = (unsigned char**)malloc(sizeof(unsigned char*) * n);
    signature = (unsigned char**)malloc(sizeof(unsigned char*) * n);
    for (int i = 0; i < n; i++) {
        sk[i] = (unsigned char*)malloc(sizeof(unsigned char) * Bls::BLS_PRI_KEY_LENGTH);
        pk[i] = (unsigned char*)malloc(sizeof(unsigned char) * Bls::BLS_PUB_KEY_LENGTH);
        signature[i] = (unsigned char*)malloc(sizeof(unsigned char) * Bls::BLS_SIGNATURE_LENGTH);
        blsPtr->keygen(sk[i], Bls::BLS_PRI_KEY_LENGTH, pk[i], Bls::BLS_PUB_KEY_LENGTH);
    }

    std::chrono::steady_clock::time_point pointStart = std::chrono::steady_clock::now();
    for (int i = 0; i < n; i++) {
        blsPtr->signature(sk[i], (void*)hmsg, strlen(hmsg), signature[i], Bls::BLS_SIGNATURE_LENGTH);
    }
    std::chrono::microseconds d = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - pointStart);
    std::cout << "sign each one consume : " << d.count() / n << " microseconds " << std::endl;

    pointStart = std::chrono::steady_clock::now();
    for (int i = 0; i < n; i++) {
        blsPtr->verify(pk[i], signature[i], (void*)hmsg, strlen(hmsg));
    }
    d = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - pointStart);
    std::cout << "verify each one consume : " << d.count() / n << " microseconds " << std::endl;
    return 0;
}