#include "core/SerializedEchoMsg.h"

namespace ultrainio {
    EchoMsg SerializedEchoMsg::toEchoMsg() const {
        EchoMsg echo;
        echo.blockId = blockId;
        echo.phase = static_cast<ConsensusPhase>(phase);
        echo.baxCount = baxCount;
        echo.proposer = proposer;
        echo.blsSignature = blsSignature;
        echo.account = account;
        echo.timestamp = timestamp;
        echo.ext = ext;
        echo.signature = signature;
        return echo;
    }

    SerializedEchoMsg::SerializedEchoMsg() {}

    SerializedEchoMsg::SerializedEchoMsg(const EchoMsg& echo) {
        blockId = echo.blockId;
        phase = echo.phase;
        baxCount = echo.baxCount;
        proposer = echo.proposer;
        blsSignature = echo.blsSignature;
        account = echo.account;
        timestamp = echo.timestamp;
        ext = echo.ext;
        signature = echo.signature;
    }
}