#include <iostream>
#include <base/Hex.h>
#include <ultrainio/chain/types.hpp>
#include <crypto/Bls.h>
#include <crypto/Digest.h>
#include <crypto/PrivateKey.h>
#include <crypto/PublicKey.h>
#include <crypto/Signature.h>

using namespace ultrainio;
using namespace std;

int main(int argc, char* argv[]) {
    if (argc < 3) {
        cout << "argument error: " << endl;
        cout << "bls_keypair_gen -n 100" << endl;
        return 0;
    }

    int n = atoi(argv[2]);
    std::shared_ptr<Bls> blsPtr = Bls::getDefault();
    for (int i = 0; i < n; i++) {
        unsigned char blsSk[Bls::BLS_PRI_KEY_LENGTH];
        unsigned char blsPk[Bls::BLS_PUB_KEY_COMPRESSED_LENGTH];
        blsPtr->keygen(blsSk, Bls::BLS_PRI_KEY_LENGTH, blsPk, Bls::BLS_PUB_KEY_COMPRESSED_LENGTH);
        cout << "my-bls-sk = " << Hex::toHex<unsigned char>(blsSk, Bls::BLS_PRI_KEY_LENGTH) << std::endl;
        cout << "my-bls-pk = " << Hex::toHex<unsigned char>(blsPk, Bls::BLS_PUB_KEY_COMPRESSED_LENGTH) << std::endl;
        PrivateKey privateKey;
        PublicKey publicKey;
        PrivateKey::generate(publicKey, privateKey);
        std::string hexPri = std::string(privateKey);
        std::string hexPub = std::string(publicKey);
        cout << "my-sk-as-committee = " << hexPri << endl;
        cout << "my-pk-as-committee = " << hexPub << endl;

        auto pk    = chain::private_key_type::generate();
        cout << "my-sk-as-account = " << std::string(pk) << endl;
        cout << "my-pk-as-account = " << std::string(pk.get_public_key()) << endl;
        cout << endl;
    }
    return 0;
}