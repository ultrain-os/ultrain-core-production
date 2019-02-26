#include <lightclient/EpochEndPoint.h>

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
                std::string r(std::get<1>(e).begin(), std::get<1>(e).end());
                m_nextCommitteeMroot = SHA256(r);
            }
        }
    }
}