#pragma once

#include <core/types.h>
#include <lightclient/CheckPoint.h>
#include <lightclient/ConfirmPoint.h>
#include <lightclient/EpochEndPoint.h>

namespace ultrainio {
    class LightClient {
    public:
        LightClient(uint64_t chainName);

        uint64_t chainName() const;

        int accept(const BlockHeader& blockHeader);

    private:
        // chain name
        uint64_t m_chainName;

        // the latest BlockHeader
        BlockHeader m_confirmedblockHeader;

        std::list<BlockHeader> m_unconfirmedBlockHeaderList;

        // the latest ConfirmPoint
        ConfirmPoint m_confirmPoint;

        std::list<CheckPoint> m_checkPointList;

        std::list<EpochEndPoint> m_epochEndPointList;
    };
}