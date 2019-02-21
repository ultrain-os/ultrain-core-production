#pragma once

#include <core/types.h>
#include <rpos/BlsVoterSet.h>
#include <rpos/CommitteeSet.h>

namespace ultrainio {
    class Checkpoint {
    public:
        static bool isCheckpoint(const BlockHeader& blockHeader);

        Checkpoint(const BlockHeader& blockHeader);
    private:
        BlockHeader m_blockHeader;

        // header extensions
        BlockIdType m_preCheckpointBlockId;
        CommitteeSet m_committeeSet;
        // confirm the block before this checkpoint
        BlsVoterSet m_blsVoterSet;
    };
}
