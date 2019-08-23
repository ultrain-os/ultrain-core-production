#define BOOST_TEST_MODULE multisignevidence_unittest
#include <boost/test/included/unit_test.hpp>

#include <iostream>
#include <core/EvidenceFactory.h>
#include <core/MultiSignEvidence.h>

using namespace ultrainio;

BOOST_AUTO_TEST_SUITE(multisignevidence_unittest)

    BOOST_AUTO_TEST_CASE(normal) {
        SignedBlockHeader a;
        a.proposer = AccountName("user.111");
        SignedBlockHeader b;
        b.proposer = AccountName("user.111");
        MultiSignEvidence evidenceMultiSign(a, b);
        std::cout << evidenceMultiSign.toString() << std::endl;
        std::shared_ptr<Evidence> evidence = EvidenceFactory::create(evidenceMultiSign.toString());
        std::cout << evidence->toString() << std::endl;
    }

BOOST_AUTO_TEST_SUITE_END()
