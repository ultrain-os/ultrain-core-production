
#include <iostream>
#include <base/Hex.h>
#include <crypto/Bls.h>

using namespace ultrainio;
using namespace std;

int main(int argc, char* argv[]) {
    if (argc < 3) {
        cout << "argument error: " << endl;
        cout << "bls_keypair_gen -n 100" << endl;
        return 0;
    }

    int n = atoi(argv[2]);
    std::shared_ptr<Bls> blsPtr = Bls::getDefault();
    unsigned char** skList = (unsigned char**)malloc(n * sizeof(unsigned char*));
    unsigned char** pkList = (unsigned char**)malloc(n * sizeof(unsigned char*));
    for (int i = 0; i < n; i++) {
        skList[i] = (unsigned char*)malloc(Bls::BLS_PRI_KEY_LENGTH);
        pkList[i] = (unsigned char*)malloc(Bls::BLS_PUB_KEY_COMPRESSED_LENGTH);
        blsPtr->keygen(skList[i], Bls::BLS_PRI_KEY_LENGTH, pkList[i], Bls::BLS_PUB_KEY_COMPRESSED_LENGTH);
    }
    for (int i = 0; i < n; i++) {
        std::cout << "\"" << Hex::toHex<unsigned char>(skList[i], Bls::BLS_PRI_KEY_LENGTH) << "\"," << std::endl;
    }
    for (int i = 0; i < n; i++) {
        std::cout << "\"" << Hex::toHex<unsigned char>(pkList[i], Bls::BLS_PUB_KEY_COMPRESSED_LENGTH) << "\"," << std::endl;
    }
    for (int i = 0; i < n; i++) {
        free(skList[i]);
        free(pkList[i]);
    }
    free(skList);
    free(pkList);
    return 0;
}