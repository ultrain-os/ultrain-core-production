#pragma once

#include <core/Message.h>
#include <crypto/Signature.h>

namespace ultrainio {
    class MsgBuilder {
    public:
        static EchoMsg constructMsg(const Block &block);

        static EchoMsg constructMsg(const ProposeMsg &propose);

        static EchoMsg constructMsg(const EchoMsg &echo);
    };
}