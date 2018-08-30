#pragma once

#include <string>
#include <core/Message.h>

namespace ultrainio {
    class Seed {
    public:
        Seed(const std::string& preHash, uint32_t blockNum, ConsensusPhase phase, int baxCount);
        explicit operator std::string() const;

    protected:
        std::string m_preHash;
        uint32_t m_blockNum;
        ConsensusPhase m_phase;
        int m_baxCount;
    };
}