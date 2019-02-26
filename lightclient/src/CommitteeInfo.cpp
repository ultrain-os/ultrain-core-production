#include <lightclient/CommitteeInfo.h>

namespace ultrainio {

    bool CommitteeInfo::isEmpty() {
        return accountName.empty() || pk.empty() || blsPk.empty();
    }

    void CommitteeInfo::toVariants(fc::variants& vs) const {
        vs.push_back(fc::variant(accountName));
        vs.push_back(fc::variant(pk));
        vs.push_back(fc::variant(blsPk));
    }

    void CommitteeInfo::fromVariants(const fc::variants& vs) {
        if (vs.size() < 3) {
            return;
        }
        accountName = vs[0].as<std::string>();
        pk = vs[1].as<std::string>();
        blsPk = vs[2].as<std::string>();
    }

    bool CommitteeInfo::operator == (const CommitteeInfo& rhs) const {
        if (this == &rhs) {
            return true;
        }
        return accountName == rhs.accountName && pk == rhs.pk && blsPk == rhs.blsPk;
    }
}