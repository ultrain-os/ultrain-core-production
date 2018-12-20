#pragma once

#include <core/Message.h>
#include <core/Redefined.h>

namespace ultrainio {
    class RoleRandom {
    public:
        RoleRandom(const BlockIdType& preHash, uint32_t blockNum, const ConsensusPhase& phase = kPhaseBA0, int baxCount = 0);

        fc::sha256 getRand() const;

        ConsensusPhase getPhase() const;

        int getBaxCount() const;
    private:
        ConsensusPhase m_phase;
        int m_baxCount;
        fc::sha256 m_rand;
    };
}