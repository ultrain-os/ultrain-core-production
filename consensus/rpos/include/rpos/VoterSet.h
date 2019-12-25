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

        VoterSet exclude(const std::vector<EchoMsg>& msgs) const;

        std::string toString() const;

        int getTotalVoterWeight() const;

        int size() const;

        EchoMsg get(size_t index) const;

    private:
        bool fromStringStream(std::stringstream& ss);

        void toStringStream(std::stringstream& ss) const;
    };

    typedef std::map<BlockIdType, VoterSet> BlockIdVoterSetMap;
}
