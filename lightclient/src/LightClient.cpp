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

    void LightClient::setStartPoint(const StartPoint& startPoint) {
        m_startPoint = startPoint;
        m_workingCommitteeSet = m_startPoint.committeeSet;
        m_latestConfirmedBlockId = m_startPoint.lastConfirmedBlockId;
    }

    // invoked by fetch block feature
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

    // pass signature when genesis block
    void LightClient::accept(const BlockHeader& blockHeader, const std::string& signature) {
        ilog("accept BlockHeader num : ${blockNum}, id : ${id} latest confirm : ${latest}",
             ("blockNum", blockHeader.block_num())("id", blockHeader.id())("latest", BlockHeader::num_from_id(m_latestConfirmedBlockId)));
        if (Helper::isGenesis(blockHeader)) {
            ilog("signature : ${s} for blockNum : ${num}", ("s", signature)("num", blockHeader.block_num()));
            handleGenesis(blockHeader);
            return;
        }

        if (isOutOfRange(blockHeader)) {
            onError(kOutOfRange, blockHeader);
            return;
        }

        if (m_workingCommitteeSet == CommitteeSet()) {
            ULTRAIN_ASSERT(CheckPoint::isCheckPoint(blockHeader), chain::chain_exception, "DO NOT pass check point when working committee set is empty");
            m_workingCommitteeSet = CheckPoint(blockHeader).committeeSet();
            ULTRAIN_ASSERT(std::string(m_workingCommitteeSet.committeeMroot()) == m_startPoint.nextCommitteeMroot, chain::chain_exception, "working committee set' MRoot not equal ${root}", ("root", m_startPoint.nextCommitteeMroot));
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

        if (ConfirmPoint::isConfirmPoint(blockHeader)) {
            ConfirmPoint confirmPoint(blockHeader);
            confirm(confirmPoint.blsVoterSet());
        }
    }

    void LightClient::confirm(const BlsVoterSet& blsVoterSet) {
        if (!blsVoterSet.valid()) {
            return;
        }
        BlockIdType maybeConfirmedBlockId = blsVoterSet.commonEchoMsg.blockId;
        if (BlockHeader::num_from_id(maybeConfirmedBlockId) <= BlockHeader::num_from_id(m_latestConfirmedBlockId)) {
            return;
        }
        ilog("maybe confirmed blockNum : ${blockNum} blockId : ${blockId}", ("blockNum", BlockHeader::num_from_id(maybeConfirmedBlockId))("blockId", maybeConfirmedBlockId));

        bool linked = false;
        // handle unconfirmed list
        BlockIdType nextBlockId = maybeConfirmedBlockId;
        auto unconfirmItor = m_unconfirmedList.begin();
        while (unconfirmItor != m_unconfirmedList.end()) {
            if (nextBlockId == unconfirmItor->id()) {
                m_confirmedList.push_front(*unconfirmItor);
                nextBlockId = unconfirmItor->previous;
                if (nextBlockId == m_latestConfirmedBlockId) {
                    linked = true;
                    break;
                }
            }
            unconfirmItor++;
        }
        if (!linked) {
            elog("block header can not link together, bls voter set id : ${id} num : ${num}", ("id", blsVoterSet.commonEchoMsg.blockId)("num", BlockHeader::num_from_id(blsVoterSet.commonEchoMsg.blockId)));
            return;
        }
        if (m_confirmedList.size() <= 1) {
            elog("no any confirmed block, bls voter set id : ${id}", ("id", blsVoterSet.commonEchoMsg.blockId));
            return;
        }
        if (!verifyBlockHeaderList(m_confirmedList, blsVoterSet)) {
            m_status = false;
            return;
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

        m_confirmedList.pop_back();
        m_latestConfirmedBlockId = m_confirmedList.back().id();
        onConfirmed(m_confirmedList);
        m_confirmedList.clear();

        ilog("unconfirmed from ${from} to ${to}",
                ("from", ((m_unconfirmedList.size() > 0) ? m_unconfirmedList.back().block_num() : -1))
                ("to", ((m_unconfirmedList.size() > 0) ? m_unconfirmedList.front().block_num() : -1)));
    }

    bool LightClient::verifyBlockHeaderList(const std::list<BlockHeader>& blockHeaderList, const BlsVoterSet& blsVoterSet) {
        std::list<ConfirmPoint> confirmPointList;
        for (auto v : blockHeaderList) {
            if (ConfirmPoint::isConfirmPoint(v)) {
                confirmPointList.push_back(ConfirmPoint(v));
            }
        }
        auto itor = blockHeaderList.begin();
        for (; itor != blockHeaderList.end(); itor++) {
            bool isConfirmed = false;
            for (auto v : confirmPointList) {
                if (itor->id() == v.confirmedBlockId()) {
                    if (!m_workingCommitteeSet.verify(v.blsVoterSet())) {
                        elog("verify bls error, id : ${id} num : ${num}", ("id", itor->id())("num", BlockHeader::num_from_id(itor->id())));
                        return false;
                    }
                    isConfirmed = true;
                }
            }
            if (itor->id() == blsVoterSet.commonEchoMsg.blockId) {
                return m_workingCommitteeSet.verify(blsVoterSet);
            }
            if (EpochEndPoint::isEpochEndPoint(*itor)) {
                if (!isConfirmed) {
                    elog("DO NOT confirm EpochEndPoint : ${id} num : ${num}", ("id", itor->id())("num", BlockHeader::num_from_id(itor->id())));
                    return false;
                }
                auto checkPointItor = itor;
                checkPointItor++;
                if (checkPointItor != blockHeaderList.end()) {
                    if (!CheckPoint::isCheckPoint(*checkPointItor)) {
                        elog("CheckPoint is not the next block of EpochEndPoint, id : ${id} num : ${num}", ("id", checkPointItor->id())("num", BlockHeader::num_from_id(checkPointItor->id())));
                        return false;
                    }
                    CheckPoint cp(*checkPointItor);
                    m_workingCommitteeSet = cp.committeeSet();
                }
            }
        }
        elog("There are not any confirmed block");
        return false;
    }

    void LightClient::addCallback(std::shared_ptr<LightClientCallback> cb) {
        m_callback = cb;
    }

    bool LightClient::getStatus() const {
        return m_status;
    }

    void LightClient::handleGenesis(const BlockHeader& blockHeader) {
        // TODO sign check
        m_latestConfirmedBlockId = blockHeader.id();
        ULTRAIN_ASSERT(m_confirmedList.size() == 0, chain::chain_exception, "m_confirmedList size != 0");
        std::list<BlockHeader> genesisBlockHeader;
        genesisBlockHeader.push_back(blockHeader);
        onConfirmed(genesisBlockHeader);
    }

    bool LightClient::isOutOfRange(const BlockHeader& blockHeader) const {
        uint32_t blockNum = blockHeader.block_num();
        if (m_unconfirmedList.size() > 0 && (blockNum > m_unconfirmedList.front().block_num() + 1)) {
            return true;
        }
        ULTRAIN_ASSERT(m_latestConfirmedBlockId != BlockIdType(), chain::chain_exception, "latest confirmed block id is ${id}", ("id", BlockIdType()));
        if (blockNum <= BlockHeader::num_from_id(m_latestConfirmedBlockId)) {
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
        m_confirmedList.clear();
        m_status = true;
    }
}