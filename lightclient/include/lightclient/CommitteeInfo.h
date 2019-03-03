#pragma once

#include <string>
#include <sstream>

namespace ultrainio {
    struct CommitteeInfo {
        std::string accountName;
        std::string pk;
        std::string blsPk;
        bool isEmpty();
        void toStrStream(std::stringstream& ss) const;
        bool fromStrStream(std::stringstream& ss);
        bool operator == (const CommitteeInfo& rhs) const;
    };
}
