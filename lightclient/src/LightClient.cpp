#include <lightclient/LightClient.h>

namespace ultrainio {
    LightClient::LightClient(uint64_t chainName) : m_chainName(chainName) {}

    uint64_t LightClient::chainName() const {
        return m_chainName;
    }

    void LightClient::accept(const BlockHeader& blockHeader) {
        if (blockHeader.block_num() <= BlockHeader::num_from_id(m_latestConfirmedBlockId)) {
            return;
        }
        if (std::string(blockHeader.proposer) == std::string("genesis")) { // see rpos/Genesis.cpp
            return;
        }

        m_unconfirmedList.push_front(blockHeader);

        if (EpochEndPoint::isEpochEndPoint(blockHeader)) {
            m_unconfirmedEpochEndPointList.push_front(blockHeader);
        }

        if (CheckPoint::isCheckPoint(blockHeader)) {
            m_unconfirmedCheckPointList.push_front(blockHeader);
        }

        if (ConfirmPoint::isConfirmPoint(blockHeader)) {
            ConfirmPoint confirmPoint(blockHeader);
            BlockIdType blockId = confirmPoint.confirmedBlockId();
            // TODO verify bls
            // update confirmed list
            m_latestConfirmedBlockId = blockId;
            auto itor = m_unconfirmedList.begin();
            while (itor != m_unconfirmedList.end()) {
                if (blockId == itor->id()) {
                    auto& t = itor;
                    itor++;
                    blockId = t->previous;
                    m_confirmedList.push_front(*t);
                    m_unconfirmedList.erase(t);
                    continue;
                }
                itor++;
            }
            // TODO punish
            uint32_t confirmedBlockNum = BlockHeader::num_from_id(m_latestConfirmedBlockId);
            itor = m_unconfirmedList.begin();
            while (itor != m_unconfirmedList.end()) {
                if (itor->block_num() <= confirmedBlockNum) {
                    m_unconfirmedList.erase(itor, m_unconfirmedList.end());
                    break;
                }
                itor++;
            }
            // update CheckPoint
            auto checkPointItor = m_unconfirmedCheckPointList.begin();
            while (checkPointItor != m_unconfirmedCheckPointList.end()) {
                if (checkPointItor->blockNum() <= confirmedBlockNum) {
                    m_unconfirmedCheckPointList.erase(checkPointItor, m_unconfirmedCheckPointList.end());
                    break;
                }
                checkPointItor++;
            }
            if (m_unconfirmedCheckPointList.size() > 0
                    && confirmedBlockNum == m_unconfirmedCheckPointList.back().blockNum() - 1) {
                // TODO check whether EpochEndPoint is confirmed
                workingCommitteeSet = m_unconfirmedCheckPointList.back().committeeSet();
            }

            auto epochEndPointItor = m_unconfirmedEpochEndPointList.begin();
            while (epochEndPointItor != m_unconfirmedEpochEndPointList.end()) {
                if (epochEndPointItor->blockNum() <= confirmedBlockNum) {
                    m_unconfirmedEpochEndPointList.erase(epochEndPointItor, m_unconfirmedEpochEndPointList.end());
                }
                epochEndPointItor++;
            }
        }
    }

    std::list<BlockHeader> LightClient::getConfirmedList() const {
        return m_confirmedList;
    }

    void LightClient::clear() {
        m_confirmedList.clear();
    }
}