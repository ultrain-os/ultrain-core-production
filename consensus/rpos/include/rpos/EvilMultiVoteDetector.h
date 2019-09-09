#pragma once

#include <core/Message.h>
#include <core/MultiVoteEvidence.h>

#include "rpos/VoterSet.h"

namespace ultrainio {
    class EvilMultiVoteDetector {
    public:
        bool hasMultiVote(BlockIdVoterSetMap& blockIdVoterSetMap, const EchoMsg& echo, MultiVoteEvidence& outEvidence);
    };
}