#include "rpos/SyncTask.h"

namespace ultrainio {
    SyncTask::SyncTask(uint32_t _checkBlock, const BlsVoterSet& _bvs, const SHA256& _nodeId, uint32_t _startBlock, uint32_t _endBlock, uint32_t _seqNum)
    : checkBlock(_checkBlock), bvs{_bvs}, nodeId(_nodeId), startBlock(_startBlock), endBlock(_endBlock), seqNum(_seqNum) {}
}