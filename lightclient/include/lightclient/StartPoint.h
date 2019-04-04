#pragma once

#include <string>
#include <vector>

#include <core/types.h>
#include <lightclient/CommitteeSet.h>

namespace ultrainio {
    class StartPoint {
    public:
        StartPoint(const CommitteeSet& cs, const BlockIdType& bid) : committeeSet(cs), lastConfirmedBlockId(bid) {}

        StartPoint() {}

        CommitteeSet committeeSet;
        BlockIdType lastConfirmedBlockId;
        std::string nextCommitteeMroot = std::string();
        std::string genesisPk = std::string("369c31f242bfc5093815511e4a4eda297f4b8772a7ff98f7806ce7a80ffffb35");
    };
}