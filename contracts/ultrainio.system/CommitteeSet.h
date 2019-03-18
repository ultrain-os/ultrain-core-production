#pragma once

#include <string>

#include "CommitteeInfo.h"

namespace ultrainiosystem {
    class CommitteeSet {
    public:
        CommitteeSet() {}

        CommitteeSet(const std::string& s) {
            init(s);
        }

        CommitteeSet(const std::vector<char>& vc) {
            init(std::string(vc.begin(), vc.end()));
        }

        CommitteeSet(const std::vector<CommitteeInfo>& committeeInfoV)
                : m_committeeInfoV(committeeInfoV) {}

        std::vector<char> toVectorChar() const {
            std::string res = toString();
            std::vector<char> vc(res.size());
            vc.assign(res.begin(), res.end());
            return vc;
        }

        std::string toString() const {
            std::stringstream ss;
            for (int i = 0; i < m_committeeInfoV.size(); i++) {
                m_committeeInfoV[i].toStrStream(ss);
                if (i != m_committeeInfoV.size() -1) {
                    ss << " ";
                }
            }
            return ss.str();
        }

        bool operator == (const CommitteeSet& rhs) const {
            if (this == &rhs) {
                return true;
            }
            return m_committeeInfoV == rhs.m_committeeInfoV;
        }

        CommitteeDelta diff(const CommitteeSet& pre) const {
            std::list<CommitteeInfo> addCommitteeInfo;
            std::list<CommitteeInfo> removedCommitteeInfo;
            for (auto itor = m_committeeInfoV.begin(); itor != m_committeeInfoV.end(); itor++) {
                addCommitteeInfo.push_front(*itor);
            }
            for (auto itor = pre.m_committeeInfoV.begin(); itor != pre.m_committeeInfoV.end(); itor++) {
                addCommitteeInfo.remove(*itor);
                removedCommitteeInfo.push_front(*itor);
            }

            for (auto itor = m_committeeInfoV.begin(); itor != m_committeeInfoV.end(); itor++) {
                removedCommitteeInfo.remove(*itor);
            }

            return CommitteeDelta(addCommitteeInfo, removedCommitteeInfo);
        }

    private:
        void init(const std::string& s) {
            std::stringstream ss(s);
            CommitteeInfo committeeInfo;
            while(committeeInfo.fromStrStream(ss)) {
                m_committeeInfoV.push_back(committeeInfo);
            }
        }
        std::vector<CommitteeInfo> m_committeeInfoV;
    };
}