#define BOOST_TEST_MODULE crypto_unittest
#include <boost/test/included/unit_test.hpp>

#include <crypto/Digest.h>
#include <crypto/PrivateKey.h>
#include <crypto/PublicKey.h>
#include <crypto/Signature.h>

using namespace ultrainio;
using namespace std;

// Private/Public Key
BOOST_AUTO_TEST_SUITE(key_test_suite)

    BOOST_AUTO_TEST_CASE(normal_flow) {
        PrivateKey privateKey = PrivateKey::generate();
        PublicKey publicKey = privateKey.getPublicKey();
        Digest digest(std::string("hash test"));
        Signature signature = privateKey.sign(digest);
        BOOST_CHECK(publicKey.verify(signature, digest));

        std::string privateKeyStr = std::string(privateKey);
        std::string publicKeyStr = std::string(publicKey);
        PublicKey pk1(publicKeyStr);
        PrivateKey sk1(privateKeyStr, pk1);
        Signature signature1 = sk1.sign(digest);
        BOOST_CHECK(pk1.verify(signature, digest));
    }

    BOOST_AUTO_TEST_CASE(verifyKeyPair) {
        PrivateKey privateKey = PrivateKey::generate();
        PublicKey publicKey = privateKey.getPublicKey();
        PrivateKey sk1 = PrivateKey::generate();
        PublicKey pk1 = sk1.getPublicKey();
        BOOST_CHECK(PrivateKey::verifyKeyPair(publicKey, privateKey));
        BOOST_CHECK(PrivateKey::verifyKeyPair(pk1, sk1));

        BOOST_CHECK(!PrivateKey::verifyKeyPair(pk1, privateKey));
        BOOST_CHECK(!PrivateKey::verifyKeyPair(publicKey, sk1));
    }

BOOST_AUTO_TEST_SUITE_END()
