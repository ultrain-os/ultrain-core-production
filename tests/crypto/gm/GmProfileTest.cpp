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

#include <ed25519/Ed25519.h>

using namespace gm;
using namespace ed25519;
using namespace std;

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cout << " -n 100 " << std::endl;
        return 0;
    }
    int n = stoi(std::string(argv[2]));
    const unsigned char msg[] = {'a', 'b', 'c', 0};

    fc::sha256 data = fc::sha256::hash(std::string((char*)msg));

    // sm2
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

    // ed25519
    std::vector<uint8_t*> ed25519PriKeyV;
    std::vector<uint8_t*> ed25519PubKeyV;
    std::vector<uint8_t*> ed25519SigV;
    for (int i = 0; i < n; i++) {
        uint8_t* priKey = (uint8_t*)malloc(Ed25519::PRIVATE_KEY_LEN);
        ed25519PriKeyV.push_back(priKey);
        uint8_t* pubKey = (uint8_t*)malloc(Ed25519::PUBLIC_KEY_LEN);
        ed25519PubKeyV.push_back(pubKey);
        uint8_t* sig = (uint8_t*)malloc(Ed25519::SIGNATURE_LEN);
        ed25519SigV.push_back(sig);
        Ed25519::keypair(ed25519PubKeyV[i], ed25519PriKeyV[i]);
        Ed25519::sign(ed25519SigV[i], msg, strlen((char*)msg), ed25519PriKeyV[i]);
    }

    std::chrono::steady_clock::time_point ed25519PointStart = std::chrono::steady_clock::now();
    for (int i = 0; i < n; i++) {
        bool ok = Ed25519::verify(msg, strlen((char*)msg), ed25519SigV[i], ed25519PubKeyV[i]);
        if (!ok) {
            std::cout << "ed25519 verify failed." << std::endl;
        }
    }
    std::chrono::microseconds ed25519Total = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now()
            - ed25519PointStart);

    std::chrono::steady_clock::time_point pointStartSecp256K1 = std::chrono::steady_clock::now();
    for (int i = 0; i < n; i++) {
        if (std::string(ecp256PubV[i]) != std::string(fc::crypto::public_key(ecp256SigV[i], data))) {
            std::cout << "secp256k1 verify error" << std::endl;
        }
    }
    std::chrono::microseconds secp256K1Total = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now()
            - pointStartSecp256K1);

    std::chrono::steady_clock::time_point sm2PointStart = std::chrono::steady_clock::now();
    for (int i = 0; i < n; i++) {
        gm::sm2::PublicKey pub = gmPubV[i];
        bool ok = pub.verify(data.data(), data.data_size(), gmSigV[i]);
        if (!ok) {
            std::cout << "gm verify error. at " << i << std::endl;
        }
    }
    std::chrono::microseconds sm2Total = std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::steady_clock::now() - sm2PointStart);

    for (int i = 0; i < n; i++) {
        free(ed25519PriKeyV[i]);
        free(ed25519PubKeyV[i]);
        free(ed25519SigV[i]);
    }
    std::cout << "ed25519 verify one time:" << ed25519Total.count() / n << std::endl;
    std::cout << "sm2 verify one time: " << sm2Total.count() / n << " microseconds " << std::endl;
    std::cout << "secp256k1 verify one time:" << secp256K1Total.count() / n << std::endl;
    return 0;
}