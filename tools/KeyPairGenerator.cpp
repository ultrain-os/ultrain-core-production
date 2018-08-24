#include <cstdlib>
#include <iostream>
#include <string>

#include <crypto/Digest.h>
#include <crypto/PrivateKey.h>
#include <crypto/PublicKey.h>
#include <crypto/Signature.h>

using namespace ultrainio;
using namespace std;

int main(int argc, char* argv[]) {
    if (argc < 3) {
        cout << "argument error: " << endl;
        cout << "keypair_generator -n 100" << endl;
        return 0;
    }

    int n = atoi(argv[2]);
    for (int i = 0; i < n; i++) {
        PrivateKey privateKey = PrivateKey::generate();
        PublicKey publicKey = privateKey.getPublicKey();
        cout << "pri:" << std::string(privateKey) << endl;
        cout << "pub:" << std::string(publicKey) << endl;
        cout << endl;
    }

    return 0;
}