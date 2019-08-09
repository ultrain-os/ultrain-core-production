#pragma once

#include "core/BlsVoterSet.h"

namespace ultrainio {
    struct SyncTask {
        uint32_t checkBlock;
        BlsVoterSet bvs;
        SHA256 nodeId;
        uint32_t startBlock;
        uint32_t endBlock;
        uint32_t seqNum;

        SyncTask(uint32_t _checkBlock, const BlsVoterSet& _bvs, const SHA256& _nodeId,
                uint32_t _startBlock, uint32_t _endBlock, uint32_t _seqNum);
    };
}