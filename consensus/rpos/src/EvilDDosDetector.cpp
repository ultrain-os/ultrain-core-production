#include "rpos/EvilDDosDetector.h"

namespace ultrainio {

    int EvilDDosDetector::s_twoPhase = 2;

    void EvilDDosDetector::deduceBlockNum(const VoterSet& voterSet, int f, uint32_t now, ConsensusPhase phase) {
        // reset
        m_expiry = 0;
        if (voterSet.empty()) {
            return;
        }
        if (voterSet.commonEchoMsg.phase == kPhaseBA1 && phase == kPhaseBA1) {
            int behindCount = 0;
            int frontTwoPhase = 0;
            uint32_t when = now - 1; // the time to vote
            for (auto e : voterSet.timePool) {
                if (e == when) { // the same time slot
                    behindCount++;
                } else if (e < when - 1) {
                    frontTwoPhase++;
                }
            }
            if (behindCount >= f + 1) {
                m_maxBlockNum = BlockHeader::num_from_id(voterSet.commonEchoMsg.blockId) + 2;
                m_expiry = now + s_twoPhase;
            } else if (frontTwoPhase >= 2 * f + 1) {
                std::vector<uint32_t> v(voterSet.timePool.begin(), voterSet.timePool.end());
                std::sort(v.begin(), v.end());
                uint32_t middle = v[v.size()/2];
                uint32_t fallBlockNum = (now - middle) / 2;
                m_maxBlockNum = BlockHeader::num_from_id(voterSet.commonEchoMsg.blockId) + 2 + fallBlockNum;
                m_expiry = now + s_twoPhase;
            }
        }
        clear();
    }

    bool EvilDDosDetector::evil(const EchoMsg& echo, uint32_t now, uint32_t localBlockNum) const {
        if (stillEffect(now)) {
            uint32_t blockNum = BlockHeader::num_from_id(echo.blockId);
            return  blockNum > m_maxBlockNum && blockNum > localBlockNum;
        }
        return false;
    }

    bool EvilDDosDetector::evil(const ProposeMsg& propose, uint32_t now, uint32_t localBlockNum) const {
        if (stillEffect(now)) {
            uint32_t blockNum = BlockHeader::num_from_id(propose.block.id());
            return blockNum > m_maxBlockNum && blockNum > localBlockNum;
        }
        return false;
    }

    void EvilDDosDetector::gatherWhenBax(const EchoMsg& echo, uint32_t blockNum, ConsensusPhase phase) {
        if (phase == kPhaseBAX && BlockHeader::num_from_id(echo.blockId) == blockNum) {
            m_recentEchoMsgV.push_back(echo);
        }
    }

    void EvilDDosDetector::deduceWhenBax(int f, uint32_t now, uint32_t blockNum, ConsensusPhase phase) {
        if (phase == kPhaseBAX) {
            std::vector<AccountName> accounts;
            for (auto e : m_recentEchoMsgV) {
                if (e.timestamp == now - 1 && BlockHeader::num_from_id(e.blockId) == blockNum) {
                    if (std::find(accounts.begin(), accounts.end(), e.account) == accounts.end()) {
                        accounts.push_back(e.account);
                    }
                }
            }
            if (accounts.size() > f) {
                m_maxBlockNum = blockNum + 2;
                m_expiry = now + s_twoPhase;
            }
            clear();
        }
    }

    bool EvilDDosDetector::stillEffect(uint32_t now) const {
        if (now < m_expiry) {
            return true;
        }
        return false;
    }

    void EvilDDosDetector::clear() {
        m_recentEchoMsgV.clear();
    }
}
