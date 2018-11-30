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
        PrivateKey::generate(privateKey, publicKey);
        BOOST_CHECK(privateKey.isValid());
        BOOST_CHECK(publicKey.isValid());
    }

    BOOST_AUTO_TEST_CASE(sign) {
        PrivateKey privateKey;
        PublicKey publicKey;
        PrivateKey::generate(privateKey, publicKey);
        Digest digest(std::string("hash test"));
        Signature signature = privateKey.sign(digest);
        BOOST_CHECK(signature.isValid());
        BOOST_CHECK(publicKey.verify(signature, digest));
    }

    BOOST_AUTO_TEST_CASE(verify) {
        PrivateKey privateKey;
        PublicKey publicKey;
        PrivateKey::generate(privateKey, publicKey);
        PrivateKey sk1;
        PublicKey pk1;
        PrivateKey::generate(sk1, pk1);
        Digest digest(std::string("hash test"));
        Signature signature = privateKey.sign(digest);
        BOOST_CHECK(!pk1.verify(signature, digest));
    }

    BOOST_AUTO_TEST_CASE(verifyKeyPair) {
        PrivateKey privateKey;
        PublicKey publicKey;
        PrivateKey::generate(privateKey, publicKey);
        PrivateKey sk1;
        PublicKey pk1;
        PrivateKey::generate(sk1, pk1);
        BOOST_CHECK(PrivateKey::verifyKeyPair(publicKey, privateKey));
        BOOST_CHECK(PrivateKey::verifyKeyPair(pk1, sk1));

        BOOST_CHECK(!PrivateKey::verifyKeyPair(pk1, privateKey));
        BOOST_CHECK(!PrivateKey::verifyKeyPair(publicKey, sk1));

        std::string priHexStr(privateKey);
        std::string pubHexStr(publicKey);
        BOOST_CHECK(PrivateKey::verifyKeyPair(PrivateKey(priHexStr).getPublicKey(), PrivateKey(priHexStr)));
    }

    BOOST_AUTO_TEST_CASE(verifyKeyPair_genesis) {
        PrivateKey privateKey("5e41d815136e380ba5b049fec3cb71d5a1d0a6f8");
        PublicKey publicKey("9b2bb26984ae9e32e8801c17b5786dd2b37c86356bcf5f861c5a4bf1ea47b9bc0e21baaa6165dabfedab1b35ccec0d359d1562248a3faa7dbd8bc0434f3e796a5d5f2463b02cf85f35d8567b4085173896988630970a9d374a087c4ec9cefb9c60951db95a6fd57e2ca49d0bfe24c1b13c2854db4a7857f42d5ab05462eb634b");
        BOOST_CHECK(PrivateKey::verifyKeyPair(publicKey, privateKey));
    }

BOOST_AUTO_TEST_SUITE_END()
