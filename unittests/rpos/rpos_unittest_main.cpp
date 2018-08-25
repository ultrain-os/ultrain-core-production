#define BOOST_TEST_MODULE rpos_unittest
#include <boost/test/included/unit_test.hpp>

#include <iostream>

#include <core/Message.h>
#include <crypto/Ed25519.h>
#include <crypto/PrivateKey.h>
#include <crypto/PublicKey.h>
#include <rpos/Proof.h>
#include <rpos/Seed.h>
#include <rpos/Vrf.h>
#include <rpos/Validator.h>
#include <rpos/Signer.h>

using namespace ultrainio;
using namespace std;

// Signer/Validator
BOOST_AUTO_TEST_SUITE(signervalidator_test_suite)

    BOOST_AUTO_TEST_CASE(normal_flow) {
        PrivateKey privateKey = PrivateKey::generate();
        PublicKey publicKey = privateKey.getPublicKey();
        Block block;
        block.version = 1;
        block.proposerPk = std::string(publicKey);
        block.signature = std::string(Signer::sign<BlockHeader>(block, privateKey));
        BOOST_CHECK(Validator::verify<BlockHeader>(Signature(block.signature), block, publicKey));

        EchoMsg echoMsg;
        echoMsg.baxCount = 0;
        echoMsg.pk = std::string(publicKey);
        echoMsg.signature = std::string(Signer::sign<UnsignedEchoMsg>(echoMsg, privateKey));
        BOOST_CHECK(Validator::verify<UnsignedEchoMsg>(Signature(echoMsg.signature), echoMsg, publicKey));
    }

BOOST_AUTO_TEST_SUITE_END()


// Proof
BOOST_AUTO_TEST_SUITE(proof_test_suite)

    BOOST_AUTO_TEST_CASE(normal_flow) {
        PrivateKey privateKey = PrivateKey::generate();
        PublicKey publicKey = privateKey.getPublicKey();
        Seed seed(std::string("preHash"), 40, static_cast<ConsensusPhase>(1), 0);
        Proof proof = Vrf::vrf(privateKey, seed, Vrf::kProposer);

        BOOST_CHECK(proof.getRand() > 0.0 && proof.getRand() < 1.0);
        BOOST_CHECK(Vrf::verify(publicKey, proof, seed, Vrf::kProposer));
        BOOST_CHECK(!Vrf::verify(publicKey, proof, seed, Vrf::kVoter));

        string hexProofStr = std::string(proof);
        BOOST_CHECK(Proof(hexProofStr).getPriority() == proof.getPriority());

        string faultHexStr(hexProofStr.data(), hexProofStr.length() - 8);
        BOOST_CHECK(!Proof(faultHexStr).isValid());
        faultHexStr += "00000000"; // 8
        Proof faultProof(faultHexStr);
        BOOST_CHECK(faultProof.getPriority() == faultProof.getPriority());
        BOOST_CHECK(!Vrf::verify(publicKey, faultProof, seed, Vrf::kProposer));
        string faultHexStr1;
        for (int i = 0; i < 2 * Ed25519::SIGNATURE_LEN; i++) {
            faultHexStr1 += "0";
        }
        Proof faultProof1(faultHexStr1);
        BOOST_CHECK(faultProof1.isValid());
        BOOST_CHECK(proof.getPriority() != faultProof1.getPriority());
    }

BOOST_AUTO_TEST_SUITE_END()