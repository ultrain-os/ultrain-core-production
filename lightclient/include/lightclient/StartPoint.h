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
        std::string genesisPk;
    };
}