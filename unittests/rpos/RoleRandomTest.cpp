#define BOOST_TEST_MODULE rolerandom_unittest
#include <boost/test/included/unit_test.hpp>

#include <iostream>
#include <rpos/RoleRandom.h>

using namespace ultrainio;


BOOST_AUTO_TEST_SUITE(rolerandom_unittest)

    BOOST_AUTO_TEST_CASE(toint) {
        BlockIdType seed1(std::string("0000052af4157bf7f13c9f08305d6510053dbb58b1c33d0ea38a3e302c6e3287"));
        RoleRandom r1(seed1);
        std::cout << "toInt : " << r1.toInt() << std::endl;

        BlockIdType seed2(std::string("0000052af4157bf7f13c9f08305d6510053dbb58b1c33d0ea38a3e302c6e3287"));
        RoleRandom r2(seed2);
        std::cout << "toInt : " << r2.toInt() << std::endl;
        BOOST_CHECK(r1.toInt() == r2.toInt());

        BlockIdType seed3(std::string("0000053f0153ff452cee9fec6d9c10d19e81937a3f17e9c3c58f4c4da746cf46"));
        RoleRandom r3(seed3);
        std::cout << "toInt : " << r3.toInt() << std::endl;
        BOOST_CHECK(r2.toInt() != r3.toInt());
    }

BOOST_AUTO_TEST_SUITE_END()