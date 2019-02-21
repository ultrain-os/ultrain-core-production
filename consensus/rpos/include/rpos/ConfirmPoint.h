#pragma once

#include <core/types.h>

namespace ultrainio {
    class ConfirmPoint {
    public:
        ConfirmPoint(const BlockHeader& blockHeader);

    private:
        BlockHeader m_blockHeader;

        // header_extensions
        BlsVoterSet m_blsVoterSet;
    };
}