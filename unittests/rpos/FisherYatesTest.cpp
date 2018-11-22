#define BOOST_TEST_MODULE fisheryates_unittest
#include <boost/test/included/unit_test.hpp>

#include <iostream>
#include <rpos/FisherYates.h>

using namespace ultrainio;


BOOST_AUTO_TEST_SUITE(fisheryates_unittest)

    BOOST_AUTO_TEST_CASE(shuffle) {
        FisherYates fys(90, 7);
        std::vector<int> v = fys.shuffle();
        for (auto i : v) {
            std::cout << i;
        }
        std::cout << std::endl;

        FisherYates fys1(91, 7);
        std::vector<int> v1 = fys1.shuffle();
        for (auto i : v1) {
            std::cout << i;
        }
        std::cout << std::endl;

        FisherYates fys2(98, 7);
        std::vector<int> v2 = fys.shuffle();
        for (auto i : v2) {
            std::cout << i;
        }
        std::cout << std::endl;

        FisherYates fys3(106, 7);
        std::vector<int> v3 = fys3.shuffle();
        for (auto i : v3) {
            std::cout << i;
        }
        std::cout << std::endl;
    }

BOOST_AUTO_TEST_SUITE_END()