#pragma once

#include <core/types.h>
#include <lightclient/CheckPoint.h>
#include <lightclient/CommitteeSet.h>
#include <lightclient/ConfirmPoint.h>
#include <lightclient/EpochEndPoint.h>
#include <lightclient/LightClientCallback.h>

namespace ultrainio {
    class LightClientCallback;

    class LightClient {
    public:
        LightClient(uint64_t chainName);

        uint64_t chainName() const;

        void reset();

        void accept(const BlockHeader& blockHeader);

        // invoked by fetch block feature
        void accept(const BlockHeader& blockHeader, const BlsVoterSet& blsVoterSet);

        BlockIdType getLatestConfirmedBlockId() const;

        bool setStartPoint(const CommitteeSet& committeeSet, const BlockIdType& blockId);

        void addCallback(std::shared_ptr<LightClientCallback> cb);

        bool getStatus() const;

    private:

        void confirm(const BlsVoterSet& blsVoterSet);

        void onError(const LightClientError& error, const BlockHeader&);

        void onConfirmed(const std::list<BlockHeader>&);

        bool isOutOfRange(const BlockHeader&) const;

        void handleGenesis(const BlockHeader&);

        bool verifyBlockHeaderList(const std::list<BlockHeader>& blockHeaderList, const BlsVoterSet& blsVoterSet);

        // chain name
        uint64_t m_chainName;

        std::shared_ptr<LightClientCallback> m_callback;

        CommitteeSet m_workingCommitteeSet;

        BlockIdType m_latestConfirmedBlockId;

        // sort by desc
        std::list<BlockHeader> m_unconfirmedList;

        // sort by asc
        std::list<BlockHeader> m_confirmedList;

        bool m_status;
    };
}