#include "rpos/RoleRandom.h"

#include <fc/crypto/sha256.hpp>
#include <rpos/Utils.h>

namespace ultrainio {
    RoleRandom::RoleRandom(const BlockIdType& preHash, uint32_t blockNum, const ConsensusPhase& phase, int baxCount)
            : m_phase(phase), m_baxCount(baxCount) {
        std::string seed = std::string(preHash) + std::to_string(blockNum) + std::to_string(static_cast<uint32_t>(phase)) + std::to_string(baxCount);
        m_rand = fc::sha256::hash(seed);
    };

    fc::sha256 RoleRandom::getRand() const {
        return m_rand;
    }

    ConsensusPhase RoleRandom::getPhase() const {
        return m_phase;
    }

    int RoleRandom::getBaxCount() const {
        return m_baxCount;
    }

}