#include <core/Message.h>
#include <uranus/Validator.h>
#include <uranus/Signer.h>

using namespace ultrainio;
using namespace std;

int main(int argc, char* argv[]) {
    PrivateKey privateKey = PrivateKey::generate();
    PublicKey publicKey = privateKey.getPublicKey();
    Block block;
    block.version = 1;
    block.proposerPk = std::string(publicKey);
    cout << "block id : " << block.id() << endl;
    block.signature = std::string(Signer::sign<BlockHeader>(block, privateKey));
    cout << "block id : " << block.id() << endl;

    cout << "verify : " << Validator::verify<BlockHeader>(Signature(block.signature), block, publicKey) << endl;

    EchoMsg echoMsg;
    echoMsg.baxCount = 0;
    echoMsg.pk = std::string(publicKey);
    echoMsg.signature = std::string(Signer::sign<UnsignedEchoMsg>(echoMsg, privateKey));

    cout << "verify : " << Validator::verify<UnsignedEchoMsg>(Signature(echoMsg.signature), echoMsg, publicKey) << endl;
}