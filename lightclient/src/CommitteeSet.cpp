#include <lightclient/CommitteeSet.h>

namespace ultrainio {
    CommitteeSet::CommitteeSet() {}

    CommitteeSet::CommitteeSet(const std::string& s) {
        init(s);
    }

    CommitteeSet::CommitteeSet(const std::vector<char>& vc) {
        init(std::string(vc.begin(), vc.end()));
    }

    void CommitteeSet::init(const std::string& s) {
        std::stringstream ss(s);
        CommitteeInfo committeeInfo;
        while(committeeInfo.fromStrStream(ss)) {
           m_committeeInfoV.push_back(committeeInfo);
        }
    }

    CommitteeSet::CommitteeSet(const std::vector<CommitteeInfo>& committeeInfoV)
            : m_committeeInfoV(committeeInfoV) {}

    CommitteeSet::CommitteeSet(const std::vector<chain::role_base>& roleBaseVector) {
        for (auto e : roleBaseVector) {
            CommitteeInfo info;
            info.accountName = std::string(e.owner);
            info.blsPk = e.bls_key;
            info.pk = e.producer_key;
            m_committeeInfoV.push_back(info);
        }
    }

    bool CommitteeSet::verify(const BlsVoterSet& blsVoterSet) const {
        std::vector<std::string> blsPkV = getBlsPk(blsVoterSet.accountPool);
        return blsVoterSet.verifyBls(blsPkV);
    }

    std::vector<std::string> CommitteeSet::getBlsPk(const std::vector<AccountName>& accountV) const {
        std::vector<std::string> pkV;
        for (auto v : accountV) {
            for (auto info : m_committeeInfoV) {
                if (std::string(v) == info.accountName) {
                    pkV.push_back(info.blsPk);
                    break;
                }
            }

        }
        return pkV;
    }

    SHA256 CommitteeSet::committeeMroot() const {
        // MUST BE the same with StakeOverBase
        return SHA256::hash(toString());
    }

    std::vector<char> CommitteeSet::toVectorChar() const {
        std::string res = toString();
        std::vector<char> vc(res.size());
        vc.assign(res.begin(), res.end());
        return vc;
    }

    std::string CommitteeSet::toString() const {
        std::stringstream ss;
        for (int i = 0; i < m_committeeInfoV.size(); i++) {
            m_committeeInfoV[i].toStrStream(ss);
            if (i != m_committeeInfoV.size() -1) {
                ss << " ";
            }
        }
        return ss.str();
    }

    bool CommitteeSet::operator == (const CommitteeSet& rhs) const {
        if (this == &rhs) {
            return true;
        }
        return m_committeeInfoV == rhs.m_committeeInfoV;
    }

    bool CommitteeSet::operator != (const CommitteeSet& rhs) const {
        return !(*this == rhs);
    }

    CommitteeDelta CommitteeSet::diff(const CommitteeSet& pre) const {
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
}