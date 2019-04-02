#pragma once

#include <string>
#include "ultrainiolib/block_header.hpp"
#include "BlockHeaderExtKey.h"

namespace ultrainiosystem {
    class EpochEndPoint {
    public:
        static bool isEpochEndPoint(const ultrainio::block_header& blockHeader) {
            auto ext = blockHeader.header_extensions;
            for (auto& e : ext) {
                if (std::get<0>(e) == kNextCommitteeMroot) {
                    return true;
                }
            }
            return false;
        }

        EpochEndPoint(const ultrainio::block_header& blockHeader) {
            auto ext = blockHeader.header_extensions;
            for (auto& e : ext) {
                BlockHeaderExtKey key = static_cast<BlockHeaderExtKey>(std::get<0>(e));
                if (key == kNextCommitteeMroot) {
                    m_nextCommitteeMroot = std::string(std::get<1>(e).begin(), std::get<1>(e).end());
                }
            }
        }

        std::string nextCommitteeMroot() const {
            return m_nextCommitteeMroot;
        }

    private:
        ultrainio::block_header m_blockHeader;

        std::string m_nextCommitteeMroot;
    };
}