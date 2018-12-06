#define BOOST_TEST_MODULE roleselection_unittest
#include <boost/test/included/unit_test.hpp>

#include <random>
#include <rpos/Config.h>
#include <rpos/RoleRandom.h>
#include <rpos/RoleSelection.h>

using namespace ultrainio;

static std::vector<std::string> genCommitteeV(int size);

BOOST_AUTO_TEST_SUITE(fisheryates_unittest)

    BOOST_AUTO_TEST_CASE(lot_of_committee_member) {
        BlockIdType seed(std::string("0000052af4157bf7f13c9f08305d6510053dbb58b1c33d0ea38a3e302c6e3287"));
        RoleRandom r(seed);
        int committeeSize = 200;
        std::vector<std::string> committee = genCommitteeV(committeeSize);
        RoleSelection selection(committee, r);
        BOOST_CHECK(selection.proposerNumber() == Config::kDesiredProposerNumber);
        BOOST_CHECK(selection.voterNumber() == Config::kDesiredVoterNumber);
    }

    BOOST_AUTO_TEST_CASE(one_committee_member) {
        BlockIdType seed(std::string("0000052af4157bf7f13c9f08305d6510053dbb58b1c33d0ea38a3e302c6e3287"));
        RoleRandom r(seed);
        int committeeSize = 1;
        std::vector<std::string> committee = genCommitteeV(committeeSize);
        RoleSelection selection(committee, r);
        BOOST_CHECK(selection.proposerNumber() == committeeSize);
        BOOST_CHECK(selection.voterNumber() == committeeSize);

        BOOST_CHECK(selection.isProposer(committee[0]));
        BOOST_CHECK(selection.isVoter(committee[0], kPhaseBA0, 0));
    }

    BOOST_AUTO_TEST_CASE(proposer_plus_one_committee_member) {
        BlockIdType seed(std::string("0000052af4157bf7f13c9f08305d6510053dbb58b1c33d0ea38a3e302c6e3287"));
        RoleRandom r(seed);
        int committeeSize = Config::kDesiredProposerNumber + 1;
        std::vector<std::string> committee = genCommitteeV(committeeSize);
        BOOST_CHECK(committee.size() == committeeSize);
        RoleSelection selection(committee, r);
        BOOST_CHECK(selection.proposerNumber() == Config::kDesiredProposerNumber);
        BOOST_CHECK(selection.voterNumber() == committeeSize);

        int voterNumber = 0;
        int proposerNumber = 0;
        for (int i = 0; i < committeeSize; i++) {
            if (selection.isVoter(committee[i], kPhaseBA0, 0)) {
                voterNumber++;
            }
            if (selection.isProposer(committee[i])) {
                proposerNumber++;
            }
        }
        BOOST_CHECK(voterNumber == committeeSize);
        BOOST_CHECK(proposerNumber == Config::kDesiredProposerNumber);
    }


BOOST_AUTO_TEST_SUITE_END()

std::vector<std::string> genCommitteeV(int size) {
    static std::string c = "abcdefjhigklmnopqrstuvwxyz";
    std::vector<std::string> v;
    for (int n = 0; n < size; n++) {
        std::string account = std::string();
        for (int i = 0; i < 15; i++) {
            int index = random() % 26;
            account.push_back(c[index]);
        }
        std::cout << "account : " << account << std::endl;
        v.push_back(account);
    }
    return v;
}
