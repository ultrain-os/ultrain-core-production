#include <lightclient/LightClientProducer.h>

namespace ultrainio {
    LightClientProducer::LightClientProducer() {}

    bool LightClientProducer::shouldBeConfirmed(const BlockHeader& blockHeader) const {
        return EpochEndPoint::isEpochEndPoint(blockHeader);
    }

    void LightClientProducer::addBlockHeaderAndBlsVoterSetPair(const BlockHeader& blockHeader, const BlsVoterSet& blsVoterSet) {
        if (!blsVoterSet.empty()) {
            m_shouldBeConfirmedBlockHeaderList.push_back(blockHeader);
            m_shouldBeConfirmedBlockHeaderBlsVoterSetList.push_back(blsVoterSet);
        }
    }

    bool LightClientProducer::hasNextTobeConfirmedBlsVoterSet() const {
        return m_shouldBeConfirmedBlockHeaderBlsVoterSetList.size() > 0;
    }

    BlsVoterSet LightClientProducer::nextTobeConfirmedBlsVoterSet() const {
        return m_shouldBeConfirmedBlockHeaderBlsVoterSetList.front();
    }

    void LightClientProducer::acceptBlockHeader(const BlockHeader& blockHeader) {
        if (ConfirmPoint::isConfirmPoint(blockHeader)) {
            ConfirmPoint confirmPoint(blockHeader);
            BlockIdType blockId = confirmPoint.confirmedBlockId();
            auto headerItor = m_shouldBeConfirmedBlockHeaderList.begin();
            auto blsItor = m_shouldBeConfirmedBlockHeaderBlsVoterSetList.begin();
            while (headerItor != m_shouldBeConfirmedBlockHeaderList.end()) {
                if (blockId == headerItor->id()) {
                    m_shouldBeConfirmedBlockHeaderList.erase(headerItor);
                    m_shouldBeConfirmedBlockHeaderBlsVoterSetList.erase(blsItor);
                    return;
                }
                headerItor++;
                blsItor++;
            }
        }
    }
}