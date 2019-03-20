#include <lightclient/LightClientProducer.h>

#include <ultrainio/chain/controller.hpp>
#include <lightclient/BlockHeaderExtKey.h>

namespace ultrainio {
    LightClientProducer::LightClientProducer() {}

    bool LightClientProducer::shouldBeConfirmed(const BlockHeader& blockHeader) const {
        if (EpochEndPoint::isEpochEndPoint(blockHeader)) {
            return true;
        }
        if (blockHeader.block_num() % m_confirmedInterval == 0) {
            return true;
        }
        return false;
    }

    bool LightClientProducer::hasNextTobeConfirmedBls() const {
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

    BlsVoterSet LightClientProducer::nextTobeConfirmedBls() const {
        return m_shouldBeConfirmedBlsVoterSetList.front();
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

    void LightClientProducer::handleConfirmPoint(ultrainio::chain::controller &chain) {
        chain.add_header_extensions_entry(kBlsVoterSet, nextTobeConfirmedBls().toVectorChar());
        ilog("add kBlsVoterSet to confirm ${confirmedBlockNum} in blockNum : ${blockNum}, set : ${set}",
             ("confirmedBlockNum", BlockHeader::num_from_id(nextTobeConfirmedBls().commonEchoMsg.blockId))
             ("blockNum", chain.head_block_num() + 1)("set", nextTobeConfirmedBls().toVectorChar()));
    }

    void LightClientProducer::handleEpochEndPoint(chain::controller& chain, const SHA256& mroot) {
        std::string s = std::string();
        std::vector<char> v(s.size());
        v.assign(s.begin(), s.end());
        chain.add_header_extensions_entry(kNextCommitteeMroot, v);
        ilog("add kNextCommitteeMroot, new mroot = ${new} in ${blockNum}", ("new", mroot)("blockNum", chain.head_block_num() + 1));
    }

    void LightClientProducer::acceptNewHeader(const BlockHeader& blockHeader, const BlsVoterSet& blsVoterSet) {
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
        std::cout << "m_shouldBeConfirmedList size = " <<m_shouldBeConfirmedList.size() << std::endl;
    }
}