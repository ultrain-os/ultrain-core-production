#pragma once

#include <string>
#include <sstream>

namespace ultrainiosystem {
    struct CommitteeInfo {
        std::string accountName;
        std::string pk;
        std::string blsPk;
        bool isEmpty() {
            return accountName.empty() || pk.empty() || blsPk.empty();
        }
        void toStrStream(std::stringstream& ss) const {
            ss << accountName << " ";
            ss << pk << " ";
            ss << blsPk;
        }

        bool fromStrStream(std::stringstream& ss) {
            if(!(ss >> accountName)) {
                return false;
            }
            if(!(ss >> pk)) {
                return false;
            }
            if(!(ss >> blsPk)) {
                return false;
            }

            return true;
        }

        bool operator == (const CommitteeInfo& rhs) const {
            if (this == &rhs) {
                return true;
            }
            return accountName == rhs.accountName && pk == rhs.pk && blsPk == rhs.blsPk;
        }
    };
}
