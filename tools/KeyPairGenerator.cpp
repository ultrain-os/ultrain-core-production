#include <cstdlib>
#include <iostream>
#include <vector>
#include <string>

#include <ed25519/Digest.h>
#include <ed25519/PrivateKey.h>
#include <ed25519/PublicKey.h>
#include <ed25519/Signature.h>

#include <stdio.h>
using namespace ed25519;
using namespace std;

int main(int argc, char* argv[]) {
    if (argc < 3) {
        cout << "argument error: " << endl;
        cout << "keypair_generator -n 100" << endl;
        return 0;
    }

    int n = atoi(argv[2]);

    std::vector<std::string> skList;
    std::vector<std::string> pkList;
    for (int i = 0; i < n; i++) {
        ed25519::PrivateKey privateKey;
        ed25519::PublicKey publicKey;
        ed25519::PrivateKey::generate(publicKey, privateKey);
        std::string hexPri = std::string(privateKey);
        std::string hexPub = std::string(publicKey);
        cout << "pri:" << hexPri << endl;
        cout << "pub:" << hexPub << endl;
        skList.push_back(hexPri);
        pkList.push_back(hexPub);
        cout << endl;
    }

    std::cout << "pri key list" << std::endl;
    for (int i = 0; i < n; i++) {
        std::cout << "\"" << skList[i] << "\"," << std::endl;
    }

    std::cout << std::endl << "pub key list" << std::endl;
    for (int i = 0; i < n; i++) {
        std::cout << "\"" << pkList[i] << "\"," << std::endl;
    }

    return 0;
}
