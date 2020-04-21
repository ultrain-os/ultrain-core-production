#define BOOST_TEST_MODULE key_test_suite
#include <boost/test/included/unit_test.hpp>

#include <iostream>

#include <ed25519/Digest.h>
#include <ed25519/PrivateKey.h>
#include <ed25519/PublicKey.h>
#include <ed25519/Signature.h>

using namespace std;

// Private/Public Key
BOOST_AUTO_TEST_SUITE(key_test_suite)
    BOOST_AUTO_TEST_CASE(generate) {
        ed25519::PrivateKey privateKey;
        ed25519::PublicKey publicKey;
        ed25519::PrivateKey::generate(publicKey, privateKey);
        BOOST_CHECK(privateKey.isValid());
        BOOST_CHECK(publicKey.valid());
    }

    BOOST_AUTO_TEST_CASE(sign) {
        ed25519::PrivateKey privateKey;
        ed25519::PublicKey publicKey;
        ed25519::PrivateKey::generate(publicKey, privateKey);
        ed25519::Digest digest(std::string("hash test"));
        ed25519::Signature signature = privateKey.sign(digest);
        BOOST_CHECK(signature.isValid());
        BOOST_CHECK(publicKey.verify(signature, digest));
    }

    BOOST_AUTO_TEST_CASE(verify) {
        ed25519::PrivateKey privateKey;
        ed25519::PublicKey publicKey;
        ed25519::PrivateKey::generate(publicKey, privateKey);
        ed25519::PrivateKey sk1;
        ed25519::PublicKey pk1;
        ed25519::PrivateKey::generate(pk1, sk1);
        ed25519::Digest digest(std::string("hash test"));
        ed25519::Signature signature = privateKey.sign(digest);
        BOOST_CHECK(!pk1.verify(signature, digest));
    }

    BOOST_AUTO_TEST_CASE(verifyKeyPair) {
        ed25519::PrivateKey privateKey;
        ed25519::PublicKey publicKey;
        ed25519::PrivateKey::generate(publicKey, privateKey);
        ed25519::PrivateKey sk1;
        ed25519::PublicKey pk1;
        ed25519::PrivateKey::generate(pk1, sk1);
        BOOST_CHECK(ed25519::PrivateKey::verifyKeyPair(publicKey, privateKey));
        BOOST_CHECK(ed25519::PrivateKey::verifyKeyPair(pk1, sk1));

        BOOST_CHECK(!ed25519::PrivateKey::verifyKeyPair(pk1, privateKey));
        BOOST_CHECK(!ed25519::PrivateKey::verifyKeyPair(publicKey, sk1));

        std::string priHexStr(privateKey);
        std::string pubHexStr(publicKey);
        BOOST_CHECK(ed25519::PrivateKey::verifyKeyPair(ed25519::PrivateKey(priHexStr).getPublicKey(), ed25519::PrivateKey(priHexStr)));
    }

    BOOST_AUTO_TEST_CASE(verifyKeyPair_genesis) {
        ed25519::PrivateKey privateKey("5079f570cde7801c70a19fb2c7e292d09923218f2684c8a1121c2da7a02a5dc3369c31f242bfc5093815511e4a4eda297f4b8772a7ff98f7806ce7a80ffffb35");
        ed25519::PublicKey publicKey("369c31f242bfc5093815511e4a4eda297f4b8772a7ff98f7806ce7a80ffffb35");
        BOOST_CHECK(ed25519::PrivateKey::verifyKeyPair(publicKey, privateKey));
    }

    BOOST_AUTO_TEST_CASE(operator_case) {
        ed25519::PrivateKey sk;
        ed25519::PublicKey pk;
        ed25519::PrivateKey::generate(pk, sk);
        std::string pkHexStr = std::string(pk);
        ed25519::PublicKey pk1(pkHexStr);
        BOOST_CHECK(pk == pk1);
        ed25519::PrivateKey sk2;
        ed25519::PublicKey pk2;
        ed25519::PrivateKey::generate(pk2, sk2);
        BOOST_CHECK(pk != pk2);
    }

BOOST_AUTO_TEST_SUITE_END()
