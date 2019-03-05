#include <lightclient/LightClient.h>

#include <iostream>

#include <lightclient/LightClientCallback.h>

namespace ultrainio {
    LightClient::LightClient(uint64_t chainName) : m_chainName(chainName) {}

    uint64_t LightClient::chainName() const {
        return m_chainName;
    }

    void LightClient::accept(const BlockHeader& blockHeader) {
        std::cout << " LightClient::accept " << blockHeader.block_num() << " latest is " << BlockHeader::num_from_id(m_latestConfirmedBlockId) << std::endl;
        if (blockHeader.block_num() <= BlockHeader::num_from_id(m_latestConfirmedBlockId)) {
            return;
        }
        if (std::string(blockHeader.proposer) == std::string("genesis")) { // see rpos/Genesis.cpp
            return;
        }

        auto unconfirmItor = m_unconfirmedList.begin();
        while (true) {
            if (unconfirmItor == m_unconfirmedList.end()) {
                m_unconfirmedList.push_back(blockHeader);
                break;
            }
            if (unconfirmItor->block_num() >= blockHeader.block_num()) {
                if (unconfirmItor->id() == blockHeader.id()) {
                    // duplicate accept
                    return;
                }
            } else {
                m_unconfirmedList.insert(unconfirmItor, blockHeader);
                break;
            }
            unconfirmItor++;
        }

        if (EpochEndPoint::isEpochEndPoint(blockHeader)) {
            auto epochEndPointItor = m_unconfirmedEpochEndPointList.begin();
            while (true) {
                if (epochEndPointItor == m_unconfirmedEpochEndPointList.end()) {
                    m_unconfirmedEpochEndPointList.push_back(EpochEndPoint(blockHeader));
                    break;
                }
                if (epochEndPointItor->blockNum() >= blockHeader.block_num()) {
                    // do not need to do duplicate check
                } else {
                    m_unconfirmedEpochEndPointList.insert(epochEndPointItor, EpochEndPoint(blockHeader));
                    break;
                }
                epochEndPointItor++;
            }
        }

        if (CheckPoint::isCheckPoint(blockHeader)) {
            auto checkPointItor = m_unconfirmedCheckPointList.begin();
            while (true) {
                if (checkPointItor == m_unconfirmedCheckPointList.begin()) {
                    m_unconfirmedCheckPointList.push_back(CheckPoint(blockHeader));
                    break;
                }
                if (checkPointItor->blockNum() >= blockHeader.block_num()) {
                    // also do not need to do duplicate check
                } else {
                    m_unconfirmedCheckPointList.insert(checkPointItor, CheckPoint(blockHeader));
                    break;
                }
                checkPointItor++;
            }
        }

        if (ConfirmPoint::isConfirmPoint(blockHeader)) {
            ConfirmPoint confirmPoint(blockHeader);
            BlockIdType blockId = confirmPoint.confirmedBlockId();
            // TODO verify bls
            // update confirmed list
            m_latestConfirmedBlockId = blockId;
            uint32_t confirmedBlockNum = BlockHeader::num_from_id(m_latestConfirmedBlockId);

            // handle unconfirmed list
            auto unconfirmItor = m_unconfirmedList.begin();
            while (unconfirmItor != m_unconfirmedList.end()) {
                if (blockId == unconfirmItor->id()) {
                    auto t = unconfirmItor;
                    unconfirmItor++;
                    blockId = t->previous;
                    m_confirmedList.push_front(*t);
                    m_unconfirmedList.erase(t);
                    continue;
                }
                unconfirmItor++;
            }
            // TODO punish
            unconfirmItor = m_unconfirmedList.begin();
            while (unconfirmItor != m_unconfirmedList.end()) {
                if (unconfirmItor->block_num() <= confirmedBlockNum) {
                    m_unconfirmedList.erase(unconfirmItor, m_unconfirmedList.end());
                    break;
                }
                unconfirmItor++;
            }

            // handle unconfirmed CheckPoint list
            auto checkPointItor = m_unconfirmedCheckPointList.begin();
            while (checkPointItor != m_unconfirmedCheckPointList.end()) {
                if (checkPointItor->blockNum() <= confirmedBlockNum) {
                    m_unconfirmedCheckPointList.erase(checkPointItor, m_unconfirmedCheckPointList.end());
                    break;
                }
                checkPointItor++;
            }
            if (m_unconfirmedCheckPointList.size() > 0
                    && m_unconfirmedCheckPointList.back().blockHeader().previous == m_latestConfirmedBlockId) {
                std::cout << "committee set blockNum : " << m_unconfirmedCheckPointList.back().blockNum() << std::endl;
                workingCommitteeSet = m_unconfirmedCheckPointList.back().committeeSet();
            }

            // handle unconfirmed EpochEndPoint list
            auto epochEndPointItor = m_unconfirmedEpochEndPointList.begin();
            while (epochEndPointItor != m_unconfirmedEpochEndPointList.end()) {
                if (epochEndPointItor->blockNum() <= confirmedBlockNum) {
                    m_unconfirmedEpochEndPointList.erase(epochEndPointItor, m_unconfirmedEpochEndPointList.end());
                    break;
                }
                epochEndPointItor++;
            }

            if (m_confirmedList.size() > 0 && m_callback) {
                m_callback->onConfirmed(m_confirmedList);
                clear();
            }

            std::cout << "unconfirmed BlockNum max : " << ((m_unconfirmedList.size() > 0) ? m_unconfirmedList.back().block_num() : -1) << std::endl;
            std::cout << "unconfirmed BlockNum min : " << ((m_unconfirmedList.size() > 0) ? m_unconfirmedList.front().block_num() : -1) << std::endl;
            std::cout << "unconfirmed CheckPoint size : " << m_unconfirmedCheckPointList.size() << std::endl;
            std::cout << "unconfirmed EpochEndPoint size : " << m_unconfirmedEpochEndPointList.size() << std::endl;
        }
    }

    void LightClient::addCallback(std::shared_ptr<LightClientCallback> cb) {
        m_callback = cb;
    }

    void LightClient::clear() {
        m_confirmedList.clear();
    }
}