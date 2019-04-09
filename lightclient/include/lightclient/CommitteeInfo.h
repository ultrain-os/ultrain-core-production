#pragma once

#include <string>

namespace ultrainio {
    struct CommitteeInfo {
        static const std::string kDelimiters;

        std::string accountName;
        std::string pk;
        std::string blsPk;
        bool isEmpty();
        void toStrStream(std::string& s) const;
        bool fromStrStream(const std::string& s, size_t start, size_t& next);
        bool operator == (const CommitteeInfo& rhs) const;
    };
}
