#include <lightclient/EpochEndPoint.h>

#include <lightclient/BlockHeaderExtKey.h>

namespace ultrainio {
    bool EpochEndPoint::isEpochEndPoint(const BlockHeader& blockHeader) {
        ExtensionsType ext = blockHeader.header_extensions;
        for (auto& e : ext) {
            if (std::get<0>(e) == kNextCommitteeMroot) {
                return true;
            }
        }
        return false;
    }

    EpochEndPoint::EpochEndPoint(const BlockHeader& blockHeader) : m_blockHeader (blockHeader) {
        ExtensionsType ext = blockHeader.header_extensions;
        for (auto& e : ext) {
            BlockHeaderExtKey key = static_cast<BlockHeaderExtKey>(std::get<0>(e));
            if (key == kNextCommitteeMroot) {
                m_nextCommitteeMroot = std::string(std::get<1>(e).begin(), std::get<1>(e).end());
            }
        }
    }

    std::string EpochEndPoint::nextCommitteeMroot() const {
        return m_nextCommitteeMroot;
    }
}