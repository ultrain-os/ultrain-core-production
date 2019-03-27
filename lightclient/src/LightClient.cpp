#include <lightclient/LightClient.h>

#include <iostream>

#include <lightclient/LightClientCallback.h>
#include <lightclient/Helper.h>

namespace ultrainio {
    LightClient::LightClient(uint64_t chainName) : m_chainName(chainName) {}

    uint64_t LightClient::chainName() const {
        return m_chainName;
    }

    bool LightClient::setStartPoint(const CommitteeSet& committeeSet, const BlockIdType& blockId) {
        ilog("set start point confirmed num : ${num}, id : ${id} committeeSet : ${committeeSet}", ("num", BlockHeader::num_from_id(blockId))("id", blockId)("committeeSet", committeeSet.toString()));
        reset();
        m_workingCommitteeSet = committeeSet;
        m_latestConfirmedBlockId = blockId;
        return true;
    }

    void LightClient::accept(const BlockHeader& blockHeader, const BlsVoterSet& blsVoterSet) {
        ilog("accept BlockHeader num : ${blockNum}, my id : ${myId}, BlsVoterSet confirm num : ${confirmedBlockNum} id : ${id}, latest confirm : ${latest}",
                ("blockNum", blockHeader.block_num())("myId", blockHeader.id())("confirmedBlockNum", BlockHeader::num_from_id(blsVoterSet.commonEchoMsg.blockId))
                ("id", blsVoterSet.commonEchoMsg.blockId)("latest", BlockHeader::num_from_id(m_latestConfirmedBlockId)));
        if (blockHeader.id() != blsVoterSet.commonEchoMsg.blockId) {
            ilog("BlsVoterSet confirm ${id} while blockHeader id is ${blockId}", ("id", blsVoterSet.commonEchoMsg.blockId)("blockId", blockHeader.id()));
            onError(kBlsVoterSetNotMatch, blockHeader);
            return;
        }
        accept(blockHeader);
        confirm(blsVoterSet);
    }

    void LightClient::accept(const BlockHeader& blockHeader) {
        ilog("accept BlockHeader num : ${blockNum}, id : ${id} latest confirm : ${latest}",
             ("blockNum", blockHeader.block_num())("id", blockHeader.id())("latest", BlockHeader::num_from_id(m_latestConfirmedBlockId)));
        if (!check(blockHeader)) {
            return;
        }
        if (Helper::isGenesis(blockHeader)) {
            handleGenesis(blockHeader);
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
            confirm(confirmPoint.blsVoterSet());
        }
    }

    void LightClient::confirm(const BlsVoterSet& blsVoterSet) {
        if (!blsVoterSet.valid()) {
            return;
        }
        // TODO verify bls
        BlockIdType maybeConfirmedBlockId = blsVoterSet.commonEchoMsg.blockId;
        ilog("maybe confirmed blockNum : ${blockNum} blockId : ${blockId}", ("blockNum", BlockHeader::num_from_id(maybeConfirmedBlockId))("blockId", maybeConfirmedBlockId));

        // handle unconfirmed list
        BlockIdType blockId = maybeConfirmedBlockId;
        auto unconfirmItor = m_unconfirmedList.begin();
        while (unconfirmItor != m_unconfirmedList.end()) {
            if (blockId == unconfirmItor->id()) {
                if (maybeConfirmedBlockId == unconfirmItor->id()) {
                    blockId = unconfirmItor->previous;
                    unconfirmItor++;
                    continue;
                }
                auto t = unconfirmItor;
                unconfirmItor++;
                blockId = t->previous;
                m_confirmedList.push_front(*t);
                m_unconfirmedList.erase(t);
                continue;
            }
            unconfirmItor++;
        }
        // TODO punish the forked ones
        uint32_t confirmedBlockNum = BlockHeader::num_from_id(maybeConfirmedBlockId) - 1;
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
            m_workingCommitteeSet = m_unconfirmedCheckPointList.back().committeeSet();
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

        if (m_confirmedList.size() > 0) {
            m_latestConfirmedBlockId = m_confirmedList.back().id();
            onConfirmed(m_confirmedList);
            m_confirmedList.clear();
        }

        ilog("unconfirmed from ${from} to ${to}",
                ("from", ((m_unconfirmedList.size() > 0) ? m_unconfirmedList.back().block_num() : -1))
                ("to", ((m_unconfirmedList.size() > 0) ? m_unconfirmedList.front().block_num() : -1)));
        ilog("unconfirmed CheckPoint size : ${size}", ("size", m_unconfirmedCheckPointList.size()));
        ilog("unconfirmed EpochEndPoint size : ${size}", ("size", m_unconfirmedEpochEndPointList.size()));
    }

    void LightClient::addCallback(std::shared_ptr<LightClientCallback> cb) {
        m_callback = cb;
    }

    bool LightClient::check(const BlockHeader& blockHeader) {
        if (exceedLargestUnconfirmedBlockNum(blockHeader)) {
            onError(kExceedLargestBlockNum, blockHeader);
            return false;
        }
        if (lessConfirmedBlockNum(blockHeader)) {
            onError(kLessConfirmedBlockNum, blockHeader);
            return false;
        }
        return true;
    }

    void LightClient::handleGenesis(const BlockHeader& blockHeader) {
        // TODO sign check
        m_latestConfirmedBlockId = blockHeader.id();
        ULTRAIN_ASSERT(m_confirmedList.size() == 0, chain::chain_exception, "m_confirmedList size != 0");
        std::list<BlockHeader> genesisBlockHeader;
        genesisBlockHeader.push_back(blockHeader);
        onConfirmed(genesisBlockHeader);
    }

    bool LightClient::exceedLargestUnconfirmedBlockNum(const BlockHeader& blockHeader) const {
        if (m_unconfirmedList.size() > 0 && (m_unconfirmedList.front().block_num() + 1 < blockHeader.block_num())) {
            return true;
        }
        return false;
    }

    bool LightClient::lessConfirmedBlockNum(const BlockHeader& blockHeader) const {
        if (m_latestConfirmedBlockId != BlockIdType() && BlockHeader::num_from_id(m_latestConfirmedBlockId) >= blockHeader.block_num()) {
            return true;
        }
        return false;
    }

    void LightClient::onError(const LightClientError& error, const BlockHeader& blockHeader) {
        if (m_callback) {
            m_callback->onError(error, blockHeader);
        }
    }

    void LightClient::onConfirmed(const std::list<BlockHeader>& blockHeaderList) {
        if (m_callback) {
            m_callback->onConfirmed(blockHeaderList);
        }
    }

    BlockIdType LightClient::getLatestConfirmedBlockId() const {
        return m_latestConfirmedBlockId;
    }

    void LightClient::reset() {
        m_workingCommitteeSet = CommitteeSet();
        m_latestConfirmedBlockId = SHA256();
        m_unconfirmedList.clear();
        m_unconfirmedCheckPointList.clear();
        m_unconfirmedEpochEndPointList.clear();
        m_confirmedList.clear();
    }
}