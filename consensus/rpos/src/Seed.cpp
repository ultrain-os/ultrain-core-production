#include <rpos/Seed.h>

namespace ultrainio {
    Seed::Seed(const std::string& preHash, uint32_t blockNum, ConsensusPhase phase, int baxCount)
            : m_preHash(preHash), m_blockNum(blockNum), m_phase(phase), m_baxCount(baxCount) {

    }

    Seed::operator std::string() const {
        return m_preHash + std::to_string(m_blockNum) + std::to_string(static_cast<uint32_t>(m_phase)) + std::to_string(m_baxCount);
    }
}