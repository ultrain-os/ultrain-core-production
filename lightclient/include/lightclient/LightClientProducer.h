#pragma once

#include <core/types.h>
#include <lightclient/CheckPoint.h>
#include <lightclient/ConfirmPoint.h>
#include <lightclient/EpochEndPoint.h>

namespace ultrainio {
    class LightClientProducer {
    public:
        LightClientProducer();

        bool shouldBeConfirmed(const BlockHeader& blockHeader) const;

        void addBlockHeaderAndBlsVoterSetPair(const BlockHeader& blockHeader, const BlsVoterSet& blsVoterSet);

        bool hasNextTobeConfirmedBlsVoterSet() const;

        BlsVoterSet nextTobeConfirmedBlsVoterSet() const;

        void acceptBlockHeader(const BlockHeader& blockHeader);

    private:
        std::list<BlockHeader> m_shouldBeConfirmedBlockHeaderList;

        std::list<BlsVoterSet> m_shouldBeConfirmedBlockHeaderBlsVoterSetList;
    };
}