#include "rpos/EvilMultiVoteDetector.h"

namespace ultrainio {
    bool EvilMultiVoteDetector::hasMultiVote(BlockIdVoterSetMap& blockIdVoterSetMap, const EchoMsg& echo, MultiVoteEvidence& outEvidence) {
        for (auto itor = blockIdVoterSetMap.begin(); itor != blockIdVoterSetMap.end(); itor++) {
            if (itor->first != echo.blockId) {
                for (int i = 0; i < itor->second.size(); i++) {
                    EchoMsg a = itor->second.get(i);
                    MultiVoteEvidence evidence(a, echo);
                    if (evidence.simpleVerify()) {
                        outEvidence = evidence;
                        return true;
                    }
                }
            }
        }
        return false;
    }
}