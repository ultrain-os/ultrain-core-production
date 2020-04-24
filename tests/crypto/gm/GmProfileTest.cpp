#include "gm/sm2/PrivateKey.h"
#include "gm/sm2/PublicKey.h"
#include "gm/sm2/Signature.h"

#include <stdio.h>
#include <string.h>
#include <iostream>

using namespace gm;
using namespace std;

int main(int argc, char* argv[]) {
    const unsigned char msg[] = {'a', 'b', 'c', 0};
    gm::sm2::PrivateKey privateKey = gm::sm2::PrivateKey::generate();
    gm::sm2::PublicKey publicKey = privateKey.getPublicKey();

    //sign
    gm::sm2::Signature signature;

    std::cout << "do sign" << std::endl;
    for (int i = 0; i < 1000000; i++) {
        privateKey.sign((char*)msg, strlen((char*)msg), signature);
    }

    std::cout << "do verify" << std::endl;
    for (int i = 0; i < 1000000; i++) {
        publicKey.verify((char*)msg, strlen((char*)msg), signature);
    }

    return 0;
}