#pragma once

#include <string>
#include <vector>
namespace ultrainio {
    struct CommitteeInfo {
        std::string accountName;
        std::string pk;
        std::string blsPk;
        int64_t stakesCount;
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

