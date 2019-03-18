#pragma once

#include <ultrainiolib/block_header.hpp>
#include "BlockHeaderExtKey.h"
#include "CommitteeSet.h"

namespace ultrainiosystem {
    class CheckPoint {
    public:
        static bool isCheckPoint(const ultrainio::block_header& blockHeader) {
            auto ext = blockHeader.header_extensions;
            for (auto& e : ext) {
                if (std::get<0>(e) == kCommitteeSet) {
                    return true;
                }
            }
            return false;
        }

        CheckPoint(const ultrainio::block_header& blockHeader) {
            auto ext = m_blockHeader.header_extensions;
            for (auto& e : ext) {
                BlockHeaderExtKey key = static_cast<BlockHeaderExtKey>(std::get<0>(e));
                if (key == kCommitteeSet) {
                    m_committeeSet = CommitteeSet(std::get<1>(e));
                }
            }
        }

        const CommitteeSet& committeeSet() const {
            return m_committeeSet;
        }

    private:
        ultrainio::block_header m_blockHeader;

        // header extensions
        CommitteeSet m_committeeSet;
    };
}
