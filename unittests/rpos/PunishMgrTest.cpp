#define BOOST_TEST_MODULE punishmgr_unittest
#include <boost/test/included/unit_test.hpp>

#include <iostream>
#include <rpos/PunishMgr.h>

using namespace ultrainio;

BOOST_AUTO_TEST_SUITE(punishmgr_unittest)

    BOOST_AUTO_TEST_CASE(push) {
        std::shared_ptr<PunishMgr> punishMgrPtr = PunishMgr::getInstance();
        punishMgrPtr->punish(AccountName("qin"), EvilType::kSignMultiPropose);
        BOOST_CHECK(punishMgrPtr->isPunished(AccountName("qin")));
        BOOST_CHECK(!punishMgrPtr->isPunished(AccountName("li")));
    }

BOOST_AUTO_TEST_SUITE_END()