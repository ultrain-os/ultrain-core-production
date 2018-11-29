#include <rpos/PunishMgr.h>

namespace ultrainio {

    Evil::Evil(const AccountName& n, int e) : accountName(n), evil(e) {

    }

    std::shared_ptr<PunishMgr> PunishMgr::s_self = nullptr;

    std::shared_ptr<PunishMgr> PunishMgr::getInstance() {
        if (!s_self) {
            s_self = std::make_shared<PunishMgr>();
        }
        return s_self;
    }

    PunishMgr::PunishMgr() : m_evils() {
    }

    bool PunishMgr::punish(const AccountName& accountName, int type) {
        bool found = false;
        for (auto& v : m_evils) {
            if (v.accountName == accountName) {
                found = true;
                v.evil |= type;
                break;
            }
        }
        if (!found) {
            m_evils.emplace_back(accountName, type);
        }
        return true;

    }

    bool PunishMgr::isPunished(const AccountName& accountName) {
        for (auto& v : m_evils) {
            if (v.accountName == accountName) {
                return true;
            }
        }
        return false;
    }
}