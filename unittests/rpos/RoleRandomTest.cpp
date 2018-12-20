#define BOOST_TEST_MODULE rolerandom_unittest
#include <boost/test/included/unit_test.hpp>

#include <iostream>
#include <rpos/RoleRandom.h>

using namespace ultrainio;


BOOST_AUTO_TEST_SUITE(rolerandom_unittest)

    BOOST_AUTO_TEST_CASE(toint) {
        BlockIdType seed(std::string("0000052af4157bf7f13c9f08305d6510053dbb58b1c33d0ea38a3e302c6e3287"));
        RoleRandom r1(seed, 1000);
        RoleRandom r2(seed, 1000);
        BOOST_CHECK(r1.getRand() == r2.getRand());

        RoleRandom r3(seed, 1001);
        BOOST_CHECK(r2.getRand() != r3.getRand());

        RoleRandom r4(seed, 1000, kPhaseBA1);
        BOOST_CHECK(r2.getRand() != r4.getRand());

        RoleRandom r5(seed, 1000, kPhaseBA0, 1);
        BOOST_CHECK(r2.getRand() != r5.getRand());

        BlockIdType seed1(std::string("0000052af4157bf7f13c9f08305d6510053dbb58b1c33d0ea38a3e302c6e1111"));
        RoleRandom r6(seed1, 1000);
        BOOST_CHECK(r2.getRand() != r6.getRand());
    }

BOOST_AUTO_TEST_SUITE_END()