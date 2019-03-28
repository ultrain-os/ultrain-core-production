#pragma once

#include <core/types.h>
#include <lightclient/CheckPoint.h>
#include <lightclient/ConfirmPoint.h>
#include <lightclient/EpochEndPoint.h>

namespace ultrainio {

    // forward declare
    namespace chain {
        class controller;

        namespace bls_votes {
            class bls_votes_manager;
        }
    }

    using chain::bls_votes::bls_votes_manager;

    class LightClientProducer {
    public:
        static void setConfirmPointInterval(int interval);

        LightClientProducer(bls_votes_manager& blsVotesManager);

        bool hasNextTobeConfirmedBls(BlsVoterSet& blsVoterSet) const;

        void acceptNewHeader(const BlockHeader& blockHeader, const BlsVoterSet& blsVoterSet);

        void handleCheckPoint(chain::controller& chain, const CommitteeSet& committeeSet);

        void handleConfirmPoint(chain::controller& chain, const BlsVoterSet& blsVoterSet);

        void handleEpochEndPoint(chain::controller& chain, const SHA256& mroot);

    private:
        bool shouldBeConfirmed(const BlockHeader& blockHeader) const;

        bls_votes_manager& m_BlsVotesMgr;
    };
}