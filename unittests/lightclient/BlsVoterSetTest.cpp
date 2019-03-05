#define BOOST_TEST_MODULE blsvoterset_test_suite
#include <boost/test/included/unit_test.hpp>

#include <iostream>
#include <fc/io/json.hpp>
#include <lightclient/BlsVoterSet.h>

using namespace ultrainio;
using namespace std;

BOOST_AUTO_TEST_SUITE(blsvoterset_test_suite)
    BOOST_AUTO_TEST_CASE(toVariants) {
        CommonEchoMsg commonEchoMsg;
        commonEchoMsg.blockId = fc::sha256("0000052af4157bf7f13c9f08305d6510053dbb58b1c33d0ea38a3e302c6e3287");
        commonEchoMsg.phase = kPhaseBA1;
        commonEchoMsg.baxCount = 10;
#ifdef CONSENSUS_VRF
        commonEchoMsg.proposerPriority = 0;
#else
        commonEchoMsg.proposer = AccountName(std::string("user.111"));
#endif
        BlsVoterSet blsVoterSet;
        blsVoterSet.commonEchoMsg = commonEchoMsg;
        blsVoterSet.accountPool.push_back(AccountName(std::string("user.112")));
        blsVoterSet.accountPool.push_back(AccountName(std::string("user.113")));
#ifdef CONSENSUS_VRF
        blsVoterSet.proofPool.push_back(std::string("proof1"));
        blsVoterSet.proofPool.push_back(std::string("proof2"));
#endif
        blsVoterSet.sigX = std::string("sigx");
        std::string restoreS = blsVoterSet.toString();
        cout << "restoreS " << restoreS << std::endl;
        BlsVoterSet restoreBlsVoterSet(restoreS);
        BOOST_CHECK(restoreBlsVoterSet.empty() == false);
        BOOST_CHECK(blsVoterSet == restoreBlsVoterSet);
    }
    BOOST_AUTO_TEST_SUITE_END()