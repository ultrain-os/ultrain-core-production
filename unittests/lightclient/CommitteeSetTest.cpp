#define BOOST_TEST_MODULE committeeset_test_suite
#include <boost/test/included/unit_test.hpp>

#include <iostream>
#include <lightclient/CommitteeInfo.h>
#include <lightclient/CommitteeSet.h>

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
        BOOST_CHECK(committeeSet.committeeMroot() == committeeSetR.committeeMroot());
    }

    BOOST_AUTO_TEST_CASE(diff) {
        CommitteeInfo committeeInfoA;
        committeeInfoA.accountName = std::string("A");
        committeeInfoA.pk = std::string("pk_A");
        committeeInfoA.blsPk = std::string("bls_pk_A");

        CommitteeInfo committeeInfoB;
        committeeInfoB.accountName = std::string("B");
        committeeInfoB.pk = std::string("pk_B");
        committeeInfoB.blsPk = std::string("bls_pk_B");

        CommitteeInfo committeeInfoC;
        committeeInfoC.accountName = std::string("C");
        committeeInfoC.pk = std::string("pk_C");
        committeeInfoC.blsPk = std::string("bls_pk_C");

        CommitteeInfo committeeInfoD;
        committeeInfoD.accountName = std::string("D");
        committeeInfoD.pk = std::string("pk_D");
        committeeInfoD.blsPk = std::string("bls_pk_D");

        CommitteeInfo committeeInfoE;
        committeeInfoE.accountName = std::string("E");
        committeeInfoE.pk = std::string("pk_E");
        committeeInfoE.blsPk = std::string("bls_pk_E");

        std::vector<CommitteeInfo> preVector;
        preVector.push_back(committeeInfoA);
        preVector.push_back(committeeInfoB);
        preVector.push_back(committeeInfoC);
        CommitteeSet pre(preVector);

        std::vector<CommitteeInfo> currVector;
        currVector.push_back(committeeInfoC);
        currVector.push_back(committeeInfoD);
        currVector.push_back(committeeInfoE);
        CommitteeSet curr(currVector);

        CommitteeDelta delta = curr.diff(pre);

        std::list<CommitteeInfo> ba;
        ba.push_back(committeeInfoB);
        ba.push_back(committeeInfoA);

        std::list<CommitteeInfo> ed;
        ed.push_back(committeeInfoE);
        ed.push_back(committeeInfoD);

        BOOST_CHECK(delta.getAdd() == ed);
        BOOST_CHECK(delta.getRemoved() == ba);
    }

BOOST_AUTO_TEST_SUITE_END()