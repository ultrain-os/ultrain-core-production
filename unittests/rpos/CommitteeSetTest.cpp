#define BOOST_TEST_MODULE committeeset_test_suite
#include <boost/test/included/unit_test.hpp>

#include <iostream>
#include <fc/io/json.hpp>
#include <rpos/CommitteeInfo.h>
#include <rpos/CommitteeSet.h>

using namespace ultrainio;
using namespace std;

BOOST_AUTO_TEST_SUITE(committeeset_test_suite)
    BOOST_AUTO_TEST_CASE(toVectorChar) {
        std::vector<CommitteeInfo> committeeInfoV;
        CommitteeInfo committeeInfoA;
        committeeInfoA.accountName = std::string("name_A");
        committeeInfoA.pk = std::string("0000052af4157bf7f13c9f08305d6510053dbb58b1c33d0ea38a3e302c6e3287");
        committeeInfoA.blsPk = std::string("0000052af4157bf7f13c9f08305d6510053dbb58b1c33d0ea38a3e302c6e3287");
        CommitteeInfo committeeInfoB;
        committeeInfoB.accountName = std::string("name_B");
        committeeInfoB.pk = std::string("0000052af4157bf7f13c9f08305d6510053dbb58b1c33d0ea38a3e302c6e3287");
        committeeInfoB.blsPk = std::string("0000052af4157bf7f13c9f08305d6510053dbb58b1c33d0ea38a3e302c6e3287");
        committeeInfoV.push_back(committeeInfoA);
        committeeInfoV.push_back(committeeInfoB);

        CommitteeSet committeeSet(committeeInfoV);
        cout << "CommitteeSet : " << committeeSet.toString() << std::endl;

        CommitteeSet committeeSetR(committeeSet.toString());
        cout << "CommitteeSetR : " << committeeSetR.toString() << std::endl;
        BOOST_CHECK(committeeSet == committeeSetR);

        cout << "mroot " << std::string(committeeSet.committeeMroot()) << std::endl;
        BOOST_CHECK(SHA256::hash(committeeInfoV) == committeeSetR.committeeMroot());
        BOOST_CHECK(committeeSet.committeeMroot() == committeeSetR.committeeMroot());
    }
BOOST_AUTO_TEST_SUITE_END()