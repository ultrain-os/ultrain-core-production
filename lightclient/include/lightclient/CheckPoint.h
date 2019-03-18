#pragma once

#include <core/types.h>
#include <lightclient/CommitteeSet.h>

namespace ultrainio {
    class CheckPoint {
    public:
        static bool isCheckPoint(const BlockHeader& blockHeader);

        CheckPoint();

        CheckPoint(const BlockHeader& blockHeader);

        uint32_t blockNum() const;

        CommitteeSet committeeSet() const;

        const BlockHeader& blockHeader() const;
    private:
        BlockHeader m_blockHeader;

        // header extensions
        BlockIdType m_preCheckPointBlockId;
        CommitteeSet m_committeeSet;
    };
}
