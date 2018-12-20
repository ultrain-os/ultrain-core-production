#pragma once

#include <string>
#include <vector>

#include <core/Message.h>
#include <core/Redefined.h>

namespace ultrainio {
    class RoleRandom;

    class RoleSelection {
    public:
        RoleSelection(const std::vector<std::string>& committeeV, const RoleRandom& rand);

        bool isProposer(const std::string& account);

        bool isVoter(const std::string& account);

        uint32_t proposerPriority(const std::string& account);

        uint32_t proposerNumber() const;

        int voterNumber() const;
    private:
        std::vector<std::string> m_voterV;
        std::vector<std::string> m_proposerV;
    };
}