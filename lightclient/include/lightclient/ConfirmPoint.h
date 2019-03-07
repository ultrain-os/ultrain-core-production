#pragma once

#include <core/BlsVoterSet.h>
#include <core/types.h>

namespace ultrainio {
    class ConfirmPoint {
    public:
        static bool isConfirmPoint(const BlockHeader& blockHeader);

        ConfirmPoint(const BlockHeader& blockHeader);

        ConfirmPoint();

        BlockIdType confirmedBlockId() const;

        const BlsVoterSet& blsVoterSet() const;

    private:
        BlockHeader m_blockHeader;

        // header_extensions
        BlsVoterSet m_blsVoterSet;
    };
}