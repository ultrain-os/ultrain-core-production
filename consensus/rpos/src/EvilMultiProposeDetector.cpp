#include "rpos/EvilMultiProposeDetector.h"

#include <core/MultiProposeEvidence.h>

namespace ultrainio {
    bool EvilMultiProposeDetector::hasMultiPropose(std::map<BlockIdType,
            ProposeMsg>& proposerMsgMap, const ProposeMsg& propose, MultiProposeEvidence& outEvidence) const {
        auto itor = proposerMsgMap.begin();
        for (auto itor = proposerMsgMap.begin(); itor != proposerMsgMap.end(); itor++) {
            MultiProposeEvidence evidence(itor->second.block, propose.block);
            if (evidence.simpleVerify()) {
                proposerMsgMap.erase(itor);
                outEvidence = evidence;
                return true;
            }
        }
        return false;
    }
}
