#pragma once

#include <map>
#include <memory>
#include <vector>

#include <core/Message.h>
#include <rpos/Proof.h>

namespace ultrainio {

    class PhaseMsg {
    public:
        void moveToNewStep(uint32_t blockNum, ConsensusPhase phase, int baxCount);

        bool isVoter() const;

    private:
        ConsensusPhase m_phase = kPhaseInit;
        int m_baxCount = 0;
        bool m_isVoter = false;
        friend class BlockMsg;
    };

    typedef std::shared_ptr<PhaseMsg> PhaseMsgPtr;
}