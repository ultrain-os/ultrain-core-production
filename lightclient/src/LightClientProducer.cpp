#include <lightclient/LightClientProducer.h>

#include <ultrainio/chain/controller.hpp>
#include <ultrainio/chain/bls_votes.hpp>
#include <lightclient/BlockHeaderExtKey.h>
#include <lightclient/Helper.h>

namespace ultrainio {
    LightClientProducer::LightClientProducer(bls_votes_manager& blsVotesMgr) : m_BlsVotesMgr(blsVotesMgr) {}

    bool LightClientProducer::shouldBeConfirmed(const BlockHeader& blockHeader) const {
        if (EpochEndPoint::isEpochEndPoint(blockHeader)) {
            return true;
        }

        return m_BlsVotesMgr.should_be_confirmed(blockHeader.block_num());
    }

    bool LightClientProducer::hasNextTobeConfirmedBls(BlsVoterSet& blsVoterSet) const {
        std::string blsStr;
        bool res = m_BlsVotesMgr.has_should_be_confirmed_bls(blsStr);
        blsVoterSet = BlsVoterSet(blsStr);
        return res;
    }

    void LightClientProducer::handleCheckPoint(chain::controller& chain, const CommitteeSet& committeeSet) {
        chain.add_header_extensions_entry(kCommitteeSet, committeeSet.toVectorChar());
        if (BlockIdType() != m_latestCheckPointId) {
            std::string s = std::string(m_latestCheckPointId);
            std::vector<char> vc(s.begin(), s.begin());
            chain.add_header_extensions_entry(kPreCheckPointId, vc);
        }
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
        if (shouldBeConfirmed(blockHeader)) {
            m_BlsVotesMgr.add_confirmed_bls_votes(blockHeader.block_num(), EpochEndPoint::isEpochEndPoint(blockHeader), !blsVoterSet.empty(), blsVoterSet.toString());
        }

//        if (CheckPoint::isCheckPoint(blockHeader)) {
//            m_latestCheckPointId = blockHeader.id();
//        }

        if (ConfirmPoint::isConfirmPoint(blockHeader)) {
            ConfirmPoint confirmPoint(blockHeader);
            uint32_t confirmedBlockNum = BlockHeader::num_from_id(confirmPoint.confirmedBlockId());
            if (m_BlsVotesMgr.check_can_confirm(confirmedBlockNum)) {
                m_BlsVotesMgr.confirm(confirmedBlockNum);
            }
        }
    }
}