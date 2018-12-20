#define BOOST_TEST_MODULE bls_test_suite
#include <boost/test/included/unit_test.hpp>

#include <iostream>

#include <base/Hex.h>
#include <crypto/Bls.h>

using namespace ultrainio;
using namespace std;

BOOST_AUTO_TEST_SUITE(bls_test_suite)
    BOOST_AUTO_TEST_CASE(aggragate) {
        std::string ultrain("ultrain");
        std::shared_ptr<Bls> blsPtr = Bls::getDefault();

        // keygen
        unsigned char sk[Bls::BLS_PRI_KEY_LENGTH];
        unsigned char pk[Bls::BLS_PUB_KEY_LENGTH];
        blsPtr->keygen(sk, Bls::BLS_PRI_KEY_LENGTH, pk, Bls::BLS_PUB_KEY_LENGTH);

        // sign
        unsigned char sig[Bls::BLS_SIGNATURE_LENGTH];
        blsPtr->signature(sk, (void*)ultrain.c_str(), 7, sig, Bls::BLS_SIGNATURE_LENGTH);
        BOOST_CHECK(blsPtr->verify(pk, sig, (void*)ultrain.c_str(), 7) == true);

        //
        unsigned char sk1[Bls::BLS_PRI_KEY_LENGTH];
        unsigned char pk1[Bls::BLS_PUB_KEY_LENGTH];
        blsPtr->keygen(sk1, Bls::BLS_PRI_KEY_LENGTH, pk1, Bls::BLS_PUB_KEY_LENGTH);

        unsigned char sig1[Bls::BLS_SIGNATURE_LENGTH];
        blsPtr->signature(sk1, (void*)ultrain.c_str(), 7, sig1, Bls::BLS_SIGNATURE_LENGTH);
        BOOST_CHECK(blsPtr->verify(pk1, sig1, (void*)ultrain.c_str(), 7) == true);

        //
        unsigned char sk2[Bls::BLS_PRI_KEY_LENGTH];
        unsigned char pk2[Bls::BLS_PUB_KEY_LENGTH];
        blsPtr->keygen(sk2, Bls::BLS_PRI_KEY_LENGTH, pk2, Bls::BLS_PUB_KEY_LENGTH);

        unsigned char sig2[Bls::BLS_SIGNATURE_LENGTH];
        blsPtr->signature(sk2, (void*)ultrain.c_str(), 7, sig2, Bls::BLS_SIGNATURE_LENGTH);
        BOOST_CHECK(blsPtr->verify(pk2, sig2, (void*)ultrain.c_str(), 7) == true);

        unsigned char* pks[3];
        unsigned char* sigs[3];
        pks[0] = pk;
        pks[1] = pk1;
        pks[2] = pk2;
        sigs[0] = sig;
        sigs[1] = sig1;
        sigs[2] = sig2;
        int vec[3];
        vec[0] = 1;
        vec[1] = 2;
        vec[2] = 3;
        BOOST_CHECK(blsPtr->aggVerify(pks, sigs, vec, 3, (void*)ultrain.c_str(), 7) == true);
    }

    BOOST_AUTO_TEST_CASE(diff_g) {
        std::shared_ptr<Bls> blsPtr = Bls::getDefault();
        unsigned char sk[Bls::BLS_PRI_KEY_LENGTH];
        unsigned char pk[Bls::BLS_PUB_KEY_LENGTH];
        blsPtr->keygen(sk, Bls::BLS_PRI_KEY_LENGTH, pk, Bls::BLS_PUB_KEY_LENGTH);

        unsigned char pkGot[Bls::BLS_PUB_KEY_LENGTH];
        blsPtr->getPk(pkGot, Bls::BLS_PUB_KEY_LENGTH, sk, Bls::BLS_PRI_KEY_LENGTH);
        BOOST_CHECK(Hex::toHex<unsigned char>(pk, Bls::BLS_PUB_KEY_LENGTH) == Hex::toHex<unsigned char>(pkGot, Bls::BLS_PUB_KEY_LENGTH));

        std::shared_ptr<Bls> blsPtrNullG = std::make_shared<Bls>(Bls::ULTRAINIO_BLS_DEFAULT_PARAM.c_str(), strlen(Bls::ULTRAINIO_BLS_DEFAULT_PARAM.c_str()));
        blsPtrNullG->initG(nullptr);
//        unsigned char skNullG[Bls::BLS_PRI_KEY_LENGTH];
//        unsigned char pkNullG[Bls::BLS_PUB_KEY_LENGTH];
//        blsPtrNullG->keygen(skNullG, Bls::BLS_PRI_KEY_LENGTH, pkNullG, Bls::BLS_PUB_KEY_LENGTH);

        unsigned char nullG[Bls::BLS_G_LENGTH];
        blsPtrNullG->getG(nullG, Bls::BLS_G_LENGTH);
        BOOST_CHECK(Hex::toHex<unsigned char>(nullG, Bls::BLS_G_LENGTH) != Bls::ULTRAINIO_BLS_DEFAULT_G);

        unsigned char pkGotNullG[Bls::BLS_PUB_KEY_LENGTH];
        blsPtrNullG->getPk(pkGotNullG, Bls::BLS_PUB_KEY_LENGTH, sk, Bls::BLS_PRI_KEY_LENGTH);
        BOOST_CHECK(Hex::toHex<unsigned char>(pk, Bls::BLS_PUB_KEY_LENGTH) != Hex::toHex<unsigned char>(pkGotNullG, Bls::BLS_PUB_KEY_LENGTH));
    }
BOOST_AUTO_TEST_SUITE_END()
