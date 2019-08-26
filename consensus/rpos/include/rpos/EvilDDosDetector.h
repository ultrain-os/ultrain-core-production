#pragma once

#include <stdint.h>
#include <vector>
#include <core/Message.h>
#include "rpos/VoterSet.h"

namespace ultrainio {
    class EvilDDosDetector {
    public:
        void deduceBlockNum(const VoterSet& voterSet, int f, uint32_t now, ConsensusPhase phase);

        bool evil(const EchoMsg& echo, uint32_t now, uint32_t localBlockNum) const;

        bool evil(const ProposeMsg& propose, uint32_t now, uint32_t localBlockNum) const;

        void gatherWhenBax(const EchoMsg& echo, uint32_t blockNum, ConsensusPhase phase);

        void deduceWhenBax(int f, uint32_t now, uint32_t blockNum, ConsensusPhase phase);

    private:
        bool stillEffect(uint32_t now) const;

        void clear();

        uint32_t m_maxBlockNum = 0;
        uint32_t m_expiry = 0;

        std::vector<EchoMsg> m_recentEchoMsgV;

        static int s_twoPhase;
    };
}
