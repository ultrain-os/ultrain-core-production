#define BOOST_TEST_MODULE gm_test_suite
#include <boost/test/included/unit_test.hpp>

#include "gm/sm2/PrivateKey.h"
#include "gm/sm2/PublicKey.h"
#include "gm/sm2/Signature.h"
#include "gm/sm3/Digest.h"
#include "base/Hex.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>

using namespace std;

// Private/Public Key
BOOST_AUTO_TEST_SUITE(gm_test_suite)

BOOST_AUTO_TEST_CASE(sign) {
    const char test[] = {'t', 'e', 's', 't'};
    for (int i = 0; i < 100; i++) {
        gm::sm2::PrivateKey privateKey = gm::sm2::PrivateKey::generate();
        gm::sm2::PublicKey publicKey = privateKey.getPublicKey();
        gm::sm2::Signature signature;
        privateKey.sign(test, strlen(test), signature);
        BOOST_CHECK(publicKey.valid());
        BOOST_CHECK(publicKey.verify(test, strlen(test), signature));
    }
}

BOOST_AUTO_TEST_CASE(regenerate) {
    for (int i = 0; i < 100; i++) {
        gm::sm2::PrivateKey privateKey = gm::sm2::PrivateKey::regenerate(fc::sha256::hash(std::string("nathan")));
        gm::sm2::PublicKey publicKey = privateKey.getPublicKey();
        BOOST_CHECK(publicKey.valid());
        gm::sm2::PrivateKey privateKey2 = gm::sm2::PrivateKey::regenerate(fc::sha256::hash(std::string("nathan")));
        gm::sm2::PublicKey publicKey2 = privateKey2.getPublicKey();
        BOOST_CHECK(publicKey2.valid());
        BOOST_CHECK(privateKey == privateKey2);
        BOOST_CHECK(publicKey == publicKey2);
    }
}

BOOST_AUTO_TEST_CASE(publickey_checksum) {
        gm::sm2::PrivateKey pri("5KV2NYzZMboeYhTz5UHs9G9CGzSxYrSm8tHcCaVeEW3b3HvJPpr");
        gm::sm2::PublicKey expectedPub("UTR7EL1bRiFqEfzcAUZBj3ggSLFd2UrkRwZkeC72tXWunia4panq9");

        gm::sm2::PublicKey actualKey = pri.getPublicKey();
        BOOST_CHECK(actualKey == expectedPub);
}

BOOST_AUTO_TEST_CASE(signature_checksum) {
        gm::sm2::PrivateKey pri("5HqEZtaJrTC936yiGr4Kgns3dStfLU1icemBUXqARbSx3yFsGJX");

        gm::sm2::PublicKey pub = pri.getPublicKey();
        gm::sm2::Signature actual;
        pri.sign("hello", 5, actual);
        gm::sm2::Signature sig(std::string("SIG_GM_LJXHki9m4T8exZDxposZoLzYdYLNxxSZXDebnHYCAqQ4mtCmzoYhm7XBchTtR2JQs4XFYHJJS8DcyE37Vhtm2ZBRuANPiP"));
        BOOST_CHECK(pub.verify("hello", 5, sig));
}

//// see sm2 doc
BOOST_AUTO_TEST_CASE(verify) {
        const unsigned char msg[] = {'a', 'b', 'c', 0};
        // private key: D8C2FCAC2EF69C732CC6F7892267A03618E11BAC2990B2F2FC1B14CF27445718
        gm::sm2::PrivateKey d8Pri("5KTkU4ZbteQXuVoJi196U5ydoMXRw2DeErjgYcaCHTpKxuUvBmk");
        gm::sm2::PublicKey d8Pub = d8Pri.getPublicKey();
        // sig: bd8c2b78eb5c94c23f6f51c5723ca6854b942b8d3569fcd4408e38d2363bb45e2e90de9f95310fdc6dfd0b2c00881c29daf8491d0adc0eff54f2d19d99e26cab
        gm::sm2::Signature d8Signature(std::string("SIG_GM_LV39xAkCK2kxmbUdoA3LY3Fn8t4Ddy2beQzhGz5mNrxb83Aor8pzi5nSSboc9CHD9Ldyxy7YrZuPxHctGSqH4kKW1gDdKm"));
        BOOST_CHECK(d8Pub.verify((char*)msg, strlen((char*)msg), d8Signature));

        gm::sm2::Signature d8SignatureErr(std::string("SIG_GM_LSQsvUrcvPcSZMz4stztPNpjqKcdedUyNKt5CWeGKdUSou2an4R6Z3UVsC3QKKX44pMRQsF6qGEpRfCkf9gDJN8tgrfLJN"));
        BOOST_CHECK(!d8Pub.verify((char*)msg, strlen((char*)msg), d8SignatureErr));
        gm::sm2::Signature d8SignatureErr2(std::string("SIG_GM_LJP7RPtyYPpuLoZiK49jUzuhfCcxtSZyt9eibhVGRT1CPa4mAHJfd2EPeiK5MR1tWYbrxULw2Cs7N9X1kECkE2MqAaLapv"));
        BOOST_CHECK(!d8Pub.verify((char*)msg, strlen((char*)msg), d8SignatureErr2));
}

BOOST_AUTO_TEST_SUITE_END()
