#include <lightclient/CheckPoint.h>

namespace ultrainio {
    bool CheckPoint::isCheckPoint(const BlockHeader& blockHeader) {
        ExtensionsType ext = blockHeader.header_extensions;
        for (auto& e : ext) {
            if (std::get<0>(e) == kCommitteeSet) {
                return true;
            }
        }
        return false;
    }

    CheckPoint::CheckPoint() {}
    CheckPoint::CheckPoint(const BlockHeader& blockHeader) : m_blockHeader(blockHeader) {
        ExtensionsType ext = m_blockHeader.header_extensions;
        for (auto& e : ext) {
            BlockHeaderExtKey key = static_cast<BlockHeaderExtKey>(std::get<0>(e));
            if (key == kPreCheckPointId) {
                auto& value = std::get<1>(e);
                m_preCheckPointBlockId = BlockIdType(std::string(value.begin(), value.end()));
            } else if (key == kCommitteeSet) {
                m_committeeSet = CommitteeSet(std::get<1>(e));
            }
        }
    }
}