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
        std::string privateKeyStr = std::string(privateKey);
        PublicKey publicKey = privateKey.getPublicKey();
        std::string publicKeyStr = std::string(publicKey);
        Digest digest(std::string("hash test"));
        Signature signature = privateKey.sign(digest);
        BOOST_CHECK(publicKey.verify(signature, digest));
        BOOST_CHECK(PublicKey(publicKeyStr).verify(signature, digest));

        PublicKey pk1(publicKeyStr);
        PrivateKey sk1(privateKeyStr, pk1);
        Signature signature1 = sk1.sign(digest);
        BOOST_CHECK(pk1.verify(signature, digest));
    }

BOOST_AUTO_TEST_SUITE_END()
