#pragma once

#include <string>
#include <vector>
namespace ultrainio {
    struct CommitteeInfo {
        std::string accountName;
        std::string pk;
        int64_t stakesCount;
    };
    struct CommitteeState {
        std::vector<CommitteeInfo> cinfo;
        bool chainStateNormal = false;
    };
}

