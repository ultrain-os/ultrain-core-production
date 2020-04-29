#include <chrono>
#include <stdio.h>
#include <string.h>
#include <iostream>

#include <fc/crypto/private_key.hpp>
#include <fc/crypto/public_key.hpp>
#include <fc/crypto/signature.hpp>
#include <fc/crypto/sha256.hpp>

#include "gm/sm2/PrivateKey.h"
#include "gm/sm2/PublicKey.h"
#include "gm/sm2/Signature.h"

using namespace gm;
using namespace std;

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cout << " -n 100 " << std::endl;
        return 0;
    }
    int n = stoi(std::string(argv[2]));
    const unsigned char msg[] = {'a', 'b', 'c', 0};

    fc::sha256 data = fc::sha256::hash(std::string((char*)msg));

    std::vector<gm::sm2::PrivateKey> gmPriV;
    std::vector<gm::sm2::PublicKey> gmPubV;
    std::vector<gm::sm2::Signature> gmSigV;

    for (int i = 0; i < n; i++) {
        gm::sm2::PrivateKey privateKey = gm::sm2::PrivateKey::generate();
        gmPriV.push_back(privateKey);
        gm::sm2::PublicKey publicKey = privateKey.getPublicKey();
        gmPubV.push_back(publicKey);
        gm::sm2::Signature sig;
        privateKey.sign(data.data(), data.data_size(), sig);
        gmSigV.push_back(sig);
    }

    // fc::
    std::vector<fc::crypto::private_key>  ecp256PriV;
    std::vector<fc::crypto::public_key>  ecp256PubV;
    std::vector<fc::crypto::signature>  ecp256SigV;

    for (int i = 0;i < n; i++) {
        fc::crypto::private_key pri = fc::crypto::private_key::generate<fc::ecc::private_key_shim>();
        ecp256PriV.push_back(pri);
        ecp256PubV.push_back(pri.get_public_key());
        ecp256SigV.push_back(pri.sign(data));
    }

    std::chrono::steady_clock::time_point pointStart1 = std::chrono::steady_clock::now();
    for (int i = 0; i < n; i++) {
        if (std::string(ecp256PubV[i]) != std::string(fc::crypto::public_key(ecp256SigV[i], data))) {
            std::cout << "secp256k1 verify error" << std::endl;
        }
    }
    std::chrono::microseconds total1 = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - pointStart1);

    std::chrono::steady_clock::time_point pointStart = std::chrono::steady_clock::now();
    for (int i = 0; i < n; i++) {
        gm::sm2::PublicKey pub = gmPubV[i];
        bool ok = pub.verify(data.data(), data.data_size(), gmSigV[i]);
        if (!ok) {
            std::cout << "gm verify error. at " << i << std::endl;
        }
    }
    std::chrono::microseconds total = std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::steady_clock::now() - pointStart);
    std::cout << "ecdsa verify one time:" << total1.count() / n << "; gm verify one time: " << total.count() / n << " microseconds " << std::endl;
    return 0;
}