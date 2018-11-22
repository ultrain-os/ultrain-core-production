#pragma once

#include <string>
#include <vector>

#include <core/Redefined.h>

namespace ultrainio {
    class RoleSelection {
    public:
        RoleSelection(const std::vector<std::string>& committeeV, const BlockIdType& rand);

        bool isProposer(const std::string& account);

        bool isVoter(const std::string& account);

        int proposerPriority(const std::string& account);

        int proposerNumber() const;

        int voterNumber() const;
    private:
        std::vector<std::string> m_voterV;
        std::vector<std::string> m_proposerV;
    };
}