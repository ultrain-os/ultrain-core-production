#pragma once

#include <list>
#include "CommitteeInfo.h"

namespace ultrainiosystem {
    class CommitteeDelta {
    public:
        CommitteeDelta::CommitteeDelta(const std::list<CommitteeInfo>& add, const std::list<CommitteeInfo>& removed)
                : m_add(add), m_removed(removed) {}

        const std::list<CommitteeInfo>& getAdd() const { return m_add; }

        const std::list<CommitteeInfo>& getRemoved() const { return m_removed; }

    private:
        std::list<CommitteeInfo> m_add;
        std::list<CommitteeInfo> m_removed;
    };
}