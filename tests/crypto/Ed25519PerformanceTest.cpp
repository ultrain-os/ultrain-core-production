#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <chrono>
#include <iostream>

#include <crypto/Ed25519.h>
#include <fc/crypto/public_key.hpp>
#include <fc/crypto/private_key.hpp>
#include <fc/crypto/signature.hpp>

using namespace ultrainio;
using namespace std;
using namespace fc;
using namespace fc::crypto;

// test ed25519 time prove/verify consume
int main(int argc, char* argv[]) {
    int NUMBER = 10000;
    uint8_t* skList[NUMBER];
    uint8_t* pkList[NUMBER];
    uint8_t* proofList[NUMBER];
    const char* hash = "hash test";
    for (int i = 0; i < NUMBER; i++) {
        skList[i] = (uint8_t*)malloc(Ed25519::PRIVATE_KEY_LEN);
        pkList[i] = (uint8_t*)malloc(Ed25519::PUBLIC_KEY_LEN);
        proofList[i] = (uint8_t*)malloc(Ed25519::SIGNATURE_LEN);
        Ed25519::keypair(pkList[i], skList[i]);
    }
    // test prove time
    std::chrono::steady_clock::time_point pointStart = std::chrono::steady_clock::now();
    for (int i = 0; i < NUMBER; i++) {
        Ed25519::sign(proofList[i], (uint8_t*)hash, strlen(hash), skList[i]);
    }
    std::chrono::microseconds d = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - pointStart);
    cout << "sign each one consume : " << d.count() / NUMBER << " microseconds " << std::endl;

    pointStart = std::chrono::steady_clock::now();
    for (int i = 0; i < NUMBER; i++) {
        Ed25519::verify((uint8_t*)hash, strlen(hash), proofList[i], pkList[i]);
    }
    d = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - pointStart);
    cout << "verify each one consume : " << d.count() / NUMBER << " microseconds " << std::endl;

    // fc::
    private_key fcSks[NUMBER];
    public_key fcPks[NUMBER];
    signature sig[NUMBER];
    auto digest = sha256::hash(hash, strlen(hash));
    for (int i = 0;i < NUMBER; i++) {
        fcSks[i] = private_key::generate<ecc::private_key_shim>();
        fcPks[i] = fcSks[i].get_public_key();
    }
    // test fc sign
    pointStart = std::chrono::steady_clock::now();
    for (int i = 0; i < NUMBER; i++) {
        sig[i] = fcSks[i].sign(digest);
    }
    d = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - pointStart);
    cout << "ecdsa fc sign each one consume : " << d.count() / NUMBER << " microseconds " << std::endl;

    pointStart = std::chrono::steady_clock::now();
    for (int i = 0; i < NUMBER; i++) {
        assert(std::string(fcPks[i]) == std::string(public_key(sig[i], digest)) == true);
    }
    d = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - pointStart);
    cout << "ecdsa fc verify each one consume : " << d.count() / NUMBER << " microseconds " << std::endl;
    return 0;
}