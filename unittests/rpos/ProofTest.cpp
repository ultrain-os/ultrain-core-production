#define BOOST_TEST_MODULE proof_unittest
#include <boost/test/included/unit_test.hpp>

#include <iostream>

#include <core/Message.h>
#include <crypto/Bls.h>
#include <crypto/PrivateKey.h>
#include <crypto/PublicKey.h>
#include <rpos/Proof.h>
#include <rpos/Seed.h>
#include <rpos/Vrf.h>

using namespace ultrainio;
using namespace std;

// Proof
BOOST_AUTO_TEST_SUITE(proof_test_suite)

    BOOST_AUTO_TEST_CASE(normal_flow) {
        PrivateKey privateKey;
        PublicKey publicKey;
        PrivateKey::generate(privateKey, publicKey);
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
        for (int i = 0; i < Bls::BLS_SIGNATURE_LENGTH; i++) {
            faultHexStr1 += "0";
        }
        Proof faultProof1(faultHexStr1);
        BOOST_CHECK(faultProof1.isValid());
        BOOST_CHECK(proof.getPriority() != faultProof1.getPriority());
    }

BOOST_AUTO_TEST_SUITE_END()