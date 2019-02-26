#pragma once

#include <core/types.h>
#include <lightclient/Checkpoint.h>
#include <lightclient/ConfirmPoint.h>
#include <lightclient/EpochEndPoint.h>

namespace ultrainio {
    class LightClient {
    public:
    private:
        // chain name
        uint64_t m_chainName;

        // the latest BlockHeader
        BlockHeader m_confirmedblockHeader;

        std::list<BlockHeader> m_unconfirmedBlockHeader;

        // the latest ConfirmPoint
        ConfirmPoint m_confirmPoint;

        Checkpoint m_secondLastestCheckpoint;

        Checkpoint m_lastestCheckpoint;

        EpochEndPoint m_epochEndPoint;
    };
}