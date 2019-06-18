#pragma once

#include <string>

#include <core/BlsVoterSet.h>
#include <core/types.h>
#include <ultrainio/chain/ultrainio_object.hpp>
#include <lightclient/CommitteeDelta.h>
#include <lightclient/CommitteeInfo.h>

namespace ultrainio {
    struct CommitteeInfo;
    class CommitteeSet {
    public:
        CommitteeSet();
        CommitteeSet(const std::string& s);
        CommitteeSet(const std::vector<char>& vc);
        CommitteeSet(const std::vector<CommitteeInfo>& committeeInfoV);
        CommitteeSet(const std::vector<chain::role_base>& roleBaseVector);
        bool verify(const BlsVoterSet& blsVoterSet) const;
        SHA256 committeeMroot() const;
        std::vector<char> toVectorChar() const;
        std::string toString() const;
        bool operator == (const CommitteeSet& rhs) const;
        bool operator != (const CommitteeSet& rhs) const;
        CommitteeDelta diff(const CommitteeSet& pre) const;
        std::vector<std::string> getBlsPk(const std::vector<AccountName>& accountV) const;

    private:
        std::vector<CommitteeInfo> m_committeeInfoV;
        void init(const std::string& s);
        int nextRoundThreshold() const;
    };
}