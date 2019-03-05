#pragma once

#include <core/types.h>
#include <lightclient/CheckPoint.h>
#include <lightclient/CommitteeSet.h>
#include <lightclient/ConfirmPoint.h>
#include <lightclient/EpochEndPoint.h>

namespace ultrainio {
    class LightClientCallback;

    class LightClient {
    public:
        LightClient(uint64_t chainName);

        uint64_t chainName() const;

        void accept(const BlockHeader& blockHeader);

        void addCallback(std::shared_ptr<LightClientCallback> cb);

        void clear();

    private:
        // chain name
        uint64_t m_chainName;

        std::shared_ptr<LightClientCallback> m_callback;

        CommitteeSet workingCommitteeSet;

        BlockIdType m_latestConfirmedBlockId;

        // sort by desc
        std::list<BlockHeader> m_unconfirmedList;

        // sort by desc
        std::list<CheckPoint> m_unconfirmedCheckPointList;

        // sort by desc
        std::list<EpochEndPoint> m_unconfirmedEpochEndPointList;

        std::list<BlockHeader> m_confirmedList;
    };
}