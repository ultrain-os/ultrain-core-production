#include "gm/sm2/PrivateKey.h"
#include "gm/sm2/PublicKey.h"
#include "gm/sm2/Signature.h"
#include "gm/sm3/Digest.h"
#include "base/Hex.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cout << "argument error: " << std::endl;
        std::cout << "gm_keys_gen -n 100" << std::endl;
        return 0;
    }

    int n = atoi(argv[2]);
    std::vector<std::string> priKeys;
    std::vector<std::string> pubsKeys;
    for (int i = 0; i < n; i++) {
        gm::sm2::PrivateKey privateKey = gm::sm2::PrivateKey::generate();
        gm::sm2::PublicKey publicKey = privateKey.getPublicKey();
        priKeys.push_back(std::string(privateKey));
        pubsKeys.push_back(std::string(publicKey));
    }
    std::cout << "==============public keys===================" << std::endl;
    for (int i = 0; i < n; i++) {
        std::cout << "\"" << pubsKeys[i] << "\"," << std::endl;
    }
    std::cout << "==============private keys===================" << std::endl;
    for (int i = 0; i < n; i++) {
        std::cout << "\"" << priKeys[i] << "\"," << std::endl;
    }
    return 0;
}