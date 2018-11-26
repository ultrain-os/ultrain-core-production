#define BOOST_TEST_MODULE fisheryates_unittest
#include <boost/test/included/unit_test.hpp>

#include <iostream>

#include <rpos/Config.h>
#include <rpos/FisherYates.h>

using namespace ultrainio;


BOOST_AUTO_TEST_SUITE(fisheryates_unittest)

    BOOST_AUTO_TEST_CASE(shuffle_equal_rand) {
        FisherYates fys(90, 7);
        std::vector<int> v = fys.shuffle();
        for (auto i : v) {
            std::cout << i;
        }
        std::cout << std::endl;

        FisherYates fys1(90, 7);
        std::vector<int> v1 = fys1.shuffle();
        BOOST_CHECK(v == v1);

        FisherYates fys2(98, 7);
        std::vector<int> v2 = fys2.shuffle();
        for (auto i : v2) {
            std::cout << i;
        }
        std::cout << std::endl;
        BOOST_CHECK(v1 != v2);
    }

    BOOST_AUTO_TEST_CASE(shuffle_size_greate_ont_thousand) {
        FisherYates fys(2767, 10000);
        std::vector<int> v = fys.shuffle();
        for (auto i : v) {
            std::cout << i << "\t";
        }
        std::cout << std::endl;
    }

BOOST_AUTO_TEST_SUITE_END()