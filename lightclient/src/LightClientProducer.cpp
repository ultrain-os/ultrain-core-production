#include <lightclient/LightClientProducer.h>

#include <ultrainio/chain/controller.hpp>
#include <ultrainio/chain/bls_votes.hpp>
#include <ultrainio/chain_plugin/chain_plugin.hpp>
#include <lightclient/BlockHeaderExtKey.h>
#include <lightclient/Helper.h>

namespace ultrainio {

    void LightClientProducer::setConfirmPointInterval(int interval) {
        bls_votes_manager::set_confirm_point_interval(interval);
    }

    LightClientProducer::LightClientProducer(bls_votes_manager& blsVotesMgr) : m_BlsVotesMgr(blsVotesMgr) {}

    bool LightClientProducer::shouldBeConfirmed(const BlockHeader& blockHeader) const {
        if (Helper::isGenesis(blockHeader)) {
            return false;
        }

        if (EpochEndPoint::isEpochEndPoint(blockHeader)) {
            return true;
        }

        return m_BlsVotesMgr.should_be_confirmed(blockHeader.block_num());
    }

    bool LightClientProducer::checkCanConfirm(uint32_t blockNum) const {
        return m_BlsVotesMgr.check_can_confirm(blockNum);
    }

    bool LightClientProducer::hasNextTobeConfirmedBls(BlsVoterSet& blsVoterSet) const {
        std::string blsStr;
        bool res = m_BlsVotesMgr.has_should_be_confirmed_bls(blsStr);
        blsVoterSet = BlsVoterSet(blsStr);
        return res;
    }

    void LightClientProducer::handleCheckPoint(chain::controller& chain, const CommitteeSet& committeeSet) {
        chain.add_header_extensions_entry(kCommitteeSet, committeeSet.toVectorChar());
        std::string s = std::string(m_BlsVotesMgr.get_latest_check_point_id());
        std::vector<char> vc(s.begin(), s.end());
        chain.add_header_extensions_entry(kPreCheckPointId, vc);
        ilog("add kCommitteeSet in blockNum : ${blockNum} : ${committeeset}", ("blockNum", chain.head_block_num() + 1)("committeeset", committeeSet.toString()));
    }

    void LightClientProducer::handleConfirmPoint(ultrainio::chain::controller &chain, const BlsVoterSet& blsVoterSet) {
        chain.add_header_extensions_entry(kBlsVoterSet, blsVoterSet.toVectorChar());
        ilog("add kBlsVoterSet to confirm ${confirmedBlockNum} in blockNum : ${blockNum}, set : ${set}",
             ("confirmedBlockNum", BlockHeader::num_from_id(blsVoterSet.commonEchoMsg.blockId))
             ("blockNum", chain.head_block_num() + 1)("set", blsVoterSet.toString()));
    }

    void LightClientProducer::handleEpochEndPoint(chain::controller& chain, const SHA256& mroot) {
        std::string s = std::string(mroot);
        std::vector<char> v(s.size());
        v.assign(s.begin(), s.end());
        chain.add_header_extensions_entry(kNextCommitteeMroot, v);
        ilog("add kNextCommitteeMroot, new mroot = ${new} in ${blockNum}", ("new", mroot)("blockNum", chain.head_block_num() + 1));
    }

    void LightClientProducer::acceptNewHeader(const BlockHeader& blockHeader, const BlsVoterSet& blsVoterSet) {
        if (Helper::isGenesis(blockHeader)) {
            m_BlsVotesMgr.confirm(blockHeader.block_num());
            return;
        }

        if (ConfirmPoint::isConfirmPoint(blockHeader)) {
            ConfirmPoint confirmPoint(blockHeader);
            uint32_t confirmedBlockNum = BlockHeader::num_from_id(confirmPoint.confirmedBlockId());
            ULTRAIN_ASSERT(checkCanConfirm(confirmedBlockNum), chain::chain_exception, "check to confirm block : ${num}", ("num", confirmedBlockNum));
            m_BlsVotesMgr.confirm(confirmedBlockNum);
        }

        if (shouldBeConfirmed(blockHeader)) {
            m_BlsVotesMgr.add_confirmed_bls_votes(blockHeader.block_num(), EpochEndPoint::isEpochEndPoint(blockHeader), blsVoterSet.valid(), blsVoterSet.toString());
        }

        if (CheckPoint::isCheckPoint(blockHeader)) {
            m_BlsVotesMgr.set_latest_check_point_id(blockHeader.id());
        }
    }

    void LightClientProducer::saveCurrentBlsVoterSet(const BlsVoterSet& blsVoterSet) {
        m_BlsVotesMgr.save_current_bls_votes(blsVoterSet.toString());
    }

    BlsVoterSet LightClientProducer::getCurrentBlsVoterSet() const {
        return BlsVoterSet(m_BlsVotesMgr.get_current_bls_votes());
    }

    BlockIdType LightClientProducer::getLatestCheckPointId() const {
        return m_BlsVotesMgr.get_latest_check_point_id();
    }

    CommitteeSet LightClientProducer::findCommitteeSet(const BlockIdType& blockId) const {
        chain::controller& chain = appbase::app().get_plugin<chain_plugin>().chain();
        uint32_t confirmedBlockNum = BlockHeader::num_from_id(blockId);
        BlockIdType latestCheckPointId = getLatestCheckPointId();
        int count = 5;
        while (--count > 0 && latestCheckPointId != BlockIdType()) {
            chain::signed_block_ptr blockPtr = chain.fetch_block_by_id(latestCheckPointId);
            uint32_t checkPointBlockNum = BlockHeader::num_from_id(latestCheckPointId);
            if (!blockPtr) {
                elog("can not found CheckPoint for block : ${num}, check we have the full block?", ("num", checkPointBlockNum));
                return CommitteeSet();
            }
            CheckPoint checkPoint(*blockPtr);
            if (checkPointBlockNum <= confirmedBlockNum) {
                return checkPoint.committeeSet();
            }
            latestCheckPointId = checkPoint.getPreCheckPointBlockId();
        }
        return CommitteeSet();
    }
}
