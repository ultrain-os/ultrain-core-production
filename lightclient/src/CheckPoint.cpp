#include <lightclient/CheckPoint.h>

#include <lightclient/BlockHeaderExtKey.h>

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

    uint32_t CheckPoint::blockNum() const {
        return m_blockHeader.block_num();
    }

    CommitteeSet CheckPoint::committeeSet() const {
        return m_committeeSet;
    }

    const BlockHeader& CheckPoint::blockHeader() const {
        return m_blockHeader;
    }

    BlockIdType CheckPoint::getPreCheckPointBlockId() const {
        return m_preCheckPointBlockId;
    }
}
