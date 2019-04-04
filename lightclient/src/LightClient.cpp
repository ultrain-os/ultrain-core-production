#include <lightclient/LightClient.h>

#include <iostream>

#include <crypto/Validator.h>
#include <lightclient/LightClientCallback.h>
#include <lightclient/Helper.h>

namespace ultrainio {
    LightClient::LightClient(uint64_t chainName) : m_chainName(chainName) {}

    uint64_t LightClient::chainName() const {
        return m_chainName;
    }

    void LightClient::setStartPoint(const StartPoint& startPoint) {
        reset();
        m_startPoint = startPoint;
        m_workingCommitteeSet = m_startPoint.committeeSet;
        m_latestConfirmedBlockId = m_startPoint.lastConfirmedBlockId;
        m_nextCommitteeMroot = m_startPoint.nextCommitteeMroot;
        ilog("set start point confirmed num : ${num}, committeeSet : ${committeeSet}", ("num", BlockHeader::num_from_id(m_latestConfirmedBlockId))("committeeSet", m_workingCommitteeSet.toString()));
    }

    // invoked by fetch block feature
    void LightClient::accept(const BlockHeader& blockHeader, const std::string& signature, const BlsVoterSet& blsVoterSet) {
        accept(blockHeader, signature);
        if (blsVoterSet.valid() && blsVoterSet.commonEchoMsg.blockId == blockHeader.id()) {
            confirm(blsVoterSet);
        }
    }

    void LightClient::accept(const BlockHeader& blockHeader, const std::string& signature) {
        ilog("accept BlockHeader num : ${blockNum}, id : ${id} latest confirm : ${latest} signature : ${s}",
             ("blockNum", blockHeader.block_num())("id", blockHeader.id())("latest", BlockHeader::num_from_id(m_latestConfirmedBlockId))("s", signature));
        if (Helper::isGenesis(blockHeader)) {
            handleGenesis(blockHeader, signature);
            return;
        }

        if (isOutOfRange(blockHeader)) {
            onError(LightClientError::kOutOfRange, blockHeader);
            return;
        }

        if (m_workingCommitteeSet == CommitteeSet()) {
            ULTRAIN_ASSERT(CheckPoint::isCheckPoint(blockHeader), chain::chain_exception, "DO NOT pass check point when working committee set is empty");
            m_workingCommitteeSet = CheckPoint(blockHeader).committeeSet();
            ULTRAIN_ASSERT(std::string(m_workingCommitteeSet.committeeMroot()) == m_nextCommitteeMroot, chain::chain_exception, "working committee set' MRoot not equal ${root}", ("root", m_nextCommitteeMroot));
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

            // MUST before EpochEndPoint::isEpochEndPoint check
            if (CheckPoint::isCheckPoint(*itor)) {
                CheckPoint cp(*itor);
                m_workingCommitteeSet = cp.committeeSet();
                if (m_nextCommitteeMroot != std::string(m_workingCommitteeSet.committeeMroot())) {
                    elog("Check Point error. expect mroot ${expect}, actual : ${actual}", ("expect", m_nextCommitteeMroot)("actual", std::string(m_workingCommitteeSet.committeeMroot())));
                    return false;
                }
            }

            if (EpochEndPoint::isEpochEndPoint(*itor)) {
                if (!isConfirmed) {
                    elog("DO NOT confirm EpochEndPoint : ${id} num : ${num}", ("id", itor->id())("num", BlockHeader::num_from_id(itor->id())));
                    return false;
                }
                EpochEndPoint eep(*itor);
                m_nextCommitteeMroot = eep.nextCommitteeMroot();
                auto checkPointItor = itor;
                checkPointItor++;
                if (checkPointItor != blockHeaderList.end()) {
                    if (!CheckPoint::isCheckPoint(*checkPointItor)) {
                        elog("CheckPoint is not the next block of EpochEndPoint, id : ${id} num : ${num}", ("id", checkPointItor->id())("num", BlockHeader::num_from_id(checkPointItor->id())));
                        return false;
                    }
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

    void LightClient::handleGenesis(const BlockHeader& blockHeader, const std::string& signature) {
//        if (!Validator::verify<BlockHeader>(Signature(signature), blockHeader, PublicKey(m_startPoint.genesisPk))) {
//            elog("signature : ${s} for blockNum : ${num}", ("s", signature)("num", blockHeader.block_num()));
//            onError(LightClientError::kSignatureError ,blockHeader);
//            m_status = false;
//        } else {
            m_latestConfirmedBlockId = blockHeader.id();
            std::list<BlockHeader> genesisBlockHeader;
            genesisBlockHeader.push_back(blockHeader);
            if (EpochEndPoint::isEpochEndPoint(blockHeader)) {
                m_nextCommitteeMroot = EpochEndPoint(blockHeader).nextCommitteeMroot();
            }
            onConfirmed(genesisBlockHeader);
        //}
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