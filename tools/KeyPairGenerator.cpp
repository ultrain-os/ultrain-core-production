#include <cstdlib>
#include <iostream>
#include <string>

#include <crypto/Digest.h>
#include <crypto/PrivateKey.h>
#include <crypto/PublicKey.h>
#include <crypto/Signature.h>

#include <stdio.h>
using namespace ultrainio;
using namespace std;

int main(int argc, char* argv[]) {
    if (argc < 3) {
        cout << "argument error: " << endl;
        cout << "keypair_generator -n 100" << endl;
        return 0;
    }

    const char* c = "12345abcdefghijklmnopqrstuvwxyz"; // do not use '.'
    int n = atoi(argv[2]);
    for (int i = 0; i < n; i++) {
        PrivateKey privateKey = PrivateKey::generate();
        PublicKey publicKey = privateKey.getPublicKey();
        std::string hexPri = std::string(privateKey);
        std::string hexPub = std::string(publicKey);
        std::string account;
        int remain = i + 1;
        int mod = 31; // 31 character
        while (remain) {
            int index = remain % mod;
            remain /= mod;
            account.push_back(c[index]);
        }
        cout << "pri:" << hexPri << endl;
        cout << "pub:" << hexPub << endl;
        cout << "acc:" << account << endl;
        cout << endl;
    }

    return 0;
}