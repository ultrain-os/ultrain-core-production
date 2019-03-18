#pragma once

#include <list>
#include <lightclient/CommitteeInfo.h>

namespace ultrainio {
    class CommitteeDelta {
    public:
        CommitteeDelta(const std::list<CommitteeInfo>& add, const std::list<CommitteeInfo>& removed);

        const std::list<CommitteeInfo>& getAdd() const { return m_add; }

        const std::list<CommitteeInfo>& getRemoved() const { return m_removed; }

    private:
        std::list<CommitteeInfo> m_add;
        std::list<CommitteeInfo> m_removed;
    };
}