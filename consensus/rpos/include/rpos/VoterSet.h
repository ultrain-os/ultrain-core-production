#pragma once

#include <vector>
#include <core/BlsVoterSet.h>
#include <core/types.h>

namespace ultrainio {
    struct VoterSet {
        CommonEchoMsg commonEchoMsg;
        std::vector<AccountName> accountPool;
        std::vector<std::string> sigPool;
        std::vector<std::string> blsSignPool;
        std::vector<uint32_t>    timePool;
        bool hasSend = false;

        VoterSet();

        VoterSet(const std::string& str);

        bool empty() const;

        BlsVoterSet toBlsVoterSet(int weight) const;

        std::string generateSigX(const std::vector<std::string>& blsSignPool) const;

        VoterSet subVoterSet(int startIndex, int endIndex) const;

        VoterSet exclude(const std::vector<AccountName>& accounts) const;

        std::string toString() const;

        int getTotalVoterWeight() const;

    private:
        bool fromStringStream(std::stringstream& ss);

        void toStringStream(std::stringstream& ss) const;
    };
}
