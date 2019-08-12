#pragma once

#include <string>

#include "committee_delta.h"
namespace ultrainiosystem {
    class committee_set {
    public:
        committee_set() {}

        committee_set(const std::string& s) {
            init(s);
        }

        committee_set(const std::vector<char>& vc) {
            init(std::string(vc.begin(), vc.end()));
        }

        committee_set(const std::vector<committee_info>& committees)
                : m_committees(committees) {}

        std::vector<char> to_vector_char() const {
            std::string res = to_string();
            std::vector<char> vc(res.size());
            vc.assign(res.begin(), res.end());
            return vc;
        }

        bool empty() {
           return m_committees.empty();
        }

        std::string to_string() const {
            std::string k_delimiters = std::string(" ");
            std::string s;
            for (size_t i = 0; i < m_committees.size(); i++) {
                m_committees[i].to_strstream(s);
                if (i != m_committees.size() - 1) {
                    s.append(k_delimiters);
                }
            }
            return s;
        }

        bool operator == (const committee_set& rhs) const {
            if (this == &rhs) {
                return true;
            }
            return m_committees == rhs.m_committees;
        }

        committee_delta diff(const committee_set& pre) const {
            std::list<committee_info> add_committees;
            std::list<committee_info> removed_committees;
            for (auto itor = m_committees.begin(); itor != m_committees.end(); itor++) {
                add_committees.push_front(*itor);
            }
            for (auto itor = pre.m_committees.begin(); itor != pre.m_committees.end(); itor++) {
                add_committees.remove(*itor);
                removed_committees.push_front(*itor);
            }

            for (auto itor = m_committees.begin(); itor != m_committees.end(); itor++) {
                removed_committees.remove(*itor);
            }

            return committee_delta(add_committees, removed_committees);
        }

        void swap(std::vector<committee_info>& cmt_vct) {
            m_committees.swap(cmt_vct);
        }

    private:
        void init(const std::string& s) {
            committee_info cmt_info;
            size_t start = 0;
            size_t next = 0;
            while(cmt_info.from_strstream(s, start, next)) {
                m_committees.push_back(cmt_info);
                if (next == std::string::npos) {
                    return;
                }
                start = next;
            }
        }
        std::vector<committee_info> m_committees;
    };
}
