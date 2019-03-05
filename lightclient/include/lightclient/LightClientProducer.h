#pragma once

#include <core/types.h>
#include <lightclient/CheckPoint.h>
#include <lightclient/ConfirmPoint.h>
#include <lightclient/EpochEndPoint.h>

namespace ultrainio {
    class LightClientProducer {
    public:
        LightClientProducer();

        bool hasNextTobeConfirmedBlsVoterSet() const;

        BlsVoterSet nextTobeConfirmedBlsVoterSet() const;

        BlockIdType getLatestCheckPointId() const;

        void accept(const BlockHeader& blockHeader, const BlsVoterSet& blsVoterSet);

    private:

        bool shouldBeConfirmed(const BlockHeader& blockHeader) const;

        BlockIdType m_latestCheckPointId;

        BlockIdType m_latestConfirmedBlockId;

        std::list<BlockHeader> m_shouldBeConfirmedList;

        std::list<BlsVoterSet> m_shouldBeConfirmedBlsVoterSetList;

        int m_confirmedInterval = 20;

        uint32_t m_lastShouldBeConfirmedBlockNum = -1;
    };
}