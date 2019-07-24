#include "rpos/EvilMultiSignDetector.h"

namespace ultrainio {
    bool EvilMultiSignDetector::hasMultiPropose(std::map<BlockIdType,
            ProposeMsg> proposerMsgMap, const ProposeMsg& propose) const {
        auto itor = proposerMsgMap.begin();
        for (auto itor = proposerMsgMap.begin(); itor != proposerMsgMap.end(); itor++) {
            if (itor->second.block.proposer == propose.block.proposer && itor->second.block.id() != propose.block.id()) {
                proposerMsgMap.erase(itor);
                return true;
            }
        }
        return false;
    }

    bool EvilMultiSignDetector::hasMultiVote(const EchoMsg& echo) {
        if (echo.phase == kPhaseBA1) {
            auto itor = m_committeeVoteBlock.find(echo.account);
            if (itor != m_committeeVoteBlock.end()) {
                if (itor->second != echo.blockId) {
                    return true;
                }
            } else {
                m_committeeVoteBlock.insert(make_pair(echo.account, echo.blockId));
            }
        }
        return false;
    }

    void EvilMultiSignDetector::reset() {
        m_committeeVoteBlock.clear();
    }
}
