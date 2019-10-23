#pragma once

#include <vector>
#include <lightclient/CommitteeSet.h>
#include <core/Message.h>
#include <rpos/VoterSet.h>

namespace ultrainio {
    class EvilBlsDetector {
    public:
        void detect(const VoterSet& voterSet, const CommitteeSet& committeeSet,
                    VoterSet& newVoterSet, std::vector<EchoMsg>& evilEchoMsgs);

    private:
        void realDetect(const VoterSet& voterSet, int fromIndex, int toIndex, const CommitteeSet& committeeSet,
                std::vector<EchoMsg>& evilEchoMsgs);
    };
}
