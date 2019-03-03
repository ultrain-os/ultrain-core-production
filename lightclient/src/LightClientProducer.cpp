#include <lightclient/LightClientProducer.h>

namespace ultrainio {
    LightClientProducer::LightClientProducer() {}

    bool LightClientProducer::shouldBeConfirmed(const BlockHeader& blockHeader) const {
        return EpochEndPoint::isEpochEndPoint(blockHeader);
    }

    bool LightClientProducer::hasNextTobeConfirmedBlsVoterSet() const {
        if (m_shouldBeConfirmedBlsVoterSetList.size() > 0) {
            BlockIdType blockId = m_shouldBeConfirmedBlsVoterSetList.front().commonEchoMsg.blockId;
            for (auto e : m_shouldBeConfirmedList) {
                if (e.id() == blockId) {
                    return true;
                }
            }
        }
        return false;
    }

    BlsVoterSet LightClientProducer::nextTobeConfirmedBlsVoterSet() const {
        return m_shouldBeConfirmedBlsVoterSetList.front();
    }

    BlockIdType LightClientProducer::getLatestCheckPointId() const {
        return m_latestCheckPointId;
    }

    void LightClientProducer::acceptBlockHeader(const BlockHeader& blockHeader, const BlsVoterSet& blsVoterSet) {
        // need Confirmed
        if (shouldBeConfirmed(blockHeader)) {
            m_shouldBeConfirmedList.push_back(blockHeader);
            if (!blsVoterSet.empty() && blsVoterSet.commonEchoMsg.blockId == blockHeader.id()) {
                m_shouldBeConfirmedBlsVoterSetList.push_back(blsVoterSet);
            }
        }

        if (CheckPoint::isCheckPoint(blockHeader)) {
            m_latestCheckPointId = blockHeader.id();
        }

        if (ConfirmPoint::isConfirmPoint(blockHeader)) {
            ConfirmPoint confirmPoint(blockHeader);
            BlockIdType blockId = confirmPoint.confirmedBlockId();
            m_latestConfirmedBlockId = blockId;
            uint32_t blockNum = BlockHeader::num_from_id(m_latestConfirmedBlockId);
            for (auto headerItor = m_shouldBeConfirmedList.begin(); headerItor != m_shouldBeConfirmedList.end(); headerItor++) {
                if (BlockHeader::num_from_id(headerItor->id()) <= blockNum) {
                    m_shouldBeConfirmedList.erase(headerItor);
                    break;
                }
            }
            for (auto blsItor = m_shouldBeConfirmedBlsVoterSetList.begin(); blsItor != m_shouldBeConfirmedBlsVoterSetList.end(); blsItor++) {
                if (BlockHeader::num_from_id(blsItor->commonEchoMsg.blockId) <= blockNum) {
                    m_shouldBeConfirmedBlsVoterSetList.erase(blsItor);
                    break;
                }
            }
        }
    }
}