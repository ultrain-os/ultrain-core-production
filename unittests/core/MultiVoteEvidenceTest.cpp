#define BOOST_TEST_MODULE multivoteevidence_unittest
#include <boost/test/included/unit_test.hpp>

#include <iostream>
#include <core/EvidenceFactory.h>
#include <core/MultiVoteEvidence.h>

using namespace ultrainio;

BOOST_AUTO_TEST_SUITE(multivoteevidence_unittest)

    BOOST_AUTO_TEST_CASE(normal) {
        EchoMsg a;
        a.proposer = AccountName("user.111");
        EchoMsg b;
        b.proposer = AccountName("user.111");
        MultiVoteEvidence evidenceMultiVote(a, b);
        std::cout << evidenceMultiVote.toString() << std::endl;
        std::shared_ptr<Evidence> evidence = EvidenceFactory::create(evidenceMultiVote.toString());
        std::cout << evidence->toString() << std::endl;
    }

BOOST_AUTO_TEST_SUITE_END()