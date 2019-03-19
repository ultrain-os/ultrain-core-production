#pragma once

#include <list>
#include "CommitteeInfo.h"

namespace ultrainiosystem {
    class CommitteeDelta {
    public:
        CommitteeDelta(const std::list<CommitteeInfo>& add, const std::list<CommitteeInfo>& removed)
                : m_add(add), m_removed(removed) {}

        const std::list<CommitteeInfo>& getAdd() const { return m_add; }

        const std::list<CommitteeInfo>& getRemoved() const { return m_removed; }

        bool checkAdded(const CommitteeInfo& cmt) {
            for(auto it = m_add.begin(); it != m_add.end(); ++it) {
                if(cmt == *it) {
                    it = m_add.erase(it);
                    return true;
                }
            }
            return false;
        }

        bool checkRemoved(const CommitteeInfo& cmt) {
            for(auto it = m_removed.begin(); it != m_removed.end(); ++it) {
                if(cmt == *it) {
                    it = m_removed.erase(it);
                    return true;
                }
            }
            return false;
        }

        bool empty() {
            return m_add.empty() && m_removed.empty();
        }

    private:
        std::list<CommitteeInfo> m_add;
        std::list<CommitteeInfo> m_removed;
    };
}
