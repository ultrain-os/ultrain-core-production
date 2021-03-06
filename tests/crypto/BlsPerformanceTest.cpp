#include <crypto/Bls.h>

#include <cstring>
#include <chrono>
#include <iostream>

#include <base/Memory.h>

using namespace ultrainio;

int main(int argc, char* argv[]) {
    std::string hmsg("ultrain");
    if (argc < 5) {
        std::cout << "./build/tests/crypto/bls_performance_test -n 100 -t 1\n";
        return 0;
    }
    int n = atoi(argv[2]);
    int t = atoi(argv[4]);
    std::shared_ptr<Bls> blsPtr = Bls::getDefault();
    while (t-- > 0) {
        // alloc
        unsigned char **sks = (unsigned char **) malloc(sizeof(unsigned char *) * n);
        unsigned char **pks = (unsigned char **) malloc(sizeof(unsigned char *) * n);
        unsigned char **signature = (unsigned char **) malloc(sizeof(unsigned char *) * n);
        for (int i = 0; i < n; i++) {
            sks[i] = (unsigned char *) malloc(sizeof(unsigned char) * Bls::BLS_PRI_KEY_LENGTH);
            pks[i] = (unsigned char *) malloc(sizeof(unsigned char) * Bls::BLS_PUB_KEY_COMPRESSED_LENGTH);
            signature[i] = (unsigned char *) malloc(sizeof(unsigned char) * Bls::BLS_SIGNATURE_COMPRESSED_LENGTH);
            blsPtr->keygen(sks[i], Bls::BLS_PRI_KEY_LENGTH, pks[i], Bls::BLS_PUB_KEY_COMPRESSED_LENGTH);
        }

        std::chrono::steady_clock::time_point pointStart = std::chrono::steady_clock::now();
        for (int i = 0; i < n; i++) {
            blsPtr->sign(sks[i], (void *) hmsg.c_str(), hmsg.length(), signature[i],
                         Bls::BLS_SIGNATURE_COMPRESSED_LENGTH);
        }
        std::chrono::microseconds d = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::steady_clock::now() - pointStart);
        std::cout << "sign each one consume : " << d.count() / n << " microseconds " << std::endl;

        pointStart = std::chrono::steady_clock::now();
        for (int i = 0; i < n; i++) {
            blsPtr->verify(pks[i], signature[i], (void *) hmsg.c_str(), hmsg.length());
        }
        d = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - pointStart);
        std::cout << "verify each one consume : " << d.count() / n << " microseconds " << std::endl;

        pointStart = std::chrono::steady_clock::now();
        unsigned char signX[Bls::BLS_SIGNATURE_COMPRESSED_LENGTH];
        for (int i = 0; i < n; i++) {
            blsPtr->aggregate(signature, n, signX, Bls::BLS_SIGNATURE_COMPRESSED_LENGTH);
        }
        d = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - pointStart);
        std::cout << "aggregate " << n << " signature consume : " << d.count() / n << " microseconds " << std::endl;

        pointStart = std::chrono::steady_clock::now();
        for (int i = 0; i < n; i++) {
            blsPtr->verifyAggregate(pks, n, signX, Bls::BLS_SIGNATURE_COMPRESSED_LENGTH, (void *) hmsg.c_str(),
                                    hmsg.length());
        }
        d = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - pointStart);
        std::cout << "verifyAggregate signature consume : " << d.count() / n << " microseconds " << std::endl;

        Memory::freeMultiDim<unsigned char>(sks, n);
        Memory::freeMultiDim<unsigned char>(pks, n);
        Memory::freeMultiDim<unsigned char>(signature, n);
    }
    return 0;
}