#pragma once

#include <map>
#include <memory>
#include <vector>

#include <core/Message.h>
#include <rpos/Proof.h>

namespace ultrainio {
    struct EchoMsgSet {
        std::vector<EchoMsg> echoMsgV;
        std::vector<std::string> pkPool; // pk pool of echoMsgDescV, speed up search
        int totalVoterCount = 0;
        BlockHeader blockHeader;
    };

    class PhaseMessage {
    public:
        void insert(const EchoMsg& echoMsg);
        void moveToNewStep(uint32_t blockNum, ConsensusPhase phase, int baxCount);

        ConsensusPhase phase = kPhaseInit;
        int baxCount = 0;
        int voterCountAsVoter = 0;
        Proof proof;
    private:
        std::map<chain::block_id_type, EchoMsgSet> echoMsgSetMap;
    };

    typedef std::shared_ptr<PhaseMessage> PhaseMessagePtr;
}