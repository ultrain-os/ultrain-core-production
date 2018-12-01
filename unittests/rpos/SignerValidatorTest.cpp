#define BOOST_TEST_MODULE signer_validator_unittest
#include <boost/test/included/unit_test.hpp>

#include <iostream>

#include <core/Message.h>
#include <core/Redefined.h>
#include <crypto/PrivateKey.h>
#include <crypto/PublicKey.h>
#include <rpos/Validator.h>
#include <rpos/Signer.h>

using namespace ultrainio;
using namespace std;

// Signer/Validator
BOOST_AUTO_TEST_SUITE(signer_validator_test_suite)

    BOOST_AUTO_TEST_CASE(normal) {
        PrivateKey privateKey = PrivateKey::generate();
        PublicKey publicKey = privateKey.getPublicKey();
        Block block;
        block.version = 1;
        block.proposer = N("ultr_genesis");
        block.signature = std::string(Signer::sign<BlockHeader>(block, privateKey));
        BOOST_CHECK(Validator::verify<BlockHeader>(Signature(block.signature), block, publicKey));

        EchoMsg echoMsg;
        echoMsg.baxCount = 0;
        echoMsg.account = N("ultr_test");
        echoMsg.signature = std::string(Signer::sign<UnsignedEchoMsg>(echoMsg, privateKey));
        BOOST_CHECK(Validator::verify<UnsignedEchoMsg>(Signature(echoMsg.signature), echoMsg, publicKey));
    }

BOOST_AUTO_TEST_SUITE_END()