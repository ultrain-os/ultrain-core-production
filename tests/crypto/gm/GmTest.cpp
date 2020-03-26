#include "gm/sm2/PrivateKey.h"
#include "gm/sm2/PublicKey.h"
#include "gm/sm2/Signature.h"
#include "gm/sm3/Digest.h"

#include <stdio.h>
#include <string.h>
#include <iostream>

using namespace gm;
using namespace std;

int main(int argc, char* argv[]) {
    const unsigned char msg[] = {'a', 'b', 'c', 0};
    gm::sm2::PrivateKey privateKey = gm::sm2::PrivateKey::generate();;
    gm::sm2::PublicKey publicKey = privateKey.getPublicKey();

    std::cout << "private: " << std::string(privateKey) << std::endl;
    std::cout << "public: " << std::string(publicKey) << std::endl;

    for (int i = 0; i < 10; i++) {
        gm::sm2::PrivateKey privateKey = gm::sm2::PrivateKey::generate();;
        gm::sm2::PublicKey publicKey = privateKey.getPublicKey();

        std::cout << "private: " << std::string(privateKey) << std::endl;
        std::cout << "public: " << std::string(publicKey) << std::endl;
    }

    //sign
    gm::sm2::Signature signature;
    privateKey.sign((char*)msg, strlen((char*)msg), signature);
    std::cout << "signature: " << std::string(signature) << std::endl;

    std::cout << "verify: " << publicKey.verify((char*)msg, strlen((char*)msg), signature) << std::endl;

    // Digest
    unsigned char hash[gm::sm3::Digest::kDigestMaxLen];
    unsigned int hashLen;
    gm::sm3::Digest::sm3Hash(msg, strlen((char*)msg), hash, &hashLen);
    gm::sm3::Digest digest;
    gm::sm3::Digest::sm3Hash(msg, strlen((char*)msg), digest);
    std::cout << "Digest: " << std::string(digest) << std::endl;

    return 0;
}