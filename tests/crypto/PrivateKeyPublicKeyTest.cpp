#include <crypto/Digest.h>
#include <crypto/PrivateKey.h>
#include <crypto/PublicKey.h>
#include <crypto/Signature.h>

#include <iostream>

using namespace ultrainio;
using namespace std;

int main(int argc, char* argv[]) {
    PrivateKey privateKey = PrivateKey::generate();
    std::string privateKeyStr = std::string(privateKey);
    cout << "private key: " << privateKeyStr << endl;
    PublicKey publicKey = privateKey.getPublicKey();
    std::string publicKeyStr = std::string(publicKey);
    cout << "public key: " << publicKeyStr << endl;
    Digest digest(std::string("hash test"));
    Signature signature = privateKey.sign(digest);
    cout << "signature: " << std::string(signature) << endl;
    cout << "result:" << publicKey.verify(signature, digest) << endl;
    cout << "result:" << PublicKey(publicKeyStr).verify(signature, digest) << endl;

    PublicKey pk1(publicKeyStr);
    PrivateKey sk1(privateKeyStr, pk1);
    Signature signature1 = sk1.sign(digest);
    cout << "pk1.verify " << pk1.verify(signature, digest) << endl;
    return 0;
}