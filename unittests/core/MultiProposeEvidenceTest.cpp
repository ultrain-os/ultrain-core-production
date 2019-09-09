#define BOOST_TEST_MODULE multiproposeevidence_unittest
#include <boost/test/included/unit_test.hpp>

#include <iostream>
#include <core/EvidenceFactory.h>
#include <core/MultiProposeEvidence.h>

using namespace ultrainio;

BOOST_AUTO_TEST_SUITE(multiproposeevidence_unittest)

    BOOST_AUTO_TEST_CASE(normal) {
        SignedBlockHeader a;
        a.proposer = AccountName("user.111");
        SignedBlockHeader b;
        b.proposer = AccountName("user.111");
        MultiProposeEvidence evidenceMultiPropose(a, b);
        std::cout << evidenceMultiPropose.toString() << std::endl;
        std::shared_ptr<Evidence> evidence = EvidenceFactory::create(evidenceMultiPropose.toString());
        std::cout << evidence->toString() << std::endl;
    }

BOOST_AUTO_TEST_SUITE_END()
