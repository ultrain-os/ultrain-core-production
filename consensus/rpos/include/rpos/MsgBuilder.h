#pragma once

#include <core/Message.h>

namespace ultrainio {
    class MsgBuilder {
    public:
        static EchoMsg constructMsg(const Block &block, size_t index);

        static EchoMsg constructMsg(const ProposeMsg &propose, size_t index);

        static EchoMsg constructMsg(const EchoMsg &echo, size_t index);
    };
}