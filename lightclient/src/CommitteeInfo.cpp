#include <lightclient/CommitteeInfo.h>

namespace ultrainio {

    bool CommitteeInfo::isEmpty() {
        return accountName.empty() || pk.empty() || blsPk.empty();
    }

    void CommitteeInfo::toStrStream(std::stringstream& ss) const {
        ss << accountName << " ";
        ss << pk << " ";
        ss << blsPk;
    }

    bool CommitteeInfo::fromStrStream(std::stringstream& ss) {
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

    bool CommitteeInfo::operator == (const CommitteeInfo& rhs) const {
        if (this == &rhs) {
            return true;
        }
        return accountName == rhs.accountName && pk == rhs.pk && blsPk == rhs.blsPk;
    }
}