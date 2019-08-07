#define BOOST_TEST_MODULE bls_test_suite
#include <boost/test/included/unit_test.hpp>

#include <iostream>

#include <base/Hex.h>
#include <crypto/Bls.h>
#include <crypto/PrivateKey.h>
#include <crypto/PublicKey.h>

using namespace ultrainio;
using namespace std;

BOOST_AUTO_TEST_SUITE(bls_test_suite)
    BOOST_AUTO_TEST_CASE(aggragate) {
        std::string ultrain("ultrain");
        std::shared_ptr<Bls> blsPtr = Bls::getDefault();

        // keygen
        unsigned char sk[Bls::BLS_PRI_KEY_LENGTH];
        unsigned char pk[Bls::BLS_PUB_KEY_COMPRESSED_LENGTH];
        blsPtr->keygen(sk, Bls::BLS_PRI_KEY_LENGTH, pk, Bls::BLS_PUB_KEY_COMPRESSED_LENGTH);

        // sign
        unsigned char sig[Bls::BLS_SIGNATURE_COMPRESSED_LENGTH];
        blsPtr->sign(sk, (void*)ultrain.c_str(), ultrain.length(), sig, Bls::BLS_SIGNATURE_COMPRESSED_LENGTH);
        BOOST_CHECK(blsPtr->verify(pk, sig, (void*)ultrain.c_str(), ultrain.length()) == true);

        //
        unsigned char sk1[Bls::BLS_PRI_KEY_LENGTH];
        unsigned char pk1[Bls::BLS_PUB_KEY_COMPRESSED_LENGTH];
        blsPtr->keygen(sk1, Bls::BLS_PRI_KEY_LENGTH, pk1, Bls::BLS_PUB_KEY_COMPRESSED_LENGTH);

        unsigned char sig1[Bls::BLS_SIGNATURE_COMPRESSED_LENGTH];
        blsPtr->sign(sk1, (void*)ultrain.c_str(), ultrain.length(), sig1, Bls::BLS_SIGNATURE_COMPRESSED_LENGTH);
        BOOST_CHECK(blsPtr->verify(pk1, sig1, (void*)ultrain.c_str(), ultrain.length()) == true);

        //
        unsigned char sk2[Bls::BLS_PRI_KEY_LENGTH];
        unsigned char pk2[Bls::BLS_PUB_KEY_COMPRESSED_LENGTH];
        blsPtr->keygen(sk2, Bls::BLS_PRI_KEY_LENGTH, pk2, Bls::BLS_PUB_KEY_COMPRESSED_LENGTH);

        unsigned char sig2[Bls::BLS_SIGNATURE_COMPRESSED_LENGTH];
        blsPtr->sign(sk2, (void*)ultrain.c_str(), ultrain.length(), sig2, Bls::BLS_SIGNATURE_COMPRESSED_LENGTH);
        BOOST_CHECK(blsPtr->verify(pk2, sig2, (void*)ultrain.c_str(), ultrain.length()) == true);

        unsigned char* pks[3];
        unsigned char* sigs[3];
        pks[0] = pk;
        pks[1] = pk1;
        pks[2] = pk2;
        sigs[0] = sig;
        sigs[1] = sig1;
        sigs[2] = sig2;
        unsigned char sigX[Bls::BLS_SIGNATURE_COMPRESSED_LENGTH];
        BOOST_CHECK(blsPtr->aggregate(sigs, 3, sigX, Bls::BLS_SIGNATURE_COMPRESSED_LENGTH) == true);
        BOOST_CHECK(blsPtr->verifyAggregate(pks, 3, sigX, Bls::BLS_SIGNATURE_COMPRESSED_LENGTH, (void*)ultrain.c_str(), ultrain.length()));
    }

    BOOST_AUTO_TEST_CASE(diff_g) {
        std::shared_ptr<Bls> blsPtr = Bls::getDefault();
        unsigned char sk[Bls::BLS_PRI_KEY_LENGTH];
        unsigned char pk[Bls::BLS_PUB_KEY_COMPRESSED_LENGTH];
        blsPtr->keygen(sk, Bls::BLS_PRI_KEY_LENGTH, pk, Bls::BLS_PUB_KEY_COMPRESSED_LENGTH);

        unsigned char pkGot[Bls::BLS_PUB_KEY_COMPRESSED_LENGTH];
        blsPtr->getPk(pkGot, Bls::BLS_PUB_KEY_COMPRESSED_LENGTH, sk, Bls::BLS_PRI_KEY_LENGTH);
        std::cout << "sk : " << Hex::toHex<unsigned char>(sk, Bls::BLS_PRI_KEY_LENGTH) << std::endl;
        std::cout << "pk : " << Hex::toHex<unsigned char>(pk, Bls::BLS_PUB_KEY_COMPRESSED_LENGTH) << std::endl;
        BOOST_CHECK(Hex::toHex<unsigned char>(pk, Bls::BLS_PUB_KEY_COMPRESSED_LENGTH) == Hex::toHex<unsigned char>(pkGot, Bls::BLS_PUB_KEY_COMPRESSED_LENGTH));

        std::shared_ptr<Bls> blsPtrNullG = std::make_shared<Bls>(Bls::ULTRAINIO_BLS_DEFAULT_PARAM.c_str(), strlen(Bls::ULTRAINIO_BLS_DEFAULT_PARAM.c_str()));
        blsPtrNullG->initG(nullptr);
//        unsigned char skNullG[Bls::BLS_PRI_KEY_LENGTH];
//        unsigned char pkNullG[Bls::BLS_PUB_KEY_COMPRESSED_LENGTH];
//        blsPtrNullG->keygen(skNullG, Bls::BLS_PRI_KEY_LENGTH, pkNullG, Bls::BLS_PUB_KEY_COMPRESSED_LENGTH);

        unsigned char nullG[Bls::BLS_G_LENGTH];
        blsPtrNullG->getG(nullG, Bls::BLS_G_LENGTH);
        BOOST_CHECK(Hex::toHex<unsigned char>(nullG, Bls::BLS_G_LENGTH) != Bls::ULTRAINIO_BLS_DEFAULT_G);

        unsigned char pkGotNullG[Bls::BLS_PUB_KEY_COMPRESSED_LENGTH];
        blsPtrNullG->getPk(pkGotNullG, Bls::BLS_PUB_KEY_COMPRESSED_LENGTH, sk, Bls::BLS_PRI_KEY_LENGTH);
        BOOST_CHECK(Hex::toHex<unsigned char>(pk, Bls::BLS_PUB_KEY_COMPRESSED_LENGTH) != Hex::toHex<unsigned char>(pkGotNullG, Bls::BLS_PUB_KEY_COMPRESSED_LENGTH));
    }

    BOOST_AUTO_TEST_CASE(verifyKeyPair) {
        std::string sk("53e3817e09f52a96a205eb5aaf3f69ac377dba2e");
        std::string fakeSk("44e3817e09f52a96a205eb5aaf3f69ac377dba2e");
        std::string pk("89f667384bbbb701424c349b65c33ebfcc7bc66936001bbfae7180a39250447f50068339f8317ab7890c88affb4e7fc26d42233419f71bae9bbc58b6057c5b8e00");
        std::string fakePk("77f667384bbbb701424c349b65c33ebfcc7bc66936001bbfae7180a39250447f50068339f8317ab7890c88affb4e7fc26d42233419f71bae9bbc58b6057c5b8e00");
        std::shared_ptr<Bls> blsPtr = Bls::getDefault();
        unsigned char skChar[Bls::BLS_PRI_KEY_LENGTH];
        unsigned char pkChar[Bls::BLS_PUB_KEY_COMPRESSED_LENGTH];
        unsigned char fakePkChar[Bls::BLS_PUB_KEY_COMPRESSED_LENGTH];
        unsigned char fakeSkChar[Bls::BLS_PRI_KEY_LENGTH];
        Hex::fromHex<unsigned char>(sk, skChar, Bls::BLS_PRI_KEY_LENGTH);
        Hex::fromHex<unsigned char>(pk, pkChar, Bls::BLS_PUB_KEY_COMPRESSED_LENGTH);
        Hex::fromHex<unsigned char>(fakeSk, fakeSkChar, Bls::BLS_PRI_KEY_LENGTH);
        Hex::fromHex<unsigned char>(fakePk, fakePkChar, Bls::BLS_PUB_KEY_COMPRESSED_LENGTH);
        BOOST_CHECK(blsPtr->verifyKeyPair(skChar, Bls::BLS_PRI_KEY_LENGTH, pkChar, Bls::BLS_PUB_KEY_COMPRESSED_LENGTH) == true);
        BOOST_CHECK(blsPtr->verifyKeyPair(fakeSkChar, Bls::BLS_PRI_KEY_LENGTH, pkChar, Bls::BLS_PUB_KEY_COMPRESSED_LENGTH) == false);
        BOOST_CHECK(blsPtr->verifyKeyPair(skChar, Bls::BLS_PRI_KEY_LENGTH, fakePkChar, Bls::BLS_PUB_KEY_COMPRESSED_LENGTH) == false);
    }
BOOST_AUTO_TEST_SUITE_END()
