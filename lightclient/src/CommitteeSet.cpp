#include <lightclient/CommitteeSet.h>

#include <fc/io/json.hpp>
#include <fc/variant.hpp>

namespace ultrainio {
    CommitteeSet::CommitteeSet() {}

    CommitteeSet::CommitteeSet(const std::string& s) {
        init(s);
    }

    CommitteeSet::CommitteeSet(const std::vector<char>& vc) {
        init(std::string(vc.begin(), vc.end()));
    }

    void CommitteeSet::init(const std::string& s) {
        fc::variant v = fc::json::from_string(s);
        fc::variants vs = v.get_array();
        for (int i = 0; i < vs.size(); i++) {
            CommitteeInfo committeeInfo;
            committeeInfo.fromVariants(vs[i].get_array());
            m_committeeInfoV.push_back(committeeInfo);
        }
    }

    CommitteeSet::CommitteeSet(const std::vector<CommitteeInfo> committeeInfoV)
            : m_committeeInfoV(committeeInfoV) {}

    SHA256 CommitteeSet::committeeMroot() const {
        // MUST BE the same with StakeOverBase
        return SHA256::hash(m_committeeInfoV);
    }

    std::vector<char> CommitteeSet::toVectorChar() const {
        std::string res = toString();
        std::vector<char> vc(res.size());
        vc.assign(res.begin(), res.end());
        return vc;
    }

    std::string CommitteeSet::toString() const {
        std::vector<fc::variants> v(m_committeeInfoV.size());
        for (int i = 0; i < m_committeeInfoV.size(); i++) {
            fc::variants vs;
            m_committeeInfoV[i].toVariants(vs);
            v[i] = vs;
        }
        return fc::json::to_string(v);
    }

    bool CommitteeSet::operator == (const CommitteeSet& rhs) const {
        if (this == &rhs) {
            return true;
        }
        return m_committeeInfoV == rhs.m_committeeInfoV;
    }
}