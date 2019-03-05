#include <lightclient/LightClientProducer.h>

namespace ultrainio {
    LightClientProducer::LightClientProducer() {}

    bool LightClientProducer::shouldBeConfirmed(const BlockHeader& blockHeader) const {
        if (EpochEndPoint::isEpochEndPoint(blockHeader)) {
            return true;
        }
        if (m_lastShouldBeConfirmedBlockNum > 0 && ((blockHeader.block_num() - m_lastShouldBeConfirmedBlockNum) % m_confirmedInterval == 0)) {
            return true;
        }
        return false;
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

    void LightClientProducer::accept(const BlockHeader& blockHeader, const BlsVoterSet& blsVoterSet) {
        // need Confirmed
        if (shouldBeConfirmed(blockHeader)) {
            m_shouldBeConfirmedList.push_back(blockHeader);
            m_lastShouldBeConfirmedBlockNum = blockHeader.block_num();
            if (!blsVoterSet.empty() && blsVoterSet.commonEchoMsg.blockId == blockHeader.id()) {
                m_shouldBeConfirmedBlsVoterSetList.push_back(blsVoterSet);
            }
        }

        if (CheckPoint::isCheckPoint(blockHeader)) {
            m_latestCheckPointId = blockHeader.id();
        }

        if (ConfirmPoint::isConfirmPoint(blockHeader)) {
            // TODO check bls
            ConfirmPoint confirmPoint(blockHeader);
            BlockIdType blockId = confirmPoint.confirmedBlockId();
            m_latestConfirmedBlockId = blockId;
            uint32_t latestConfirmedBlockNum = BlockHeader::num_from_id(m_latestConfirmedBlockId);

            // update should be confirmed list
            auto headerItor = m_shouldBeConfirmedList.begin();
            while (true) {
                if (headerItor == m_shouldBeConfirmedList.end()) {
                    m_shouldBeConfirmedList.clear();
                    break;
                }
                if (headerItor->block_num() > latestConfirmedBlockNum) {
                    if (headerItor != m_shouldBeConfirmedList.begin()) {
                        m_shouldBeConfirmedList.erase(m_shouldBeConfirmedList.begin(), headerItor);
                    }
                    break;
                }
                headerItor++;
            }

            // update should be confirmed bls voter set list
            auto blsItor = m_shouldBeConfirmedBlsVoterSetList.begin();
            while (true) {
                if (blsItor == m_shouldBeConfirmedBlsVoterSetList.end()) {
                    m_shouldBeConfirmedBlsVoterSetList.clear();
                    break;
                }
                if (BlockHeader::num_from_id(blsItor->commonEchoMsg.blockId) > latestConfirmedBlockNum) {
                    if (blsItor != m_shouldBeConfirmedBlsVoterSetList.begin()) {
                        m_shouldBeConfirmedBlsVoterSetList.erase(m_shouldBeConfirmedBlsVoterSetList.begin(), blsItor);
                    }
                    break;
                }
                blsItor++;
            }
        }
        std::cout << "m_lastShouldBeConfirmedBlockNum = " << m_lastShouldBeConfirmedBlockNum << std::endl;
        std::cout << "m_shouldBeConfirmedList size = " <<m_shouldBeConfirmedList.size() << std::endl;
    }
}