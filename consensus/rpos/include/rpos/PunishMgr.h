#pragma once

#include <memory>
#include <vector>

#include <core/types.h>
#include <rpos/EvilType.h>

namespace ultrainio {
    struct Evil {
        AccountName accountName;
        int evil;
        Evil(const AccountName& n, int e);
    };

    class PunishMgr {
    public:
        static std::shared_ptr<PunishMgr> getInstance();

        PunishMgr();

        bool punish(const AccountName& accountName, int type);

        bool isPunished(const AccountName& accountName);

        //bool relievePunish(Evil evil);

    private:
        static std::shared_ptr<PunishMgr> s_self;

        std::vector<Evil> m_evils;
    };
}
