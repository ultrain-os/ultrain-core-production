#define BOOST_TEST_MODULE evidencemultisign_unittest
#include <boost/test/included/unit_test.hpp>

#include <iostream>
#include <rpos/EvidenceMultiSign.h>

using namespace ultrainio;

BOOST_AUTO_TEST_SUITE(evidencemultisign_unittest)

    BOOST_AUTO_TEST_CASE(normal) {
        AccountName acc("qin");
        SignedBlockHeader a;
        a.proposer = AccountName("xiaofen");
        SignedBlockHeader b;
        EvidenceMultiSign evidenceMultiSign(acc, a, b);
        std::cout << evidenceMultiSign.toString() << std::endl;
        //BOOST_CHECK();
    }

BOOST_AUTO_TEST_SUITE_END()
