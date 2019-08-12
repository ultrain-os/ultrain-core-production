#pragma once

#include <list>
#include "committee_info.h"

namespace ultrainiosystem {
    class committee_delta {
    public:
        committee_delta(const std::list<committee_info>& add, const std::list<committee_info>& remove)
                : m_add(add), m_remove(remove) {}

        const std::list<committee_info>& get_add() const { return m_add; }

        const std::list<committee_info>& get_remove() const { return m_remove; }

        bool check_added(const committee_info& cmt) {
            for(auto it = m_add.begin(); it != m_add.end(); ++it) {
                if(cmt == *it) {
                    it = m_add.erase(it);
                    return true;
                }
            }
            return false;
        }

        bool check_removed(const committee_info& cmt) {
            for(auto it = m_remove.begin(); it != m_remove.end(); ++it) {
                if(cmt == *it) {
                    it = m_remove.erase(it);
                    return true;
                }
            }
            return false;
        }

        bool empty() {
            return m_add.empty() && m_remove.empty();
        }

    private:
        std::list<committee_info> m_add;
        std::list<committee_info> m_remove;
    };
}
