#include <lightclient/ConfirmPoint.h>

#include <lightclient/BlockHeaderExtKey.h>

namespace ultrainio {
    bool ConfirmPoint::isConfirmPoint(const BlockHeader& blockHeader) {
        ExtensionsType ext = blockHeader.header_extensions;
        for (auto& e : ext) {
            if (std::get<0>(e) == kBlsVoterSet) {
                return true;
            }
        }
        return false;
    }

    ConfirmPoint::ConfirmPoint() {}

    ConfirmPoint::ConfirmPoint(const BlockHeader& blockHeader) : m_blockHeader(blockHeader) {
        ExtensionsType ext = blockHeader.header_extensions;
        for (auto& e : ext) {
            BlockHeaderExtKey key = static_cast<BlockHeaderExtKey>(std::get<0>(e));
            if (key == kBlsVoterSet) {
                m_blsVoterSet = BlsVoterSet(std::get<1>(e));
            }
        }
    }

    BlockIdType ConfirmPoint::confirmedBlockId() const {
        return m_blsVoterSet.commonEchoMsg.blockId;
    }

    const BlsVoterSet& ConfirmPoint::blsVoterSet() const {
        return m_blsVoterSet;
    }
}