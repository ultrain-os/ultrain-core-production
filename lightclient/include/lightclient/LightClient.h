#pragma once

#include <core/types.h>
#include <lightclient/CheckPoint.h>
#include <lightclient/CommitteeSet.h>
#include <lightclient/ConfirmPoint.h>
#include <lightclient/EpochEndPoint.h>

namespace ultrainio {
    class LightClient {
    public:
        LightClient(uint64_t chainName);

        uint64_t chainName() const;

        void accept(const BlockHeader& blockHeader);

        std::list<BlockHeader> getConfirmedList() const;

        void clear();

    private:
        // chain name
        uint64_t m_chainName;

        CommitteeSet workingCommitteeSet;

        BlockIdType m_latestConfirmedBlockId;

        std::list<BlockHeader> m_unconfirmedList;

        std::list<CheckPoint> m_unconfirmedCheckPointList;

        std::list<EpochEndPoint> m_unconfirmedEpochEndPointList;

        std::list<BlockHeader> m_confirmedList;
    };
}