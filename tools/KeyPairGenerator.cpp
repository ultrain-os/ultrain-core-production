#include <cstdlib>
#include <iostream>
#include <vector>
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

    int n = atoi(argv[2]);

    std::vector<std::string> skList;
    std::vector<std::string> pkList;
    for (int i = 0; i < n; i++) {
        PrivateKey privateKey;
        PublicKey publicKey;
        PrivateKey::generate(publicKey, privateKey);
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
