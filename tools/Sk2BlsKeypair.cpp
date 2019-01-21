#include <iostream>

#include <base/Hex.h>
#include <crypto/Ed25519.h>
#include <crypto/Bls.h>

using namespace ultrainio;
using namespace std;

static std::string sk[8] = {
        "5079f570cde7801c70a19fb2c7e292d09923218f2684c8a1121c2da7a02a5dc3369c31f242bfc5093815511e4a4eda297f4b8772a7ff98f7806ce7a80ffffb35",
        "97d157bf7fdedfeaef46216e246cc83f9c25574b49bacf83a1862ef0d82233ecb7e0a16fdca44d4ece1b14d8e7e6207402a6447115ca7d2d7edb08958e6d8ed5",
        "b5b3b423eed6fd6f255fe1e4570e3a9f9878a36870b584b1d231ca778ad95ede4031a95071a092eca8646d3999c438bdfde368d4837770755af65a11b4520a48",
        "5cfdb237515f969620f857658baac5485266f1c143fee80208e0da9935bbfaae2b43a3d8e0523a85141bbfca41006cf9abc47587d0ff3c46d257551ec05f1677",
        "6b8ed821703f369f1a6b7c767bf076b093a17a5f839ad66d77d808f86a68dd7db9a55c3c661abd8c539b7a7c05c8176036f87aeeb6b117a138327cfdb374cc23",
        "fe512a00e3c5d22cc8a849a0713df76733ed730bd66cb85b08dee5e7d6e7b15a8f6e3b3336276138023617f0ae0e6fd0c37d27aa9995f9803fe78df4941dd3ec",
        "ab0c2e8ea8ba037f169dbb384605b227016c617d48495fd78250b2809dfa06203696fdfb2a9716ae4b1bca0d5b0a861a1e5d64562994aeb515eed49290c9f1c2",
        "0bccefe21fa6beb22335e1b200c8a4fdd9d250527d643f33bb4d080d6941ff1a578451935c370d9c7fbcdd77e35a40e49bf0a5311e065035778ac27e6263b10d"
};

int main(int argc, char* argv[]) {
    std::shared_ptr<Bls> blsPtr = Bls::getDefault();
    int n = 8;
    std::cout << "n : " << n << std::endl;
    for (int i = 0; i < n; i++) {
        unsigned char usk[Bls::BLS_PRI_KEY_LENGTH];
        Bls::getSk(sk[i], usk, Bls::BLS_PRI_KEY_LENGTH);
        unsigned char pk[Bls::BLS_PUB_KEY_COMPRESSED_LENGTH];
        blsPtr->getPk(pk, Bls::BLS_PUB_KEY_COMPRESSED_LENGTH, usk, Bls::BLS_PRI_KEY_LENGTH);
        std::cout << "\"" << Hex::toHex<unsigned char>(pk, Bls::BLS_PUB_KEY_COMPRESSED_LENGTH) << "\"" << std::endl;
    }
    return 0;
}
