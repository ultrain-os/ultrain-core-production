#pragma once

#include <string>
#include <vector>

#include <fc/reflect/reflect.hpp>

namespace ultrainio {
    struct CommitteeInfo {
        std::string accountName;
        std::string pk;
        std::string blsPk;
        bool isEmpty() {
            return accountName.empty() || pk.empty() || blsPk.empty();
        }
    };
    struct CommitteeState {
        std::vector<CommitteeInfo> cinfo;
        bool chainStateNormal = false;
        uint64_t chainMinStakeThresh {};
    };
}

FC_REFLECT( ultrainio::CommitteeInfo, (accountName)(pk)(blsPk))
