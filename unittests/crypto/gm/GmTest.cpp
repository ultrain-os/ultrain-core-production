#define BOOST_TEST_MODULE gm_test_suite
#include <boost/test/included/unit_test.hpp>

#include "gm/sm2/PrivateKey.h"
#include "gm/sm2/PublicKey.h"
#include "gm/sm2/Signature.h"
#include "gm/sm3/Sm3.h"
#include "gm/sm4/Sm4.h"
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

//// see sm2 spec
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

// sm3 spec
BOOST_AUTO_TEST_CASE(hash) {
    const unsigned char msg[] = {'a', 'b', 'c', 0};
    BOOST_CHECK(std::string(gm::sm3::Sm3::hash(msg, strlen((char*)msg))) == std::string("66c7f0f462eeedd9d1f2d46bdc10e4e24167c4875cf2f7a2297da02b8f4ba8e0"));

    const unsigned char msg2[] = "abcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcd";
    BOOST_CHECK(std::string(gm::sm3::Sm3::hash(msg2, strlen((char*)msg2))) == std::string("debe9ff92275b8a138604889c18e5a4d6fdb70e5387e5765293dcba39c0c5732"));
}

// sm4 spec
BOOST_AUTO_TEST_CASE(encrypt) {
        const uint8_t data[] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF, 0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10};
        const uint8_t key[]  = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF, 0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10};
        std::vector<char> encoded = gm::sm4::Sm4::cbcEncrypt(key, data, sizeof(data), false);
        BOOST_CHECK(ultrainio::Hex::toHex<uint8_t>((uint8_t*)encoded.data(), encoded.size()) == std::string("681edf34d206965e86b3e94f536e4246"));

        // decrypt
        int blockSize = 16; // bytes
        uint8_t enc[blockSize];
        ultrainio::Hex::fromHex<uint8_t>("681edf34d206965e86b3e94f536e4246", enc, blockSize);
        std::vector<char> decoded = gm::sm4::Sm4::cbcDecrypt(key, enc, blockSize, false);
        std::cout << "decoded size : " << decoded.size() << std::endl;
        BOOST_CHECK(decoded.size() == blockSize && memcmp(data, decoded.data(), blockSize) == 0);

        uint8_t encryptData[] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF, 0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10};
        const uint8_t encryptKey[]  = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF, 0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10};
        std::vector<char> tmp((char*)encryptData, (char*)encryptData + 16);
        std::vector<char> result;
        for (int i = 0; i < 1000000; i++) {
            result = gm::sm4::Sm4::cbcEncrypt(encryptKey, (uint8_t*)tmp.data(), tmp.size(), false);
            tmp = result;
        }
        BOOST_CHECK(ultrainio::Hex::toHex<uint8_t>((uint8_t*)result.data(), result.size()) == std::string("595298c7c6fd271f0402f804c33d3f66"));
}

BOOST_AUTO_TEST_CASE(EncDecPaddingGreateABlock) {
    uint8_t key[gm::sm4::Sm4::kCipherKeySize];
    std::string keyStr = "833c1a730486be4800e18b3b4943f6c8";
    ultrainio::Hex::fromHex<uint8_t>(keyStr, key, gm::sm4::Sm4::kCipherKeySize);
    int plainSize = 156;
    uint8_t plain[plainSize];
    std::string plainStr = "2000000040833c1a730486be4800e18b3b4943f6c8178a27fda535c20fdab021b754ff1ba80000000000000000000000000000000000000000000000000000000000000000010003a96a347e9eb7ac74dea836e765a94ea3341adfb847c234e19f01538a473e101033354a5a684146774e6e74655439324467716a4d5044386b597a47454d643842354743376761464e444a36475861387251506870";
    ultrainio::Hex::fromHex<uint8_t>(plainStr, plain, plainSize);

    std::vector<char> cipherActual = gm::sm4::Sm4::cbcEncrypt(key, plain, plainSize);
    std::cout << "cipher size : " << cipherActual.size() << " cipher: " << ultrainio::Hex::toHex<uint8_t>((uint8_t*)cipherActual.data(), cipherActual.size()) << std::endl;

    std::vector<char> plainActual = gm::sm4::Sm4::cbcDecrypt(key, (uint8_t*)cipherActual.data(), cipherActual.size());
        std::cout << "plain size : " << plainActual.size() << " plain: " << ultrainio::Hex::toHex<uint8_t>((uint8_t*)plainActual.data(), plainActual.size()) << std::endl;
    BOOST_CHECK(plainStr == ultrainio::Hex::toHex<uint8_t>((uint8_t*)plainActual.data(), plainActual.size()));
}

BOOST_AUTO_TEST_CASE(EncDecPaddingLessABlock) {
    uint8_t key[gm::sm4::Sm4::kCipherKeySize];
    std::string keyStr = "833c1a730486be4800e18b3b4943f6c8";
    ultrainio::Hex::fromHex<uint8_t>(keyStr, key, gm::sm4::Sm4::kCipherKeySize);
    int plainSize = 5;
    uint8_t plain[plainSize];
    std::string plainStr = "2000004083";
    ultrainio::Hex::fromHex<uint8_t>(plainStr, plain, plainSize);

    std::vector<char> cipherActual = gm::sm4::Sm4::cbcEncrypt(key, plain, plainSize);
    std::cout << "cipher size : " << cipherActual.size() << " cipher: " << ultrainio::Hex::toHex<uint8_t>((uint8_t*)cipherActual.data(), cipherActual.size()) << std::endl;

    std::vector<char> plainActual = gm::sm4::Sm4::cbcDecrypt(key, (uint8_t*)cipherActual.data(), cipherActual.size());
    std::cout << "plain size : " << plainActual.size() << " plain: " << ultrainio::Hex::toHex<uint8_t>((uint8_t*)plainActual.data(), plainActual.size()) << std::endl;
    BOOST_CHECK(plainStr == ultrainio::Hex::toHex<uint8_t>((uint8_t*)plainActual.data(), plainActual.size()));
}

BOOST_AUTO_TEST_CASE(EncDecPaddingEqualABlock) {
    uint8_t key[gm::sm4::Sm4::kCipherKeySize];
    std::string keyStr = "833c1a730486be4800e18b3b4943f6c8";
    ultrainio::Hex::fromHex<uint8_t>(keyStr, key, gm::sm4::Sm4::kCipherKeySize);
    int plainSize = gm::sm4::Sm4::kCipherBlockSize;
    uint8_t plain[plainSize];
    std::string plainStr = "20000040832000004083200000408399";
    ultrainio::Hex::fromHex<uint8_t>(plainStr, plain, plainSize);

    std::vector<char> cipherActual = gm::sm4::Sm4::cbcEncrypt(key, plain, plainSize);
    std::cout << "cipher size : " << cipherActual.size() << " cipher: " << ultrainio::Hex::toHex<uint8_t>((uint8_t*)cipherActual.data(), cipherActual.size()) << std::endl;

    std::vector<char> plainActual = gm::sm4::Sm4::cbcDecrypt(key, (uint8_t*)cipherActual.data(), cipherActual.size());
    std::cout << "plain size : " << plainActual.size() << " plain: " << ultrainio::Hex::toHex<uint8_t>((uint8_t*)plainActual.data(), plainActual.size()) << std::endl;
    BOOST_CHECK(plainStr == ultrainio::Hex::toHex<uint8_t>((uint8_t*)plainActual.data(), plainActual.size()));
}

BOOST_AUTO_TEST_SUITE_END()
