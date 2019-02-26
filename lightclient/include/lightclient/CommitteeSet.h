#pragma once

#include <string>

#include <core/types.h>
#include <lightclient/CommitteeInfo.h>

namespace ultrainio {
    struct CommitteeInfo;
    class CommitteeSet {
    public:
        CommitteeSet();
        CommitteeSet(const std::string& s);
        CommitteeSet(const std::vector<char>& vc);
        CommitteeSet(const std::vector<CommitteeInfo> committeeInfoV);
        SHA256 committeeMroot() const;
        std::vector<char> toVectorChar() const;
        std::string toString() const;
        bool operator == (const CommitteeSet& rhs) const;

    private:
        std::vector<CommitteeInfo> m_committeeInfoV;
        void init(const std::string& s);
    };
}