#pragma once

#include "core/Message.h"

namespace ultrainio {
    // sync with EchoMsg
    struct SerializedEchoMsg {
        BlockIdType blockId;
        int phase;
        uint32_t    baxCount;
        AccountName proposer;
        std::string blsSignature;
        AccountName account;
        uint32_t    timestamp;
        MsgExtension ext;
        std::string signature;

        EchoMsg toEchoMsg() const;

        SerializedEchoMsg();

        SerializedEchoMsg(const EchoMsg& echo);
    };
}

FC_REFLECT( ultrainio::SerializedEchoMsg, (blockId)(phase)(baxCount)(proposer)(blsSignature)(account)(timestamp)(ext)(signature))
