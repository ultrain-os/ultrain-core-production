#define BOOST_TEST_MODULE evidencemultisign_unittest
#include <boost/test/included/unit_test.hpp>

#include <iostream>
#include <rpos/EvidenceFactory.h>
#include <rpos/EvidenceMultiSign.h>

using namespace ultrainio;

BOOST_AUTO_TEST_SUITE(evidencemultisign_unittest)

    BOOST_AUTO_TEST_CASE(normal) {
        AccountName acc("xiaofen");
        SignedBlockHeader a;
        a.proposer = AccountName("xiaofen");
        SignedBlockHeader b;
        b.proposer = AccountName("xiaofen");
        EvidenceMultiSign evidenceMultiSign(acc, a, b);
        std::cout << evidenceMultiSign.toString() << std::endl;
        std::shared_ptr<Evidence> evidence = EvidenceFactory::create(evidenceMultiSign.toString());
        std::cout << evidence->toString() << std::endl;
    }

BOOST_AUTO_TEST_SUITE_END()
