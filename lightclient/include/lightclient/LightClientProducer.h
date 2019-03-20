#pragma once

#include <core/types.h>
#include <lightclient/CheckPoint.h>
#include <lightclient/ConfirmPoint.h>
#include <lightclient/EpochEndPoint.h>

namespace ultrainio {

    // forward declare
    namespace chain {
        class controller;
    }

    class LightClientProducer {
    public:
        LightClientProducer();

        bool hasNextTobeConfirmedBls() const;

        void acceptNewHeader(const BlockHeader& blockHeader, const BlsVoterSet& blsVoterSet);

        void handleCheckPoint(chain::controller& chain, const CommitteeSet& committeeSet);

        void handleConfirmPoint(chain::controller& chain);

        void handleEpochEndPoint(chain::controller& chain, const SHA256& mroot);

    private:

        bool shouldBeConfirmed(const BlockHeader& blockHeader) const;

        BlsVoterSet nextTobeConfirmedBls() const;

        BlockIdType m_latestCheckPointId;

        BlockIdType m_latestConfirmedBlockId;

        std::list<BlockHeader> m_shouldBeConfirmedList;

        std::list<BlsVoterSet> m_shouldBeConfirmedBlsVoterSetList;

        int m_confirmedInterval = 20;
    };
}