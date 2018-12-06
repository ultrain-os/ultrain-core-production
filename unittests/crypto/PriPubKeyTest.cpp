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
        PrivateKey privateKey;
        PublicKey publicKey;
        PrivateKey::generate(publicKey, privateKey);
        BOOST_CHECK(privateKey.isValid());
        BOOST_CHECK(publicKey.isValid());
    }

    BOOST_AUTO_TEST_CASE(sign) {
        PrivateKey privateKey;
        PublicKey publicKey;
        PrivateKey::generate(publicKey, privateKey);
        Digest digest(std::string("hash test"));
        Signature signature = privateKey.sign(digest);
        BOOST_CHECK(signature.isValid());
        BOOST_CHECK(publicKey.verify(signature, digest));
    }

    BOOST_AUTO_TEST_CASE(verify) {
        PrivateKey privateKey;
        PublicKey publicKey;
        PrivateKey::generate(publicKey, privateKey);
        PrivateKey sk1;
        PublicKey pk1;
        PrivateKey::generate(pk1, sk1);
        Digest digest(std::string("hash test"));
        Signature signature = privateKey.sign(digest);
        BOOST_CHECK(!pk1.verify(signature, digest));
    }

    BOOST_AUTO_TEST_CASE(verifyKeyPair) {
        PrivateKey privateKey;
        PublicKey publicKey;
        PrivateKey::generate(publicKey, privateKey);
        PrivateKey sk1;
        PublicKey pk1;
        PrivateKey::generate(pk1, sk1);
        BOOST_CHECK(PrivateKey::verifyKeyPair(publicKey, privateKey));
        BOOST_CHECK(PrivateKey::verifyKeyPair(pk1, sk1));

        BOOST_CHECK(!PrivateKey::verifyKeyPair(pk1, privateKey));
        BOOST_CHECK(!PrivateKey::verifyKeyPair(publicKey, sk1));

        std::string priHexStr(privateKey);
        std::string pubHexStr(publicKey);
        BOOST_CHECK(PrivateKey::verifyKeyPair(PrivateKey(priHexStr).getPublicKey(), PrivateKey(priHexStr)));
    }

    BOOST_AUTO_TEST_CASE(verifyKeyPair_genesis) {
        PrivateKey privateKey("5079f570cde7801c70a19fb2c7e292d09923218f2684c8a1121c2da7a02a5dc3369c31f242bfc5093815511e4a4eda297f4b8772a7ff98f7806ce7a80ffffb35");
        PublicKey publicKey("369c31f242bfc5093815511e4a4eda297f4b8772a7ff98f7806ce7a80ffffb35");
        BOOST_CHECK(PrivateKey::verifyKeyPair(publicKey, privateKey));
    }

    BOOST_AUTO_TEST_CASE(operator_case) {
        PrivateKey sk;
        PublicKey pk;
        PrivateKey::generate(pk, sk);
        std::string pkHexStr = std::string(pk);
        PublicKey pk1(pkHexStr);
        BOOST_CHECK(pk == pk1);
        PrivateKey sk2;
        PublicKey pk2;
        PrivateKey::generate(pk2, sk2);
        BOOST_CHECK(pk != pk2);
    }

BOOST_AUTO_TEST_SUITE_END()
