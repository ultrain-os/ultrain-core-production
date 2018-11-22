#include "rpos/RoleSelection.h"

#include <algorithm>

#include <rpos/Config.h>
#include <rpos/FisherYates.h>
#include <rpos/RoleRandom.h>

namespace ultrainio {
    RoleSelection::RoleSelection(const std::vector<std::string>& committeeV, const BlockIdType& rand) {
        ULTRAIN_ASSERT(committeeV.size() > 0, chain::chain_exception, "committee size");
        RoleRandom r(rand);
        FisherYates fys(r.toInt(), committeeV.size());
        std::vector<int> c = fys.shuffle();
        for (int i = 0; i < committeeV.size() && i < Config::kDesiredVoterNumber; i++) {
            int index = c[i];
            if (i < Config::kDesiredProposerNumber) {
                m_proposerV.push_back(committeeV[index]);
            }
            m_voterV.push_back(committeeV[index]);
        }
    }

    bool RoleSelection::isProposer(const std::string& account) {
        return std::find(m_proposerV.begin(), m_proposerV.end(), account) != m_proposerV.end();
    }

    bool RoleSelection::isVoter(const std::string& account) {
        return std::find(m_voterV.begin(), m_voterV.end(), account) != m_voterV.end();
    }

    int RoleSelection::proposerPriority(const std::string& account) {
        for (int i = 0; i < m_proposerV.size(); i++) {
            if (account == m_proposerV[i]) {
                return i;
            }
        }
        return m_proposerV.size();
    }

    int RoleSelection::proposerNumber() const {
        return m_proposerV.size();
    }

    int RoleSelection::voterNumber() const {
        return m_voterV.size();
    }
}