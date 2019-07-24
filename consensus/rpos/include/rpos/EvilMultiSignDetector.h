#pragma once

#include <map>
#include <core/Message.h>

namespace ultrainio {
    class EvilMultiSignDetector {
    public:
        bool hasMultiVote(const EchoMsg& echo);

        bool hasMultiPropose(std::map<BlockIdType, ProposeMsg> proposerMsgMap, const ProposeMsg& propose) const;

        void reset();
    private:

        std::map<AccountName, BlockIdType> m_committeeVoteBlock;
    };
}