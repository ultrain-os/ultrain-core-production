#define BOOST_TEST_MODULE votersystem_unittest
#include <boost/test/included/unit_test.hpp>

#include <boost/chrono.hpp>

#include <rpos/CommitteeInfo.h>
#include <rpos/Node.h>


using namespace ultrainio;
using namespace std;

static std::shared_ptr<CommitteeState> getCorrectCommitteeState();

static std::shared_ptr<CommitteeState> getFaultCommitteeState();

static std::shared_ptr<CommitteeState> getNullCommitteeState();

BOOST_AUTO_TEST_SUITE(votersystem_test_suite)


    BOOST_AUTO_TEST_CASE(getCommitteeMemberNumber) {
        UranusNode::GENESIS = boost::chrono::system_clock::now();
        //std::shared_ptr<VoterSystem> voterSystem = VoterSystem::create(nullptr);

        //BOOST_CHECK(voterSystem->getCommitteeMemberNumber() == 1);
    }

BOOST_AUTO_TEST_SUITE_END()

std::shared_ptr<CommitteeState> getCorrectCommitteeState() {
    std::shared_ptr<CommitteeState> committeeStatePtr = std::make_shared<CommitteeState>();
    committeeStatePtr->chainStateNormal = true;
    CommitteeInfo info;
    info.accountName = "ultr_qxf_2";
    info.pk = "b1823a3b041207f652370be3784145c214508a5cb3b8988633ae84aae9bd73db";
    committeeStatePtr->cinfo.push_back(info);

    info.accountName = "ultr_qxf_3";
    info.pk = "1931782abf7228f7dd15de1d41cae4f093e8819f47a651dca53322016fc74415";
    committeeStatePtr->cinfo.push_back(info);

    info.accountName = "ultr_qxf_4";
    info.pk = "7bdf53d74502fa20102a71d802b9f0dac3981069cfd45434dc120e94eaf45f5f";
    committeeStatePtr->cinfo.push_back(info);

    info.accountName = "ultr_qxf_5";
    info.pk = "162fecca374c87a110762eb2e5f173342204f193306c53007f0f157ebfc15f56";
    committeeStatePtr->cinfo.push_back(info);

    info.accountName = "ultr_qxf_6";
    info.pk = "e10cfd7c3e52f2960f85e83c8d05801807554f9541c989dbeee95cd12f19bac9";
    committeeStatePtr->cinfo.push_back(info);
    return committeeStatePtr;
}