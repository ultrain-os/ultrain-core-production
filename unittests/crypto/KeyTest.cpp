#define BOOST_TEST_MODULE key_test_suite
#include <boost/test/included/unit_test.hpp>

#include <iostream>

#include <crypto/Digest.h>
#include <crypto/PrivateKey.h>
#include <crypto/PublicKey.h>
#include <crypto/Signature.h>

using namespace ultrainio;
using namespace std;

// Private/Public Key
BOOST_AUTO_TEST_SUITE(key_test_suite)
    BOOST_AUTO_TEST_CASE(generate) {
        PrivateKey privateKey = PrivateKey::generate();
        PublicKey publicKey = privateKey.getPublicKey();
        BOOST_CHECK(privateKey.isValid());
        BOOST_CHECK(publicKey.isValid());
    }

    BOOST_AUTO_TEST_CASE(sign) {
        PrivateKey privateKey = PrivateKey::generate();
        PublicKey publicKey = privateKey.getPublicKey();
        Digest digest(std::string("hash test"));
        Signature signature = privateKey.sign(digest);
        BOOST_CHECK(signature.isValid());
        BOOST_CHECK(publicKey.verify(signature, digest));
    }

    BOOST_AUTO_TEST_CASE(verify) {
        PrivateKey privateKey = PrivateKey::generate();
        PublicKey publicKey = privateKey.getPublicKey();
        PrivateKey sk1 = PrivateKey::generate();
        PublicKey pk1 = sk1.getPublicKey();
        Digest digest(std::string("hash test"));
        Signature signature = privateKey.sign(digest);
        BOOST_CHECK(!pk1.verify(signature, digest));
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

        std::string priHexStr(privateKey);
        std::string pubHexStr(publicKey);
        cout << "pri : " << priHexStr << endl;
        cout << "pub : " << pubHexStr << endl;
        BOOST_CHECK(PrivateKey::verifyKeyPair(PublicKey(pubHexStr), PrivateKey(priHexStr, PublicKey(pubHexStr))));
    }

BOOST_AUTO_TEST_SUITE_END()
