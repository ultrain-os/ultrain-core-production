#pragma once

#include <core/types.h>
#include <lightclient/BlsVoterSet.h>

namespace ultrainio {
    class ConfirmPoint {
    public:
        static bool isConfirmPoint(const BlockHeader& blockHeader);

        ConfirmPoint(const BlockHeader& blockHeader);

    private:
        BlockHeader m_blockHeader;

        // header_extensions
        BlsVoterSet m_blsVoterSet;
    };
}