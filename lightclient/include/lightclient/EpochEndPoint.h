#pragma once

#include <core/Message.h>

namespace ultrainio {
    class EpochEndPoint {
    public:
        static bool isEpochEndPoint(const BlockHeader& blockHeader);

        EpochEndPoint(const BlockHeader& blockHeader);

        EpochEndPoint();

        uint32_t blockNum() const;

        std::string nextCommitteeMroot() const;

    private:
        BlockHeader m_blockHeader;

        std::string m_nextCommitteeMroot;
    };
}