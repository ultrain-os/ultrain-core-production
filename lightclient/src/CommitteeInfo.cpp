#include <lightclient/CommitteeInfo.h>

namespace ultrainio {

    const std::string CommitteeInfo::kDelimiters = std::string(" ");

    bool CommitteeInfo::isEmpty() {
        return accountName.empty() || pk.empty() || blsPk.empty();
    }

    void CommitteeInfo::toStrStream(std::string& s) const {
        s.append(accountName).append(kDelimiters);
        s.append(pk).append(kDelimiters);
        s.append(blsPk);
    }

    bool CommitteeInfo::fromStrStream(const std::string& s, size_t start, size_t& next) {
        std::size_t found = s.find(kDelimiters, start);

        if (found == std::string::npos) {
            return false;
        }
        accountName = s.substr(start, found - start);
        start = found + kDelimiters.size();

        found = s.find(kDelimiters, start);
        if (found == std::string::npos) {
            return false;
        }
        pk = s.substr(start, found - start);

        start = found + kDelimiters.size();

        if (start >= s.size()) {
            return false;
        }

        found = s.find(kDelimiters, start);

        if (found == std::string::npos) {
            blsPk = s.substr(start);
            next = std::string::npos;
        } else {
            blsPk = s.substr(start, found - start);
            next = found + kDelimiters.size();
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