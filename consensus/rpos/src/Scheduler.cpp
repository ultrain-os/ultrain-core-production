#include <rpos/Scheduler.h>

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <random>

#include <boost/asio.hpp>
#include <fc/log/logger.hpp>
#include <fc/crypto/sha256.hpp>
#include <fc/io/json.hpp>
#include <fc/scoped_exit.hpp>

#include <appbase/application.hpp>
#include <ultrainio/chain_plugin/chain_plugin.hpp>
#include <ultrainio/chain/block_timestamp.hpp>
#include <ultrainio/chain/callback.hpp>
#include <ultrainio/chain/config.hpp>
#include <ultrainio/chain/controller.hpp>
#include <ultrainio/chain/exceptions.hpp>
#include <ultrainio/chain/global_property_object.hpp>
#include <ultrainio/chain/name.hpp>
#include <ultrainio/chain/config.hpp>

#include <base/Memory.h>
#include <crypto/Signer.h>
#include <crypto/Validator.h>
#include <core/Evidence.h>
#include <lightclient/CommitteeSet.h>
#include <lightclient/EpochEndPoint.h>
#include <lightclient/LightClientProducer.h>
#include <lightclient/LightClientMgr.h>
#include <rpos/Config.h>
#include <rpos/EvilBlsDetector.h>
#include <rpos/Genesis.h>
#include <rpos/MsgBuilder.h>
#include <rpos/MsgMgr.h>
#include <rpos/NativeTrx.h>
#include <rpos/Node.h>
#include <rpos/PunishMgr.h>
#include <rpos/Seed.h>
#include <rpos/StakeVoteBase.h>
#include <rpos/Utils.h>
#include <rpos/Vrf.h>
#include <appbase/application.hpp>

using namespace boost::asio;
using namespace std;
using namespace appbase;

namespace ultrainio {

    Scheduler::~Scheduler() {}

    Scheduler::Scheduler() : m_ba0Block(), m_proposerMsgMap(), m_echoMsgMap(),
                                           m_cacheProposeMsgMap(), m_cacheEchoMsgMap(),
                                           m_echoMsgAllPhase() {
        m_syncTaskTimer.reset(new boost::asio::steady_timer(app().get_io_service()));
        m_memleakCheck.reset(new boost::asio::steady_timer(app().get_io_service()));
        startMemleakCheck();
        chain::controller& chain = appbase::app().get_plugin<chain_plugin>().chain();
        m_lightClientProducer = std::make_shared<LightClientProducer>(chain.get_bls_votes_manager());
        m_currentBlsVoterSet = m_lightClientProducer->getCurrentBlsVoterSet();
    }

    chain::checksum256_type Scheduler::getCommitteeMroot(uint32_t block_num) {
        std::shared_ptr<StakeVoteBase> voterSysPtr = MsgMgr::getInstance()->getStakeVote(block_num);
        if (voterSysPtr) {
            return voterSysPtr->getCommitteeMroot();
        } else {
            return chain::checksum256_type();
        }
    }

    void Scheduler::startMemleakCheck() {
      m_memleakCheck->expires_from_now(m_memleakCheckPeriod);
      m_memleakCheck->async_wait( [this](boost::system::error_code ec) {
            if( !ec) {
                startMemleakCheck();
                int s1 = m_proposerMsgMap.size();

                int s2 = m_echoMsgMap.size();
                int s3 = 0;
                for(const auto& it : m_echoMsgMap) {
                    s3 += it.second.accountPool.size();
                    s3 += it.second.sigPool.size();
                    s3 += it.second.timePool.size();
                }

                int s4 = m_cacheProposeMsgMap.size();
                int s5 = 0;
                for(const auto& it: m_cacheProposeMsgMap) {
                    s5 += it.second.size();
                }

                int s6 = m_cacheEchoMsgMap.size();
                int s7 = 0;
                for(const auto& it: m_cacheEchoMsgMap) {
                    s7 += it.second.size();
                }

                int s8 = m_echoMsgAllPhase.size();
                int s9 = 0;
                for(const auto& it : m_echoMsgAllPhase) {
                    for( const auto& it2: it.second) {
                        s9 += it2.second.accountPool.size();
                        s9 += it2.second.sigPool.size();
                        s9 += it2.second.timePool.size();
                    }
                }

                int s10 = 0;
                auto mm = MsgMgr::getInstance();
                if (mm) {
                    s10 = mm->m_blockMsgPoolMap.size();
                }
                ilog("memleak check m_proposerMsgMap ${1}, m_echoMsgMap ${2} ${3} m_cacheProposeMsgMap ${4} ${5} m_cacheEchoMsgMap ${6} ${7} m_echoMsgAllPhase ${8} ${9} m_blockMsgPoolMap ${10}",
                     ("1", s1)("2", s2)("3", s3)("4", s4)("5", s5)("6", s6)
                     ("7", s7)("8", s8)("9", s9)("10", s10));
            }
            else {
               elog( "Error from memleak check monitor: ${m}",( "m", ec.message()));
               startMemleakCheck( );
            }
         });
    }

    void Scheduler::reset() {
        uint32_t blockNum = getLastBlocknum();
        m_ba0Block = Block();
        m_ba0VerifiedBlkId = BlockIdType();
        m_ba0FailedBlkId = BlockIdType();
        clearPreRunStatus();
        m_proposerMsgMap.clear();
        m_echoMsgMap.clear();
        clearMsgCache(m_cacheProposeMsgMap, blockNum);
        clearMsgCache(m_cacheEchoMsgMap, blockNum);
        clearMsgCache(m_echoMsgAllPhase, blockNum);
    }

    void Scheduler::resetEcho() {
        m_echoMsgMap.clear();
    }

    bool Scheduler::insert(const EchoMsg &echo) {
        auto itor = m_echoMsgMap.find(echo.blockId);
        if (itor != m_echoMsgMap.end()) {
            auto pkItor = std::find(itor->second.accountPool.begin(), itor->second.accountPool.end(), echo.account);
            if (pkItor == itor->second.accountPool.end()) {
                itor->second.accountPool.push_back(echo.account);
                itor->second.sigPool.push_back(echo.signature);
                itor->second.timePool.push_back(echo.timestamp);
                itor->second.blsSignPool.push_back(echo.blsSignature);
            }
        } else {
            VoterSet voterSet;
            voterSet.commonEchoMsg = echo;
            voterSet.accountPool.push_back(echo.account);
            voterSet.sigPool.push_back(echo.signature);
            voterSet.timePool.push_back(echo.timestamp);
            voterSet.blsSignPool.push_back(echo.blsSignature);
            voterSet.hasSend = true;
            m_echoMsgMap.insert(make_pair(echo.blockId, voterSet));
        }
        return true;
    }

    bool Scheduler::insert(const ProposeMsg &propose) {
        dlog("insert.save propose msg.blockhash = ${blockhash}", ("blockhash", short_hash(propose.block.id())));
        m_proposerMsgMap.insert(make_pair(propose.block.id(), propose));
        return true;
    }

    bool Scheduler::isLaterMsg(const EchoMsg &echo) const {
        uint32_t currentBlockNum = Node::getInstance()->getBlockNum();
        ConsensusPhase currentPhase = Node::getInstance()->getPhase();
        uint32_t currentBaxCount = Node::getInstance()->getBaxCount();

        uint32_t blockNum = BlockHeader::num_from_id(echo.blockId);
        if (blockNum > currentBlockNum) {
            return true;
        }

        if (blockNum == currentBlockNum && (echo.phase + echo.baxCount > currentPhase + currentBaxCount)) {
            return true;
        }

        return false;
    }

    bool Scheduler::isLaterMsg(const ProposeMsg& propose) const {
        uint32_t currentBlockNum = Node::getInstance()->getBlockNum();
        ConsensusPhase currentPhase = Node::getInstance()->getPhase();
        if (propose.block.block_num() > currentBlockNum
                || (propose.block.block_num() == currentBlockNum && currentPhase == kPhaseInit)) {
            return true;
        }
        return false;
    }

    bool Scheduler::processLaterMsg(const EchoMsg& echo) {
        RoundInfo info(BlockHeader::num_from_id(echo.blockId), echo.phase + echo.baxCount);
        auto itor = m_cacheEchoMsgMap.find(info);
        if (itor == m_cacheEchoMsgMap.end()) {
            if (m_cacheEchoMsgMap.size() >= m_maxCacheEcho) {
                //NOTE: itor will be invalid after the operation below.
                clearOldCachedEchoMsg();
            }
            std::vector<EchoMsg> echoV;
            echoV.push_back(echo);
            m_cacheEchoMsgMap.insert(make_pair(info, echoV));
        } else {
            if (itor->second.size() < m_maxCommitteeSize) {
                itor->second.emplace_back(echo);
            } else {
                ilog("Size of vector in m_cacheEchoMsgMap exceeds ${mcs}", ("mcs", m_maxCommitteeSize));
            }
        }
        return true;
    }

    bool Scheduler::duplicated(const EchoMsg& echo) const {
        RoundInfo info(echo.blockNum(), echo.phase + echo.baxCount);
        auto itor = m_cacheEchoMsgMap.find(info);
        if (itor != m_cacheEchoMsgMap.end()) {
            for (auto e : itor->second) {
                if (e == echo) {
                    return true;
                }
            }
        }
        return false;
    }

    bool Scheduler::satisfyVoteRules(const EchoMsg& echo) const {
        /*
         * 1. In BA0, Voter can vote multi propose, but not great the number of proposer
         * 2. In the other, voter can vote one
         */
        RoundInfo info(echo.blockNum(), echo.phase + echo.baxCount);
        auto itor = m_cacheEchoMsgMap.find(info);
        if (itor != m_cacheEchoMsgMap.end()) {
            int existCount = 0;
            for (auto e : itor->second) {
                if (e.account == echo.account) {
                    existCount++;
                }
            }
            if ((echo.phase == kPhaseBA0 && existCount >= Config::kDesiredProposerNumber) || (echo.phase != kPhaseBA0 && existCount >= 1)) {
                return false;
            }
        }
        return true;
    }

    bool Scheduler::duplicated(const ProposeMsg& propose) const {
        RoundInfo info(propose.block.block_num(), kPhaseBA0);
        auto itor = m_cacheProposeMsgMap.find(info);
        if (itor != m_cacheProposeMsgMap.end()) {
            for (auto e : itor->second) {
                if (e.block.id() == propose.block.id()) {
                    return true;
                }
            }
        }
        return false;
    }

    bool Scheduler::processLaterMsg(const ProposeMsg& propose) {
        RoundInfo info(propose.block.block_num(), kPhaseBA0);
        auto itor = m_cacheProposeMsgMap.find(info);
        if (itor == m_cacheProposeMsgMap.end()) {
            if (m_cacheProposeMsgMap.size() >= m_maxCachePropose) {
                //NOTE: itor will be invalid after the operation below.
                clearOldCachedProposeMsg();
            }
            std::vector<ProposeMsg> proposeV;
            proposeV.push_back(propose);
            m_cacheProposeMsgMap.insert(make_pair(info, proposeV));
        } else {
            if (itor->second.size() < m_maxCommitteeSize) {
                itor->second.emplace_back(propose);
            }
            else {
                wlog("Size of vector in cacheProposeMsgMap exceeds ${mcs}", ("mcs", m_maxCommitteeSize));
            }
        }
        return true;
    }

    bool Scheduler::sameBlockNumAndPhase(const ProposeMsg& propose) const {
        return propose.block.block_num() == Node::getInstance()->getBlockNum() && kPhaseBA0 == Node::getInstance()->getPhase();
    }

    bool Scheduler::loopback(const EchoMsg& echo) const {
        return StakeVoteBase::getMyAccount() == echo.account;
    }

    bool Scheduler::sameBlockNumButBeforePhase(const EchoMsg &echo) const {
        if (BlockHeader::num_from_id(echo.blockId) == Node::getInstance()->getBlockNum()
                && (echo.phase + echo.baxCount < Node::getInstance()->getPhase() + Node::getInstance()->getBaxCount())) {
            return true;
        }
        return false;
    }

    bool Scheduler::sameBlockNumAndPhase(const EchoMsg& echo) const {
        return BlockHeader::num_from_id(echo.blockId) == Node::getInstance()->getBlockNum()
                && echo.phase == Node::getInstance()->getPhase() && echo.baxCount == Node::getInstance()->getBaxCount();
    }

    bool Scheduler::obsolete(const EchoMsg& echo) const {
        return BlockHeader::num_from_id(echo.blockId) < Node::getInstance()->getBlockNum();
    }

    bool Scheduler::processBeforeMsg(const EchoMsg &echo) {
        RoundInfo info(BlockHeader::num_from_id(echo.blockId), echo.phase + echo.baxCount);
        BlockIdVoterSetMap echo_msg_map;

        if (echo.phase == kPhaseBA0) {
            return false;
        }

        dlog("processBeforeMsg. account : ${account} blockNum : ${blockNum}, phase : ${phase} baxCount : ${count} id : ${id}",
             ("account", std::string(echo.account))("blockNum", BlockHeader::num_from_id(echo.blockId))("phase", static_cast<int>(echo.phase))
             ("count", echo.baxCount)("id", short_hash(echo.blockId)));

        auto map_it = m_echoMsgAllPhase.find(info);
        if (map_it == m_echoMsgAllPhase.end()) {
            if (m_echoMsgAllPhase.size() >= m_maxCachedAllPhaseKeys) {
                dlog("processBeforeMsg.map reach the up limit. size = ${size}",("size",m_echoMsgAllPhase.size()));
                return false;
            }
            auto result = m_echoMsgAllPhase.insert(make_pair(info, echo_msg_map));
            map_it = result.first;
        }

        auto itor = map_it->second.find(echo.blockId);
        if (itor != map_it->second.end()) {
            //dlog("processBeforeMsg.blockhash is already exist.");
            if (updateAndMayResponse(itor->second, echo, false)) {
                if (isMinEcho(itor->second, map_it->second) || isMinFEcho(itor->second, map_it->second)) {
                    return true;
                }
            }
        } else {
            VoterSet voterSet;
            voterSet.commonEchoMsg = echo;
            //dlog("processBeforeMsg.new blockhash.");
            if (updateAndMayResponse(voterSet, echo, false)) {
                map_it->second.insert(make_pair(echo.blockId, voterSet));
                if (isMinEcho(voterSet, map_it->second) || isMinFEcho(voterSet, map_it->second)) {
                    return true;
                }
            }
        }
        return false;
    }

    bool Scheduler::updateAndMayResponse(VoterSet& voterSet, const EchoMsg &echo, bool response) {
        uint32_t blockNum = BlockHeader::num_from_id(echo.blockId);
        //ilog("update echo blockId: ${id}", ("id", short_hash(echo.blockId)));
        auto pkItor = std::find(voterSet.accountPool.begin(), voterSet.accountPool.end(), echo.account);
        if (pkItor == voterSet.accountPool.end()) {
            voterSet.accountPool.push_back(echo.account);
            voterSet.blsSignPool.push_back(echo.blsSignature);
            voterSet.sigPool.push_back(echo.signature);
            voterSet.timePool.push_back(echo.timestamp);
            std::shared_ptr<StakeVoteBase> stakeVotePtr = MsgMgr::getInstance()->getStakeVote(blockNum);
            if (response && voterSet.getTotalVoterWeight() >= stakeVotePtr->getSendEchoThreshold() && !voterSet.hasSend
                && Node::getInstance()->getPhase() == kPhaseBA0 && isMinFEcho(voterSet)) {
                if (MsgMgr::getInstance()->isVoter(Node::getInstance()->getBlockNum(), echo.phase,
                                                           echo.baxCount)) {
                    ilog("send echo when > f + 1");
                    voterSet.hasSend = true;
                    EchoMsg myEcho = MsgBuilder::constructMsg(echo);
                    ULTRAIN_ASSERT(verifyMyBlsSignature(myEcho), chain::chain_exception, "bls signature error, check bls private key pls");
                    insert(myEcho);
                    Node::getInstance()->sendMessage(myEcho);
                }
            }
            return true;
        }
        return false;
    }

    bool Scheduler::isNeedSync() {
        if (m_cacheEchoMsgMap.empty()) {
            return false;
        }

        if (Node::getInstance()->isSyncing()) {
            elog("node is syncing.");
            return false;
        }

        uint32_t maxBlockNum = Node::getInstance()->getBlockNum();

        for (auto vector_itor = m_cacheEchoMsgMap.begin(); vector_itor != m_cacheEchoMsgMap.end(); ++vector_itor) {
            if (vector_itor->first.blockNum > maxBlockNum) {
                BlockIdVoterSetMap echo_msg_map;

                for (auto &echo : vector_itor->second) {
                    auto itor = echo_msg_map.find(echo.blockId);
                    if (itor != echo_msg_map.end()) {
                        insertAccount(itor->second, echo);
                    } else {
                        VoterSet voterSet;
                        voterSet.commonEchoMsg = echo;
                        insertAccount(voterSet, echo);
                        echo_msg_map.insert(make_pair(echo.blockId, voterSet));
                    }
                }

                for (auto echo_itor = echo_msg_map.begin(); echo_itor != echo_msg_map.end(); ++echo_itor) {
                    if (echo_itor->second.accountPool.size() >= THRESHOLD_SYNCING) {
                        maxBlockNum = vector_itor->first.blockNum;
                        return true;
                    }
                }
            }
        }

        return false;
    }

    bool Scheduler::isChangePhase() {
        if (m_cacheEchoMsgMap.empty()) {
            return false;
        }

        std::shared_ptr<StakeVoteBase> stakeVotePtr = MsgMgr::getInstance()->getStakeVote(Node::getInstance()->getBlockNum());
        for (auto vector_itor = m_cacheEchoMsgMap.begin(); vector_itor != m_cacheEchoMsgMap.end(); ++vector_itor) {
            if ((vector_itor->first.blockNum == Node::getInstance()->getBlockNum())
                && (vector_itor->first.phase >= Config::kMaxBaxCount)) {
                BlockIdVoterSetMap echo_msg_map;
                for (auto &echo : vector_itor->second) {
                    auto itor = echo_msg_map.find(echo.blockId);
                    if (itor != echo_msg_map.end()) {
                        updateAndMayResponse(itor->second, echo, false);
                    } else {
                        VoterSet voterSet;
                        voterSet.commonEchoMsg = echo;
                        updateAndMayResponse(voterSet, echo, false);
                        echo_msg_map.insert(make_pair(echo.blockId, voterSet));
                    }
                }

                for (auto echo_itor = echo_msg_map.begin(); echo_itor != echo_msg_map.end(); ++echo_itor) {
                    if (echo_itor->second.getTotalVoterWeight() >= stakeVotePtr->getSendEchoThreshold()) {
                        return true;
                    }
                }
            }
        }

        return false;
    }

    bool Scheduler::isValid(const EchoMsg &echo) const {
        uint32_t blockNum = BlockHeader::num_from_id(echo.blockId);
        std::shared_ptr<StakeVoteBase> stakeVotePtr = MsgMgr::getInstance()->getStakeVote(blockNum);
        PublicKey publicKey = stakeVotePtr->getPublicKey(echo.account);
        if (!Validator::verify<UnsignedEchoMsg>(Signature(echo.signature), echo, publicKey)) {
            elog("validator echo error. account : ${account} pk : ${pk} signature : ${signature}",
                    ("account", std::string(echo.account))("pk", std::string(publicKey))("signature", echo.signature));
            return false;
        }
        if (!stakeVotePtr->isVoter(echo.account, echo.phase, echo.baxCount, Node::getInstance()->getNonProducingNode())) {
            elog("send echo by no voter. account : ${account}", ("account", std::string(echo.account)));
            return false;
        }
//        if (!stakeVotePtr->isProposer(echo.proposer, Node::getInstance()->getNonProducingNode())) {
//            elog("echo an propose message, proposed by no proposer. account : ${account}", ("account", std::string(echo.proposer)));
//            return false;
//        }
        return true;
    }

    bool Scheduler::loopback(const ProposeMsg& propose) const {
        return StakeVoteBase::getMyAccount() == propose.block.proposer;
    }

    bool Scheduler::obsolete(const ProposeMsg& propose) const {
        uint32_t blockNum = propose.block.block_num();
        uint32_t myBlockNum = Node::getInstance()->getBlockNum();
        return blockNum < myBlockNum || (blockNum == myBlockNum && Node::getInstance()->getPhase() > kPhaseBA0);
    }

    bool Scheduler::isValid(const ProposeMsg& propose) const {
        if (propose.block.previous != getPreviousBlockhash()) {
            elog("block ${blockId} 's previous hash ${previous} not equal current head : ${head}",
                    ("blockId", propose.block.id())("previous", propose.block.previous)("head", getPreviousBlockhash()));
            return false;
        }

        std::shared_ptr<StakeVoteBase> stakeVotePtr = MsgMgr::getInstance()->getStakeVote(propose.block.block_num());
        PublicKey publicKey = stakeVotePtr->getPublicKey(propose.block.proposer);
        if (!Validator::verify<BlockHeader>(Signature(propose.block.signature), propose.block, publicKey)) {
            elog("validator proposer error. proposer : ${proposer}", ("proposer", std::string(propose.block.proposer)));
            return false;
        }

        if (!stakeVotePtr->isProposer(propose.block.proposer, Node::getInstance()->getNonProducingNode())) {
            elog("send propose by non proposer. account : ${account}", ("account", std::string(propose.block.proposer)));
            return false;
        }

        const auto mroot = stakeVotePtr->getCommitteeMroot();
        if (mroot != propose.block.committee_mroot) {
            elog("verify committee mroot error. own c_mroot : ${m1}, block mroot ${m2}",
                 ("m1", mroot) ("m2", propose.block.committee_mroot));
            return false;
        }

        if (ConfirmPoint::isConfirmPoint(propose.block)) {
            ConfirmPoint confirmPoint(propose.block);
            BlockIdType blockId = confirmPoint.confirmedBlockId();
            uint32_t confirmedBlockNum = BlockHeader::num_from_id(blockId);
            if (!m_lightClientProducer->checkCanConfirm(confirmedBlockNum)) {
                elog("try to confirm wrong block num : ${num}", ("num", confirmedBlockNum));
                return false;
            }
            CommitteeSet c = m_lightClientProducer->findCommitteeSet(blockId);
            if (!c.empty()) {
                BlsVoterSet blsVoterSet = confirmPoint.blsVoterSet();
                if (!c.verify(blsVoterSet)) {
                    // TODO(xiaofen) punish
                    elog("BlsVoterSet error for : ${num} from ${who}", ("num", propose.block.block_num())("who", std::string(propose.block.proposer)));
                    return false;
                }
            }
        }
        return true;
    }

    bool Scheduler::fastHandleMessage(const ProposeMsg &propose) {
        try {
            ULTRAIN_ASSERT(sameBlockNumAndPhase(propose), chain::chain_exception, "unexpected result");

            if (!isValid(propose)) {
                return false;
            }

            std::shared_ptr<PunishMgr> punishMgrPtr = PunishMgr::getInstance();
            if (punishMgrPtr->isPunished(propose.block.proposer)) {
                ilog("${account} has been punished", ("account", std::string(propose.block.proposer)));
                return false;
            }

            MultiProposeEvidence evidence;
            if (m_evilMultiProposeDetector.hasMultiPropose(m_proposerMsgMap, propose, evidence)) {
                ilog("${account} sign multiple propose message", ("account", std::string(evidence.getEvilAccount())));
                EvilDesc evilDesc(appbase::app().get_plugin<chain_plugin>().get_chain_name(), evidence.getEvilAccount(),
                                  BlockHeader::num_from_id(propose.block.id()));
                NativeTrx::reportEvil(evilDesc, evidence);
                punishMgrPtr->punish(evidence.getEvilAccount(), Evidence::kMultiPropose);
                return false;
            }

            auto itor = m_proposerMsgMap.find(propose.block.id());
            if (itor == m_proposerMsgMap.end()) {
                if (isMinPropose(propose)) {
                    m_proposerMsgMap.insert(make_pair(propose.block.id(), propose));
                    return true;
                }
            }
            return false;
        } catch (const fc::exception& e) {
            edump((e.to_detail_string()));
            return false;
        }
    }

    bool Scheduler::fastHandleMessage(const EchoMsg &echo) {
        try {
            ULTRAIN_ASSERT(sameBlockNumAndPhase(echo), chain::chain_exception, "unexpected result");

            if (!isValid(echo)) {
                return false;
            }

            if (m_fastTimestamp < echo.timestamp) {
                m_fastTimestamp = echo.timestamp;
            }

            std::shared_ptr<PunishMgr> punishMgrPtr = PunishMgr::getInstance();
            if (punishMgrPtr->isPunished(echo.account)) {
                ilog("${account} has been punished", ("account", std::string(echo.account)));
                return false;
            }

            MultiVoteEvidence evidence;
            if (m_evilMultiVoteDetector.hasMultiVote(m_echoMsgMap, echo, evidence)) {
                ilog("${account} vote multiple block", ("account", std::string(evidence.getEvilAccount())));
                EvilDesc evilDesc(appbase::app().get_plugin<chain_plugin>().get_chain_name(), evidence.getEvilAccount(),
                                  echo.blockNum());
                NativeTrx::reportEvil(evilDesc, evidence);
                punishMgrPtr->punish(echo.account, Evidence::kMultiVote);
                return false;
            }

            auto itor = m_echoMsgMap.find(echo.blockId);
            if (itor != m_echoMsgMap.end()) {
                updateAndMayResponse(itor->second, echo, false);
            } else {
                VoterSet voterSet;
                voterSet.commonEchoMsg = echo;
                updateAndMayResponse(voterSet, echo, false);
                m_echoMsgMap.insert(make_pair(echo.blockId, voterSet));
            }
            return true;
        } catch (const fc::exception& e) {
            edump((e.to_detail_string()));
            return false;
        }
    }

    bool Scheduler::handleMessage(const ProposeMsg& propose) {
        try {
            if (loopback(propose)) {
                return false;
            }

            if (obsolete(propose)) {
                return false;
            }

            if (isLaterMsg(propose)) {
                if (m_evilDDosDetector.evil(propose, Node::getInstance()->getRoundCount(),
                                            Node::getInstance()->getBlockNum())) {
                    elog("evil propose id : ${id} : blockNum : ${blockNum} local : ${local}",
                         ("id", propose.block.id())("blockNum", propose.block.block_num())("local",
                                                                                           Node::getInstance()->getBlockNum()));
                    return false;
                }
                if (duplicated(propose)) {
                    return false;
                }
                std::shared_ptr<StakeVoteBase> stakeVotePtr = MsgMgr::getInstance()->getStakeVote(
                        Node::getInstance()->getBlockNum());
                PublicKey publicKey = stakeVotePtr->getPublicKey(propose.block.proposer);
                if (!publicKey.isValid()) {
                    elog("can not find pk of proposer : ${p} at block ${num}",
                         ("p", std::string(propose.block.proposer))("num", Node::getInstance()->getBlockNum()));
                    return false;
                }
                if (!Validator::verify<BlockHeader>(Signature(propose.block.signature), propose.block, publicKey)) {
                    elog("validator proposer error. proposer : ${p} at block ${num} sig : ${sig}",
                         ("p", std::string(propose.block.proposer))("num", Node::getInstance()->getBlockNum())("sig",
                                                                                                               short_sig(
                                                                                                                       propose.block.signature)));
                    return false;
                }
                return processLaterMsg(propose);
            }

            ULTRAIN_ASSERT(sameBlockNumAndPhase(propose), chain::chain_exception, "unexpected result");

            if (!isValid(propose)) {
                return false;
            }

            if (Node::getInstance()->isSyncing()) {
                dlog("receive propose msg. node is syncing. blockhash = ${blockhash}",
                     ("blockhash", short_hash(propose.block.id())));
                return true;
            }

            std::shared_ptr<PunishMgr> punishMgrPtr = PunishMgr::getInstance();
            if (punishMgrPtr->isPunished(propose.block.proposer)) {
                ilog("${account} has been punished", ("account", std::string(propose.block.proposer)));
                return false;
            }

            MultiProposeEvidence evidence;
            if (m_evilMultiProposeDetector.hasMultiPropose(m_proposerMsgMap, propose, evidence)) {
                ilog("${account} sign multiple propose message", ("account", std::string(propose.block.proposer)));
                EvilDesc evilDesc(appbase::app().get_plugin<chain_plugin>().get_chain_name(), evidence.getEvilAccount(),
                                  BlockHeader::num_from_id(propose.block.id()));
                NativeTrx::reportEvil(evilDesc, evidence);
                punishMgrPtr->punish(evidence.getEvilAccount(), Evidence::kMultiPropose);
                return false;
            }

            dlog("receive propose msg.blockhash = ${blockhash}", ("blockhash", short_hash(propose.block.id())));
            auto itor = m_proposerMsgMap.find(propose.block.id());
            if (itor == m_proposerMsgMap.end()) {
                if (isMinPropose(propose)) {
                    if (MsgMgr::getInstance()->isVoter(propose.block.block_num(), kPhaseBA0, 0)) {
                        EchoMsg echo = MsgBuilder::constructMsg(propose);
                        ULTRAIN_ASSERT(verifyMyBlsSignature(echo), chain::chain_exception,
                                       "bls signature error, check bls private key pls");
                        Node::getInstance()->sendMessage(echo);
                        insert(echo);
                    }
                    dlog("save propose msg.blockhash = ${blockhash}", ("blockhash", short_hash(propose.block.id())));
                    m_proposerMsgMap.insert(make_pair(propose.block.id(), propose));
                    return true;
                }
            }
            return false;
        } catch (const fc::exception& e) {
            edump((e.to_detail_string()));
            return false;
        }
    }

    bool Scheduler::handleMessage(const EchoMsg &echo) {
        try {
            if (!echo.valid()) {
                elog("echo check failed. account : ${account}, phase : ${phase}, baxCount : ${baxCount}",
                        ("account", std::string(echo.account))("phase", static_cast<int>(echo.phase))("baxCount", echo.baxCount));
                return false;
            }

            if (loopback(echo)) {
                elog("loopback echo. account : ${account}", ("account", std::string(echo.account)));
                return false;
            }

            if (obsolete(echo)) {
                return false;
            }

            if (isLaterMsg(echo)) {
                if (m_evilDDosDetector.evil(echo, Node::getInstance()->getRoundCount(),
                                            Node::getInstance()->getBlockNum())) {
                    elog("evil echo : blockNum : ${blockNum} local : ${local}",
                         ("blockNum", BlockHeader::num_from_id(echo.blockId))("local",
                                                                              Node::getInstance()->getBlockNum()));
                    return false;
                }
                if (duplicated(echo)) {
                    elog("duplicate echo message from account : ${account}, blockNum : ${n}, phase : ${p}",
                         ("account", std::string(echo.account))("n", BlockHeader::num_from_id(echo.blockId))("p", static_cast<int>(echo.phase)));
                    return false;
                }
                if (!satisfyVoteRules(echo)) {
                    // TODO(qinxiaofen) how to punish
                    elog("echo does not satisfy rules. message from account : ${account}, blockNum : ${n}, phase : ${p}",
                         ("account", std::string(echo.account))("n", BlockHeader::num_from_id(echo.blockId))("p", static_cast<int>(echo.phase)));
                    return false;
                }
                std::shared_ptr<StakeVoteBase> stakeVotePtr = MsgMgr::getInstance()->getStakeVote(
                        Node::getInstance()->getBlockNum());
                PublicKey publicKey = stakeVotePtr->getPublicKey(echo.account);
                if (!publicKey.isValid()) {
                    elog("can not find pk of account : ${account} at block ${num}",
                         ("account", std::string(echo.account))("num", Node::getInstance()->getBlockNum()));
                    return false;
                }
                if (!Validator::verify<UnsignedEchoMsg>(Signature(echo.signature), echo, publicKey)) {
                    elog("validator echo error. account : ${account} at block : ${num} sig : ${sig}",
                         ("account", std::string(echo.account))("num", Node::getInstance()->getBlockNum())("sig",
                                                                                                           short_sig(
                                                                                                                   echo.signature)));
                    return false;
                }
                m_evilDDosDetector.gatherWhenBax(echo, Node::getInstance()->getBlockNum(),
                                                 Node::getInstance()->getPhase());
                return processLaterMsg(echo);
            }

            if (sameBlockNumButBeforePhase(echo)) {
                if (!isValid(echo)) {
                    return false;
                }
                m_evilDDosDetector.gatherWhenBax(echo, Node::getInstance()->getBlockNum(),
                                                 Node::getInstance()->getPhase());
                return processBeforeMsg(echo);
            }

            ULTRAIN_ASSERT(sameBlockNumAndPhase(echo), chain::chain_exception, "unexpected result");

            if (!isValid(echo)) {
                return false;
            }

            if ((Node::getInstance()->isSyncing()) && (Node::getInstance()->getPhase() != kPhaseBAX)) {
                dlog("receive echo msg. node is syncing. blockhash = ${blockhash} echo'account = ${account}",
                     ("blockhash", short_hash(echo.blockId))("account", std::string(echo.account)));
                return true;
            }

            std::shared_ptr<PunishMgr> punishMgrPtr = PunishMgr::getInstance();
            if (punishMgrPtr->isPunished(echo.account)) {
                ilog("${account} has been punished", ("account", std::string(echo.account)));
                return false;
            }

            MultiVoteEvidence evidence;
            if (m_evilMultiVoteDetector.hasMultiVote(m_echoMsgMap, echo, evidence)) {
                ilog("${account} vote multiple block", ("account", std::string(evidence.getEvilAccount())));
                EvilDesc evilDesc(appbase::app().get_plugin<chain_plugin>().get_chain_name(), evidence.getEvilAccount(),
                                  echo.blockNum());
                NativeTrx::reportEvil(evilDesc, evidence);
                punishMgrPtr->punish(echo.account, Evidence::kMultiVote);
                return false;
            }
            m_evilDDosDetector.gatherWhenBax(echo, Node::getInstance()->getBlockNum(), Node::getInstance()->getPhase());

            auto itor = m_echoMsgMap.find(echo.blockId);
            bool bret;
            if (itor != m_echoMsgMap.end()) {
                bret = updateAndMayResponse(itor->second, echo, true);
                if ((isMinEcho(itor->second) || isMinFEcho(itor->second)) && bret) {
                    return true;
                }
            } else {
                VoterSet voterSet;
                voterSet.commonEchoMsg = echo;
                bret = updateAndMayResponse(voterSet, echo, true);
                m_echoMsgMap.insert(make_pair(echo.blockId, voterSet));
                if ((isMinEcho(voterSet) || isMinFEcho(voterSet)) && bret) {
                    return true;
                }
            }
            return false;
        } catch (const fc::exception& e) {
            edump((e.to_detail_string()));
            return false;
        }
    }

    bool Scheduler::handleMessage(const fc::sha256 &nodeId, const ReqSyncMsg &msg) {
        if (Node::getInstance()->isSyncing()) {
            return true;
        }

        if (nodeId == fc::sha256() || m_syncTaskQueue.size() >= m_maxSyncClients) {
            ilog("peer node id is empty or sync task queue is full. node id:${n} queue size:${qz}",
                 ("n", nodeId)("qz", m_syncTaskQueue.size()));
            return false;
        }

        uint32_t last_block_num = getLastBlocknum();
        for (std::list<SyncTask>::iterator l_it = m_syncTaskQueue.begin(); l_it != m_syncTaskQueue.end(); ++l_it) {
            if (l_it->nodeId == nodeId) {
                if (l_it->startBlock == last_block_num + 1 && Node::getInstance()->getBaxCount() > 0) { // When the whole chain blocks at bax, we erase the old task and will add new one.
                    m_syncTaskQueue.erase(l_it);
                    break;
                } else {
                    ilog("peer node ${node} has been already in sync queue.", ("node", nodeId));
                    return false;
                }
            }
        }

        uint32_t end_block_num = msg.endBlockNum <= last_block_num + 1 ? msg.endBlockNum : last_block_num + 1;
        chain::controller &chain = appbase::app().get_plugin<chain_plugin>().chain();
        uint32_t max_count = m_maxPacketsOnce / 3;
        uint32_t send_count = 0;
        uint32_t num = msg.startBlockNum;

        SyncBlockMsg sync_block;
        sync_block.seqNum = msg.seqNum;
        for (; num <= end_block_num && send_count < max_count; num++, send_count++) {
            auto b = chain.fetch_block_by_number(num);
            if (b) {
                sync_block.block = *b;
                if (num == last_block_num) {
                    sync_block.proof = m_currentBlsVoterSet.toString();
                    ilog("send last block, voter set: ${s}", ("s", m_currentBlsVoterSet.toString()));
                }
                Node::getInstance()->sendMessage(nodeId, sync_block);
            } else if (num == end_block_num) { // try to send last block next time
                break;
            } else {// else: skip the block if not exist
                wlog("block ${n} does not exist", ("n", num));
            }
        }

        if (num <= end_block_num) {
            if (end_block_num > num + m_maxSyncBlocks) {
                end_block_num = num + m_maxSyncBlocks;
            }
            m_syncTaskQueue.emplace_back(last_block_num, m_currentBlsVoterSet, nodeId, num, end_block_num, msg.seqNum);
        }

        return true;
    }

    bool Scheduler::handleMessage(const fc::sha256 &nodeId, const ReqBlockNumRangeMsg &msg) {
        RspBlockNumRangeMsg rsp_msg;
        rsp_msg.seqNum = msg.seqNum;

        chain::controller &chain = appbase::app().get_plugin<chain_plugin>().chain();
        rsp_msg.firstNum = chain.first_block_num();
        ilog("first_block: ${f} head_block: ${h}", ("f", rsp_msg.firstNum)("h", chain.head_block_num()));
        if (!chain.fetch_block_by_number(rsp_msg.firstNum)) { // no block in db, only world status
            ilog("no block in db, only world status");
            rsp_msg.firstNum = 0;
            rsp_msg.lastNum = 0;
            return true;
        }

        rsp_msg.lastNum = chain.head_block_num();
        auto b = chain.fetch_block_by_number(rsp_msg.lastNum);
        if (b) {
            rsp_msg.blockHash = b->id();
            rsp_msg.prevBlockHash = b->previous;
        } else {
            rsp_msg.lastNum = INVALID_BLOCK_NUM;
        }

        Node::getInstance()->sendMessage(nodeId, rsp_msg);
        return true;
    }

    uint32_t Scheduler::getLastBlocknum() {
        const chain::controller &chain = appbase::app().get_plugin<chain_plugin>().chain();
        return chain.head_block_num();
    }

    bool Scheduler::handleMessage(const SyncBlockMsg &msg, bool safe) {
        const Block& block = msg.block;
        uint32_t last_num = getLastBlocknum();
        dlog("@@@@@@@@@@@@@@@ last block num in local chain:${last}", ("last", last_num));
        chain::controller &chain = appbase::app().get_plugin<chain_plugin>().chain();
        auto b = chain.fetch_block_by_number(last_num);
        if (b) {
            if (block.previous == b->id()) {
                // TODO(yufengshen) -- Do not copy here, should have shared_ptr at the first place.
                if (safe) {
                    produceBlock(std::make_shared<chain::signed_block>(block), true);
                } else {
                    bool result = (!msg.proof.empty() && setBlsVoterSet(msg.proof));
                    if (!result) {
                        elog("Bls voter set is wrong, so we can't save the received block. bls proof: ${p}", ("p", msg.proof));
                        return false;
                    }
                    produceBlock(std::make_shared<chain::signed_block>(block), true);
                }
                dlog("sync block finish blockNum = ${block_num}, hash = ${hash}, head_hash = ${head_hash}",
                     ("block_num", getLastBlocknum())("hash", short_hash(block.id()))("head_hash", short_hash(block.previous)));
                return true;
            } else {
                elog("block error. block in local chain: blockNum = ${localNum} hash = ${localHash} comming block num:${n} hash:${h} previous hash:${ph}",
                        ("localNum", last_num)("localHash", b->id())("n", block.block_num())("h", block.id())("ph", block.previous));
                return false;
            }
        }

        elog("Error!!! Get last block failed!!! num:${n}", ("n", last_num));
        return false;
    }

    bool Scheduler::handleMessage(const fc::sha256& nodeId, const SyncStopMsg &msg) {
        ilog("Stop sync msg to ${node}, seqNum: ${sn}", ("node", nodeId)("sn", msg.seqNum));
        if (m_syncTaskQueue.empty()) {
            return true;
        }

        for (std::list<SyncTask>::iterator it = m_syncTaskQueue.begin(); it != m_syncTaskQueue.end(); ++it) {
            if (it->nodeId == nodeId) {
                if (it->seqNum != msg.seqNum) {
                    elog("seqNum in task:${snt} != seqNum in msg:${snm}, but we still stop sync msg.", ("snt", it->seqNum)("snm", msg.seqNum));
                }
                m_syncTaskQueue.erase(it);
                break;
            }
        }

        return true;
    }

    bool Scheduler::is2fEcho(const VoterSet& voterSet, uint32_t phaseCount) const {
        std::shared_ptr<StakeVoteBase> stakeVotePtr = MsgMgr::getInstance()->getStakeVote(Node::getInstance()->getBlockNum());
        ULTRAIN_ASSERT(stakeVotePtr, chain::chain_exception, "stakeVotePtr is null");
        int totalVoterWeight = voterSet.getTotalVoterWeight();

        if (phaseCount < Config::kMaxBaxCount) {
            return totalVoterWeight >= stakeVotePtr->getNextRoundThreshold();
        } else if (phaseCount >= Config::kMaxBaxCount && phaseCount < Config::kDeadlineCnt) {
            return totalVoterWeight >= stakeVotePtr->getEmptyBlockThreshold();
        } else {
            return totalVoterWeight >= stakeVotePtr->getEmptyBlock2Threshold();
        }
    }

    bool Scheduler::isMinPropose(const ProposeMsg &proposeMsg) {
        std::shared_ptr<StakeVoteBase> stakeVotePtr = MsgMgr::getInstance()->getStakeVote(proposeMsg.block.block_num());
        ULTRAIN_ASSERT(stakeVotePtr, chain::chain_exception, "stakeVotePtr is null");
        uint32_t priority = stakeVotePtr->proposerPriority(proposeMsg.block.proposer, kPhaseBA0, 0);
        for (auto itor = m_proposerMsgMap.begin(); itor != m_proposerMsgMap.end(); ++itor) {
            if (stakeVotePtr->proposerPriority(itor->second.block.proposer, kPhaseBA0, 0) < priority) {
                return false;
            }
        }
        return true;
    }

    bool Scheduler::isMinFEcho(const VoterSet& voterSet, const BlockIdVoterSetMap& blockIdVoterSetMap) const {
        std::shared_ptr<StakeVoteBase> stakeVotePtr
                = MsgMgr::getInstance()->getStakeVote(BlockHeader::num_from_id(voterSet.commonEchoMsg.blockId));
        ULTRAIN_ASSERT(stakeVotePtr, chain::chain_exception, "stakeVotePtr is null");
        uint32_t priority = stakeVotePtr->proposerPriority(voterSet.commonEchoMsg.proposer, kPhaseBA0, 0);
        for (auto itor = blockIdVoterSetMap.begin(); itor != blockIdVoterSetMap.end(); ++itor) {
            if (itor->second.getTotalVoterWeight() >= stakeVotePtr->getSendEchoThreshold()) {
                if (stakeVotePtr->proposerPriority(itor->second.commonEchoMsg.proposer, kPhaseBA0, 0) < priority) {
                    return false;
                }
            }
        }
        return true;
    }

    bool Scheduler::isMinFEcho(const VoterSet& voterSet) const {
        return isMinFEcho(voterSet, m_echoMsgMap);
    }

    bool Scheduler::isMinEcho(const VoterSet& voterSet, const BlockIdVoterSetMap& blockIdVoterSetMap) const {
        std::shared_ptr<StakeVoteBase> stakeVotePtr = MsgMgr::getInstance()->getStakeVote(BlockHeader::num_from_id(voterSet.commonEchoMsg.blockId));
        ULTRAIN_ASSERT(stakeVotePtr, chain::chain_exception, "stakeVotePtr is null");
        uint32_t priority = stakeVotePtr->proposerPriority(voterSet.commonEchoMsg.proposer, kPhaseBA0, 0);
        for (auto itor = blockIdVoterSetMap.begin(); itor != blockIdVoterSetMap.end(); ++itor) {
            if (stakeVotePtr->proposerPriority(itor->second.commonEchoMsg.proposer, kPhaseBA0, 0) < priority) {
                return false;
            }
        }
        return true;
    }

    bool Scheduler::isMinEcho(const VoterSet& voterSet) const {
        return isMinEcho(voterSet, m_echoMsgMap);
    }

    size_t Scheduler::runScheduledTrxs(std::vector<chain::transaction_id_type> &trxs,
                                              fc::time_point hard_cpu_deadline,
                                              fc::time_point block_time) {
        chain::controller &chain = app().get_plugin<chain_plugin>().chain();
        const auto& cfg = chain.get_global_properties().configuration;
        const auto& max_trx_cpu = cfg.max_transaction_cpu_usage;
        ilog("-------runScheduledTrxs start running scheduled ${num} trxs", ("num", trxs.size()));
        size_t count = 0;
        for (const auto &trx : trxs) {
            try {
                auto deadline = fc::time_point::now() + fc::milliseconds(max_trx_cpu);
                chain::transaction_trace_ptr trace;
                try {
                    trace = chain.push_scheduled_transaction(trx, deadline);
                } catch (const chain::unknown_transaction_exception& e) {
                    ilog("-----------runScheduledTrxs unknown trx ${e}; skip");
                    continue;
                }
                if (trace->except) {
                    ilog("-----------runScheduledTrxs scheduled push trx failed ${e}", ("e", (trace->except)->what()));
                    auto code = trace->except->code();
                    // If the fail is due to block cpu/net, we break and we don't erase
                    // the trx since it will run through in the next block
                    if (code == chain::block_net_usage_exceeded::code_value) {
                        ilog("-----runScheduledTrxs code exec exceeds the max allowed net, break");
                        break;
                    } else if (code == chain::block_cpu_usage_exceeded::code_value) {
                        ilog("-----runScheduledTrxs code exec exceeds the max allowed time, break");
                        break;
                    }
                } else {
                    count++;
                    m_initTrxCount++;
                    if (fc::time_point::now() > hard_cpu_deadline) {
                        ilog("-----runScheduledTrxs code exec exceeds the hard cpu deadline, break");
                        break;
                    }
                    if (m_initTrxCount >= chain::config::default_max_propose_trx_count) {
                        break;
                    }
                }
            } FC_CAPTURE_AND_RETHROW();
        }
        return count;
    }

    size_t Scheduler::runUnappliedTrxs(std::vector<chain::transaction_metadata_ptr> &trxs,
                                              fc::time_point hard_cpu_deadline,
                                              fc::time_point block_time) {
        chain::controller &chain = app().get_plugin<chain_plugin>().chain();
        const auto& cfg = chain.get_global_properties().configuration;
        const auto& max_trx_cpu = cfg.max_transaction_cpu_usage;
        ilog("------- start running unapplied ${num} trxs", ("num", trxs.size()));
        size_t count = 0;
        for (auto &trx : trxs) {
            if (!trx) {
                chain.drop_unapplied_transaction(trx);
                ilog("-----------initProposeMsg null trx");
                // nulled in the loop above, skip it
                continue;
            }

            if (chain.is_known_unexpired_transaction(trx->id)) {
                chain.drop_unapplied_transaction(trx);
                ilog("-----------expired trx");
                continue;
            }

            if (m_blacklistTrx.find(trx->signed_id) != m_blacklistTrx.end()) {
                chain.drop_unapplied_transaction(trx);
                ilog("-----------blacklisted unapplied trx");
                continue;
            }

            if (fc::time_point(trx->packed_trx.expiration()) < block_time) {
                //                ilog("-----------initProposeMsg expired trx exp ${exp}, blocktime ${bt}",
                //                     ("exp",trx->packed_trx.expiration())("bt",block_time));
                chain.drop_unapplied_transaction(trx);
                continue;
            }

            try {
                auto deadline = fc::time_point::now() + fc::milliseconds(max_trx_cpu);
                auto trace = chain.push_transaction(trx, deadline);
                if (trace->except) {
                    ilog("-----------initProposeMsg push trx failed ${e}", ("e", (trace->except)->what()));
                    auto code = trace->except->code();
                    // If the fail is due to block cpu/net, we break and we don't erase
                    // the trx since it will run through in the next block
                    if (code == chain::block_net_usage_exceeded::code_value) {
                        ilog("----- code exec exceeds the max allowed net, break");
                        break;
                    } else if (code == chain::block_cpu_usage_exceeded::code_value) {
                        ilog("----- code exec exceeds the max allowed time, break");
                        break;
                    } else {
                        // for othe kind of failure, we should erase the trx
                        chain.drop_unapplied_transaction(trx);
                    }
                }

                m_initTrxCount++;
                count++;
                if (fc::time_point::now() > hard_cpu_deadline) {
                    ilog("----- code exec exceeds the hard cpu deadline, break");
                    break;
                }
                if (m_initTrxCount >= chain::config::default_max_propose_trx_count) {
                    break;
                }
            } FC_CAPTURE_AND_RETHROW();
        }
        return count;
    }

    size_t Scheduler::runPendingTrxs(std::list<chain::transaction_metadata_ptr> *trxs,
                                            fc::time_point hard_cpu_deadline,
                                            fc::time_point block_time) {
        chain::controller &chain = app().get_plugin<chain_plugin>().chain();
        const auto& cfg = chain.get_global_properties().configuration;
        const auto& max_trx_cpu = cfg.max_transaction_cpu_usage;
        ilog("------- start running pending ${num} trxs", ("num", trxs->size()));
        // TODO(yufengshen) : also scheduled trxs.
        size_t count = 0;
        while (!trxs->empty()) {
            auto &trx = trxs->front();
            if (!trx) {
                chain.drop_unapplied_transaction(trx);
                trxs->pop_front();
                ilog("-----------initProposeMsg null trx");
                // nulled in the loop above, skip it
                continue;
            }

            if (chain.is_known_unexpired_transaction(trx->id)) {
                //ilog("-------run pending duplicate trx");
                chain.drop_unapplied_transaction(trx);
                chain.drop_pending_transaction_from_set(trx);
                trxs->pop_front();
                continue;
            }

            if (m_blacklistTrx.find(trx->signed_id) != m_blacklistTrx.end()) {
                chain.drop_unapplied_transaction(trx);
                chain.drop_pending_transaction_from_set(trx);
                trxs->pop_front();
                ilog("-----------blacklisted pending trx");
                continue;
            }

            if (fc::time_point(trx->trx.expiration) < block_time) {
                //ilog("-------run pending expired trx");
                chain.drop_unapplied_transaction(trx);
                chain.drop_pending_transaction_from_set(trx);
                trxs->pop_front();
                continue;
            }

            try {
                auto deadline = fc::time_point::now() + fc::milliseconds(max_trx_cpu);
                auto trace = chain.push_transaction(trx, deadline);
                if (trace->except) {
                    ilog("-----------initProposeMsg push trx failed ${e}", ("e", (trace->except)->what()));
                    auto code = trace->except->code();
                    // If the fail is due to block cpu/net, we break and we don't erase
                    // the trx since it will run through in the next block
                    if (code == chain::block_net_usage_exceeded::code_value) {
                        ilog("----- code exec exceeds the max allowed net, break");
                        break;
                    } else if (code == chain::block_cpu_usage_exceeded::code_value) {
                        ilog("----- code exec exceeds the max allowed time, break");
                        break;
                    } else {
                        // for othe kind of failure, we should erase the trx
                        chain.drop_unapplied_transaction(trx);
                    }
                }
                // Pop the trx after the above exception handling.
                chain.drop_pending_transaction_from_set(trx);
                trxs->pop_front();

                m_initTrxCount++;
                count++;
                if (fc::time_point::now() > hard_cpu_deadline) {
                    ilog("----- code exec exceeds the hard cpu deadline, break");
                    break;
                }
                if (m_initTrxCount >= chain::config::default_max_propose_trx_count) {
                    break;
                }
            } FC_CAPTURE_AND_RETHROW();
        }
        return count;
    }

    bool Scheduler::initProposeMsg(ProposeMsg& proposeMsg) {
        auto& block = proposeMsg.block;
        auto start_timestamp = fc::time_point::now();
        chain::controller &chain = app().get_plugin<chain_plugin>().chain();
        size_t count2 = 0, count3 = 0;
  //      uint32_t trx_run_time = 3'000'000 * (Config::s_maxPhaseSeconds/5 +0.1*(Config::s_maxPhaseSeconds%5));
        try {
            SHA256 committeeMroot = getCommitteeMroot(chain.head_block_num() + 1);
            if (!chain.pending_block_state()) {
                auto block_timestamp = chain.get_proper_next_block_timestamp();
                ilog("initProposeMsg: start block at ${time} and block_timestamp is ${timestamp}",
                     ("time", fc::time_point::now())("timestamp", block_timestamp));
                chain.start_block(block_timestamp, committeeMroot, "");
            }

            // CheckPoint
            if (EpochEndPoint::isEpochEndPoint(chain.head_block_header())) { // CheckPoint iff the before one is EpochEndHeader
                std::shared_ptr<StakeVoteBase> stakeVotePtr = MsgMgr::getInstance()->getStakeVote(chain.head_block_num() + 1);
                ULTRAIN_ASSERT(stakeVotePtr, chain::chain_exception, "stakeVotePtr is null");
                CommitteeSet committeeSet = stakeVotePtr->getCommitteeSet();
                m_lightClientProducer->handleCheckPoint(chain, committeeSet);
            }

            // ConfirmPoint
            BlsVoterSet blsVoterSet;
            if (m_lightClientProducer->hasNextTobeConfirmedBls(blsVoterSet) && blsVoterSet.valid()) {
                m_lightClientProducer->handleConfirmPoint(chain, blsVoterSet);
            }

            // TODO(yufengshen): We have to cap the block size, cpu/net resource when packing a block.
            // Refer to the subjective and exhausted design.
            std::list<chain::transaction_metadata_ptr> *pending_trxs = chain.get_pending_transactions();
            auto unapplied_trxs = chain.get_unapplied_transactions();

            m_initTrxCount = 0;
            auto block_time = chain.pending_block_state()->header.timestamp.to_time_point();
            // There is case where cpu block limits can't handle, which is when there are huge number
            // of pending trxs that are all from the same user but the user has used up his cpu resources
            // and keep failing the trx execution; so we still need the hard cpu deadline to handle this.
            fc::time_point hard_cpu_deadline =
                fc::time_point::now() + fc::microseconds(Config::s_maxTrxMicroSeconds);/*can change in conig file*/
            size_t count1 = runPendingTrxs(pending_trxs, hard_cpu_deadline, block_time);

            if (fc::time_point::now() < hard_cpu_deadline &&
                m_initTrxCount < chain::config::default_max_propose_trx_count) {
                count2 = runUnappliedTrxs(unapplied_trxs, hard_cpu_deadline, block_time);
            }

            if (fc::time_point::now() < hard_cpu_deadline &&
                m_initTrxCount < chain::config::default_max_propose_trx_count) {
                auto scheduled_trxs = chain.get_scheduled_transactions();
                unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
                std::default_random_engine e(seed);
                std::shuffle(scheduled_trxs.begin(), scheduled_trxs.end(), e);
                count3 = runScheduledTrxs(scheduled_trxs, hard_cpu_deadline, block_time);
            }

            chain.finish_block_hack();
            ilog("------- run ${count1} ${count2}  ScheduledTrxs:${count3} trxs, taking time ${time}, remaining pending trx ${count4}, remaining unapplied trx ${count5}",
                 ("count1", count1)
                 ("count2", count2)
                 ("count3", count3)
                 ("time", fc::time_point::now() - start_timestamp)
                 ("count4", pending_trxs->size())
                 ("count5", unapplied_trxs.size() - count2));
            // TODO(yufengshen) - Do we finalize here ?
            // If we finalize here, we insert the block summary into the database.
            //chain.finalize_block();
            // TODO(yufengshen) - Do we need to include the merkle in the block propose?
            chain.set_action_merkle_hack();
            chain.set_trx_merkle_hack();
            // EpochEndPoint iff CommitteeSet changed
            std::shared_ptr<CommitteeState> committeeState = StakeVoteBase::getCommitteeState(chain::self_chain_name);
            if (committeeState && committeeState->chainStateNormal && committeeState->cinfo.size() > 0) {
                CommitteeSet committeeSet(committeeState->cinfo);
                SHA256 newMRoot = committeeSet.committeeMroot();
                if (newMRoot != committeeMroot) {
                    m_lightClientProducer->handleEpochEndPoint(chain, newMRoot);
                }
            }
            chain.set_version(0);
            chain.set_proposer(StakeVoteBase::getMyAccount());
            // Construct the block msg from pbs.
            const auto &pbs = chain.pending_block_state();
            FC_ASSERT(pbs, "pending_block_state does not exist but it should, another plugin may have corrupted it");
            const auto &bh = pbs->header;
            block.timestamp = bh.timestamp;
            block.proposer = bh.proposer;
            block.version = bh.version;
            block.previous = bh.previous;
            block.transaction_mroot = bh.transaction_mroot;
            block.action_mroot = bh.action_mroot;
            block.transactions = pbs->block->transactions;
            block.committee_mroot = bh.committee_mroot;
            block.header_extensions = bh.header_extensions;
            block.signature = std::string(Signer::sign<BlockHeader>(block, StakeVoteBase::getMyPrivateKey()));
            ilog("-------- propose a block, trx num ${num} proposer ${proposer} block signature ${signature} committee mroot ${mroot}",
                 ("num", block.transactions.size())
                 ("proposer", std::string(block.proposer))
                 ("signature", short_sig(block.signature))
                 ("mroot", short_hash(block.committee_mroot))
                 );
        } catch (const chain::guard_exception& e ) {
            edump((e.to_detail_string()));
            chain.abort_block(true);
            throw;
        } catch (const fc::exception &e) {
            edump((e.to_detail_string()));
            chain.abort_block(true);
            throw;
        }
        return true;
    }

    void Scheduler::processCache(const RoundInfo& info) {
        auto proposeItor = m_cacheProposeMsgMap.find(info);
        if (proposeItor != m_cacheProposeMsgMap.end()) {
            dlog("cache propose msg size = ${size}. blockNum = ${num}, phase = ${phase}",
                 ("size", proposeItor->second.size())
                         ("num", info.blockNum)("phase", info.phase));
            for (auto &propose : proposeItor->second) {
                handleMessage(propose);
            }
            m_cacheProposeMsgMap.erase(proposeItor);
        }

        auto echoItor = m_cacheEchoMsgMap.find(info);
        if (echoItor != m_cacheEchoMsgMap.end()) {
            dlog("cache echo msg num = ${num}. blockNum = ${id}, phase = ${phase}", ("num", echoItor->second.size())
                    ("id", info.blockNum)("phase", info.phase));
            for (auto &echo : echoItor->second) {
                handleMessage(echo);
            }
            m_cacheEchoMsgMap.erase(echoItor);
        }
    }

    void Scheduler::fastProcessCache(const RoundInfo& info) {
        resetTimestamp();

        auto proposeItor = m_cacheProposeMsgMap.find(info);
        if (proposeItor != m_cacheProposeMsgMap.end()) {
            dlog("fastProcessCache. cache propose msg size = ${size}. blockNum = ${num}, phase = ${phase}",
                 ("size", proposeItor->second.size())
                         ("num", info.blockNum)("phase", info.phase));
            for (auto &propose : proposeItor->second) {
                fastHandleMessage(propose);
            }
            m_cacheProposeMsgMap.erase(proposeItor);
        }

        auto echoItor = m_cacheEchoMsgMap.find(info);
        if (echoItor != m_cacheEchoMsgMap.end()) {
            dlog("fastProcessCache. cache echo msg size = ${size}. blockNum = ${num}, phase = ${phase}", ("size", echoItor->second.size())
                    ("num", info.blockNum)("phase", info.phase));
            for (auto &echo : echoItor->second) {
                fastHandleMessage(echo);
            }
            m_cacheEchoMsgMap.erase(echoItor);
        }
    }

    bool Scheduler::findEchoCache(const RoundInfo& info) {
        auto echoItor = m_cacheEchoMsgMap.find(info);
        if (echoItor != m_cacheEchoMsgMap.end()) {
            return true;
        }
        return false;
    }

    bool Scheduler::isFastba0(const RoundInfo& info) {
        std::shared_ptr<StakeVoteBase> stakeVotePtr
                = MsgMgr::getInstance()->getStakeVote(Node::getInstance()->getBlockNum());
        ULTRAIN_ASSERT(stakeVotePtr, chain::chain_exception, "stakeVotePtr is null");
        auto echoItor = m_cacheEchoMsgMap.find(info);
        if (echoItor != m_cacheEchoMsgMap.end()) {
            if (echoItor->second.size() > stakeVotePtr->getSendEchoThreshold()) {
                return true;
            }
        }

        return false;
    }

    bool Scheduler::findProposeCache(const RoundInfo& info) {
        auto proposeItor = m_cacheProposeMsgMap.find(info);
        if (proposeItor != m_cacheProposeMsgMap.end()) {
            return true;
        }
        return false;
    }

    Block Scheduler::produceBaxBlock() {
        VoterSet voterSet;
        std::shared_ptr<StakeVoteBase> stakeVotePtr
                = MsgMgr::getInstance()->getStakeVote(Node::getInstance()->getBlockNum());
        ULTRAIN_ASSERT(stakeVotePtr, chain::chain_exception, "stakeVotePtr is null");
        uint32_t minPriority = stakeVotePtr->getProposerNumber();
        dlog("begin.");
        for (auto mapItor = m_echoMsgAllPhase.begin(); mapItor != m_echoMsgAllPhase.end(); ++mapItor) {
            if ((Node::getInstance()->getPhase() + Node::getInstance()->getBaxCount()) >= Config::kMaxBaxCount) {
                if (mapItor->first.phase < Config::kMaxBaxCount) {
                    continue;
                }
            }

            for (auto itor : mapItor->second) {
                if (is2fEcho(itor.second, mapItor->first.phase)) {
                    dlog("found >= 2f + 1 echo. blocknum = ${blocknum} phase = ${phase}",
                         ("blocknum",mapItor->first.blockNum)("phase",mapItor->first.phase));
                    uint32_t priority = stakeVotePtr->proposerPriority(itor.second.commonEchoMsg.proposer, kPhaseBA0, 0);
                    if (minPriority >= priority) {
                        dlog("min proof change.");
                        voterSet = itor.second;
                        minPriority = priority;
                    }
                }
            }

            if (voterSet.empty() || isBlank(voterSet.commonEchoMsg.blockId)) {
                continue;
            }

            if (isEmpty(voterSet.commonEchoMsg.blockId)) {
                dlog("produce empty Block. save VoterSet in bax blockId = ${blockId}", ("blockId", short_hash(voterSet.commonEchoMsg.blockId)));
                m_currentBlsVoterSet = toBlsVoterSetAndFindEvil(voterSet, stakeVotePtr->getCommitteeSet(),
                        stakeVotePtr->isGenesisPeriod(), stakeVotePtr->getNextRoundThreshold() + 1);
                m_evilDDosDetector.deduceBlockNum(voterSet, stakeVotePtr->getSendEchoThreshold() + 1,
                        Node::getInstance()->getRoundCount(), Node::getInstance()->getPhase());
                return emptyBlock();
            }
            auto proposeItor = m_proposerMsgMap.find(voterSet.commonEchoMsg.blockId);
            if (proposeItor != m_proposerMsgMap.end()
                    && stakeVotePtr->proposerPriority(proposeItor->second.block.proposer, kPhaseBA0, 0) == minPriority) {
                dlog("find propose msg ok. blocknum = ${blocknum} phase = ${phase} save VoterSet in bax blockId = ${blockId}",
                     ("blocknum", mapItor->first.blockNum)("phase", mapItor->first.phase)("blockId", short_hash(voterSet.commonEchoMsg.blockId)));
                m_currentBlsVoterSet = toBlsVoterSetAndFindEvil(voterSet, stakeVotePtr->getCommitteeSet(),
                        stakeVotePtr->isGenesisPeriod(), stakeVotePtr->getNextRoundThreshold() + 1);
                m_evilDDosDetector.deduceBlockNum(voterSet, stakeVotePtr->getSendEchoThreshold() + 1,
                        Node::getInstance()->getRoundCount(), Node::getInstance()->getPhase());
                return proposeItor->second.block;
            }
            dlog("can not find 2f + 1 echo's propose. hash = ${hash}",("hash", short_hash(voterSet.commonEchoMsg.blockId)));
        }

        return Block();
    }

    void Scheduler::reportEmptyBlockReason(const BlockIdType& blockId, bool syncing) {
        if (blockId == emptyBlock().id() && syncing == false) {
            EmptyBlockReason reason;
            reason.blockNum = Node::getInstance()->getBlockNum();
            if (m_currentBlsVoterSet.valid() && m_currentBlsVoterSet.commonEchoMsg.blockId == blockId) {
                reason.phase = m_currentBlsVoterSet.commonEchoMsg.phase;
                reason.baxCount = m_currentBlsVoterSet.commonEchoMsg.phase;
            }
            reason.currentPhase = Node::getInstance()->getPhase();
            reason.currentBaxCount = Node::getInstance()->getBaxCount();
            reason.proposeCount = m_proposerMsgMap.size();
            reason.isBa0Empty = m_ba0Block.id() == emptyBlock().id();
            NativeTrx::reportEmptyBlockReason(std::string(appbase::app().get_plugin<chain_plugin>().get_chain_name()), reason.blockNum, reason);
        }
    }

    /**
     *
     * @return
     * empty block or normal block when ba0 while other phase may return blank, empty, normal block.
     */
    Block Scheduler::produceTentativeBlock() {
        BlockIdType minBlockId = BlockIdType();
        uint32_t phase = Node::getInstance()->getPhase();
        phase += Node::getInstance()->getBaxCount();

        std::shared_ptr<StakeVoteBase> stakeVotePtr = MsgMgr::getInstance()->getStakeVote(Node::getInstance()->getBlockNum());
        ULTRAIN_ASSERT(stakeVotePtr, chain::chain_exception, "stakeVotePtr is null");
        uint32_t minPriority = stakeVotePtr->getProposerNumber();
        for (auto itor = m_echoMsgMap.begin(); itor != m_echoMsgMap.end(); itor++) {
            dlog("finish display_echo. phase = ${phase} size = ${size} totalVoter = ${totalVoter} block_hash : ${block_hash}",
                 ("phase", (uint32_t) itor->second.commonEchoMsg.phase)("size", itor->second.accountPool.size())(
                     "totalVoter", itor->second.getTotalVoterWeight())("block_hash", short_hash(itor->second.commonEchoMsg.blockId)));
            if (is2fEcho(itor->second, phase)) {
                uint32_t priority = stakeVotePtr->proposerPriority(itor->second.commonEchoMsg.proposer, kPhaseBA0, 0);
                if (minPriority >= priority) {
                    minBlockId = itor->second.commonEchoMsg.blockId;
                    minPriority = priority;
                }
            }
        }

        if (minBlockId == BlockIdType()) { // not found > 2f + 1 echo
            dlog("can not find >= 2f + 1 = ${num}", ("num", stakeVotePtr->getNextRoundThreshold()));
            if (Node::getInstance()->getPhase() == kPhaseBA0) {
                return emptyBlock();
            }
            if ((!m_echoMsgAllPhase.empty()) && (Node::getInstance()->getPhase() == kPhaseBAX)) {
                return produceBaxBlock();
            }
            return blankBlock();
        }
        for (auto itor = m_echoMsgMap.begin(); itor != m_echoMsgMap.end(); itor++) {
            uint32_t priority = stakeVotePtr->proposerPriority(itor->second.commonEchoMsg.proposer, kPhaseBA0, 0);
            if (minPriority == priority && phase >= kPhaseBA1) { // check priority only
                ilog("save VoterSet blockId = ${blockId}", ("blockId", short_hash(itor->second.commonEchoMsg.blockId)));
                // save VoterSet
                m_currentBlsVoterSet = toBlsVoterSetAndFindEvil(itor->second, stakeVotePtr->getCommitteeSet(),
                        stakeVotePtr->isGenesisPeriod(), stakeVotePtr->getNextRoundThreshold() + 1);
                m_evilDDosDetector.deduceBlockNum(itor->second, stakeVotePtr->getSendEchoThreshold() + 1,
                        Node::getInstance()->getRoundCount(), Node::getInstance()->getPhase());
                break;
            }
        }
        if (minBlockId == emptyBlock().id()) {
            dlog("produce empty Block");
            return emptyBlock();
        }
        auto proposeItor = m_proposerMsgMap.find(minBlockId);
        if (proposeItor != m_proposerMsgMap.end()) {
            return proposeItor->second.block;
        }
        dlog("> 2f + 1 echo ${hash} can not find it's propose.", ("hash", short_hash(minBlockId)));
        if (kPhaseBA0 == Node::getInstance()->getPhase()) {
            dlog("produce empty Block");
            return emptyBlock();
        }
        return blankBlock();
    }

    void Scheduler::clearPreRunStatus() {
        m_voterPreRunBa0InProgress = false;
        m_currentPreRunBa0TrxIndex = -1;
        chain::controller &chain = appbase::app().get_plugin<chain_plugin>().chain();
        chain.abort_block();
    }

    bool Scheduler::verifyBa0Block() {
        chain::controller &chain = appbase::app().get_plugin<chain_plugin>().chain();
        const auto& cfg = chain.get_global_properties().configuration;
        const chain::signed_block &block = m_ba0Block;
        auto id = block.id();

        if (isBlank(id)) {
            return false;
        }

        auto existing = chain.fetch_block_by_id(id);
        if (existing) {
            ULTRAIN_ASSERT(!existing, chain::chain_exception, "Produced block is already in the chain");
            return false;
        }

        //not to run trxs multiple times in same block
        if (m_ba0VerifiedBlkId == id) {
            m_voterPreRunBa0InProgress = true;
            ilog("Block ${blk} has been verified before", ("blk", id));
            return true;
        }

        if (m_ba0FailedBlkId == id) {
            ilog("Block ${blk} has been failed before", ("blk", id));
            return false;
        }

        if (block.timestamp.to_time_point() > (fc::time_point::now() + fc::hours(2))) {
            ilog("Block ${blk} timestamp ${time} is too far into the future",
                 ("blk", id)("time", string(block.timestamp.to_time_point())));
            m_ba0FailedBlkId = id;
            return false;
        }

        chain.abort_block();

        ilog("---- Scheduler::verifyBa0Block with trx number ${count}",
             ("count", block.transactions.size()));
        // Here is the hack, we are actually using the template of ba0_block, but we don't use
        // chain's push_block, so we have to copy some members of ba0_block into the head state,
        // e.g. pk, proof, producer.
        chain.start_block(block.timestamp, getCommitteeMroot(chain.head_block_num() + 1), block.signature);
        chain::block_state_ptr pbs = chain.pending_block_state_hack();
        chain::signed_block_ptr bp = pbs->block;
        chain::signed_block_header *hp = &(pbs->header);
        // TODO(yufengshen): Move all this into start_block() to remove dup codes.
        bp->proposer = block.proposer;
        hp->proposer = block.proposer;
        bp->header_extensions = block.header_extensions;
        hp->header_extensions = block.header_extensions;
        auto start_timestamp = fc::time_point::now();
        try {
            for (int i = 0; i < block.transactions.size(); i++) {
                const auto &receipt = block.transactions[i];
                chain::transaction_trace_ptr trace;
                // This passed in deadline is used to guard for non-stopping while loop.
                auto max_cpu_usage = fc::microseconds(cfg.max_transaction_cpu_usage);
                auto max_deadline = fc::time_point::now() + max_cpu_usage;
                bool scheduled = false;
                if (receipt.trx.contains<chain::packed_transaction>()) {
                    // Malicious producer setting wrong cpu_usage_us.
                    ULTRAIN_ASSERT(receipt.cpu_usage_us >= cfg.min_transaction_cpu_usage,
                                   chain::block_trx_min_cpu_usage_exception,
                                   "trx in proposed block has wrong cpu_usage_us set ${n}",
                                   ("n", receipt.cpu_usage_us));
                    auto &pt = receipt.trx.get<chain::packed_transaction>();
                    auto mtrx = std::make_shared<chain::transaction_metadata>(pt);
                    trace = chain.push_transaction(mtrx, max_deadline, receipt.cpu_usage_us);
                } else if (receipt.trx.contains<chain::transaction_id_type>()) {
                    trace = chain.push_scheduled_transaction(receipt.trx.get<chain::transaction_id_type>(),
                                                             max_deadline, receipt.cpu_usage_us);
                    scheduled = true;
                } else if(receipt.trx.contains<chain::packed_generated_transaction>()) {
                    trace = chain.push_generated_transaction(receipt.trx.get<chain::packed_generated_transaction>(),
                                                             max_deadline, receipt.cpu_usage_us);
                    scheduled = true;
                }

                bool transaction_failed =  trace && trace->except;
                bool transaction_can_fail = (receipt.status == chain::transaction_receipt_header::hard_fail && scheduled);
                if (transaction_failed && !transaction_can_fail) {
                    // If failing a trx in a proposed block, this trx is very suspicious;
                    // Put this trx into a blacklist map, and discard it when proposing block.
                    if (receipt.trx.contains<chain::packed_transaction>()) {
                        auto &pt = receipt.trx.get<chain::packed_transaction>();
                        auto mtrx = std::make_shared<chain::transaction_metadata>(pt);
                        m_blacklistTrx[mtrx->signed_id] = chain.head_block_num();
                    }
                    // So we can terminate early
                    throw *trace->except;
                }
                ULTRAIN_ASSERT((fc::time_point::now() - start_timestamp) < fc::seconds(5),
                               chain::transaction_exception,
                               "voter code exec exceeds the max allowed time");
            }
            chain.finish_block_hack();
            chain.set_action_merkle_hack();
            chain.set_trx_merkle_hack();
            chain.set_header_extensions(m_ba0Block.header_extensions);
            ULTRAIN_ASSERT(pbs->header.action_mroot == block.action_mroot,
                           chain::chain_exception,
                           "Verify Ba0 block not generating expected action_mroot");
            ULTRAIN_ASSERT(pbs->header.transaction_mroot == block.transaction_mroot,
                           chain::chain_exception,
                           "Verify Ba0 block not generating expected transaction_mroot");
        } catch (const fc::exception &e) {
            edump((e.to_detail_string()));
            m_ba0FailedBlkId = id;
            chain.abort_block(true);
            return false;
        }
        // TODO(yufengshen): SHOULD CHECK the signature and block's validity.
        m_voterPreRunBa0InProgress = true;
        m_ba0VerifiedBlkId = id;
        return true;
    }

    bool Scheduler::preRunBa0BlockStart() {
        chain::controller &chain = appbase::app().get_plugin<chain_plugin>().chain();
        chain.abort_block();
        const chain::signed_block &block = m_ba0Block;
        if (isBlank(block.id()) || block.transactions.empty()) {
            return false;
        }

        auto id = block.id();
        auto existing = chain.fetch_block_by_id(id);
        if (existing) {
            ULTRAIN_ASSERT(!existing, chain::chain_exception, "Produced block is already in the chain");
            return false;
        }
        ilog("---- Scheduler::preRunBa0BlockStart() start block");
        // Here is the hack, we are actually using the template of ba0_block, but we don't use
        // chain's push_block, so we have to copy some members of ba0_block into the head state,
        // e.g. pk, proof, producer.
        try {
            chain.start_block(block.timestamp, getCommitteeMroot(chain.head_block_num() + 1), block.signature);
            chain::block_state_ptr pbs = chain.pending_block_state_hack();
            chain::signed_block_ptr bp = pbs->block;
            chain::signed_block_header *hp = &(pbs->header);
            bp->proposer = block.proposer;
            hp->proposer = block.proposer;
            bp->header_extensions = block.header_extensions;
            hp->header_extensions = block.header_extensions;
            m_currentPreRunBa0TrxIndex = 0;
        } catch (const fc::exception &e) {
            edump((e.to_detail_string()));
            chain.abort_block();
            m_currentPreRunBa0TrxIndex = -1;
            return false;
        }
        return true;
    }

    bool Scheduler::preRunBa0BlockStep() {
        chain::controller &chain = app().get_plugin<chain_plugin>().chain();
        const auto& cfg = chain.get_global_properties().configuration;
        const auto &pbs = chain.pending_block_state();
        if (!m_voterPreRunBa0InProgress) {
            return false;
        }

        if (!pbs) {
            ilog("------- NO PBS in preRunBa0BlockStep, abort");
            return false;
        }
        ULTRAIN_ASSERT(m_currentPreRunBa0TrxIndex >= 0, chain::chain_exception,
                       "m_currentPreRunBa0TrxIndex must be valid");
        const chain::signed_block &b = m_ba0Block;
        int trx_count = 0;
        ilog("------ Continue pre-running ba0 block from ${count}", ("count", m_currentPreRunBa0TrxIndex));
        try {
            for (; m_currentPreRunBa0TrxIndex < b.transactions.size() &&
                   trx_count <= (200 * Config::s_maxPhaseSeconds); m_currentPreRunBa0TrxIndex++, trx_count++) {
                const auto &receipt = b.transactions[m_currentPreRunBa0TrxIndex];
                chain::transaction_trace_ptr trace;
                bool scheduled = false;
                // Malicious producer setting wrong cpu_usage_us.
                ULTRAIN_ASSERT(receipt.cpu_usage_us >= cfg.min_transaction_cpu_usage,
                               chain::block_trx_min_cpu_usage_exception,
                               "trx in proposed block has wrong cpu_usage_us set ${n}",
                               ("n", receipt.cpu_usage_us));
                auto max_cpu_usage = fc::microseconds(cfg.max_transaction_cpu_usage);
                auto max_deadline = fc::time_point::now() + max_cpu_usage;
                if (receipt.trx.contains<chain::packed_transaction>()) {
                    auto &pt = receipt.trx.get<chain::packed_transaction>();
                    auto mtrx = std::make_shared<chain::transaction_metadata>(pt);
                    trace = chain.push_transaction(mtrx, max_deadline, receipt.cpu_usage_us);
                } else if (receipt.trx.contains<chain::transaction_id_type>()) {
                    trace = chain.push_scheduled_transaction(receipt.trx.get<chain::transaction_id_type>(),
                                                     max_deadline, receipt.cpu_usage_us);
                    scheduled = true;
                } else if(receipt.trx.contains<chain::packed_generated_transaction>()) {
                    trace = chain.push_generated_transaction(receipt.trx.get<chain::packed_generated_transaction>(),
                                                     max_deadline, receipt.cpu_usage_us);
                    scheduled = true;
                }

                bool transaction_failed =  trace && trace->except;
                bool transaction_can_fail = (receipt.status == chain::transaction_receipt_header::hard_fail && scheduled);
                if (transaction_failed && !transaction_can_fail) {
                    // So we can terminate early
                    throw *trace->except;
                }
            }
        } catch (const fc::exception &e) {
            edump((e.to_detail_string()));
            chain.abort_block(true);
            m_currentPreRunBa0TrxIndex = -1;
            return false;
        }

        return true;
    }

    void Scheduler::produceBlock(const chain::signed_block_ptr &block, bool force_push_whole_block) {
        chain::controller &chain = appbase::app().get_plugin<chain_plugin>().chain();
        uint32_t last_num = getLastBlocknum();

        dlog("produceBlock. last block num in local chain:${last}", ("last", last_num));
        auto blk = chain.fetch_block_by_number(last_num);
        if (blk) {
            if (block->previous != blk->id()) {
                ULTRAIN_ASSERT(false, chain::chain_exception, "DB error. please reset with cmd --delete-all-blocks.");
                return;
            }
        }

        auto id = block->id();
        auto existing = chain.fetch_block_by_id(id);
        if (existing) {
            ULTRAIN_ASSERT(!existing, chain::chain_exception, "Produced block is already in the chain");
            return;
        }

        const auto &pbs = chain.pending_block_state();
        bool needs_push_whole_block = true;

        if (Node::getInstance()->isSyncing()) {
            chain.disable_worldstate_creation();
        } else {
            chain.enable_worldstate_creation();
        }

        if (pbs && m_voterPreRunBa0InProgress && !force_push_whole_block) {
            ULTRAIN_ASSERT(m_currentPreRunBa0TrxIndex == -1,
                           chain::chain_exception,
                           "Voter wont' have ba0 pre-run");
            // first check if ba1 block is indeed ba0 block.
            if (theSameOne(m_ba0Block, block)) {
                ilog("------ Finish voter pre-running ba0 block");
                chain.finalize_block();
                chain.assign_header_to_block();
                chain.commit_block();
                needs_push_whole_block = false;
                // No need to check trx/action_mroot, it was already verified in verifyBa0Block();
            }
        }

        // We are already pre-running ba0_block
        if (pbs && m_currentPreRunBa0TrxIndex >= 0 && !force_push_whole_block) {
            if (theSameOne(m_ba0Block, block)) {
                ilog("------ Finish pre-running ba0 block from ${count}", ("count", m_currentPreRunBa0TrxIndex));
                try {
                    for (; m_currentPreRunBa0TrxIndex < m_ba0Block.transactions.size(); m_currentPreRunBa0TrxIndex++) {
                        const auto &receipt = m_ba0Block.transactions[m_currentPreRunBa0TrxIndex];
                        chain::transaction_trace_ptr trace;
                        bool scheduled = false;
                        if (receipt.trx.contains<chain::packed_transaction>()) {
                            auto &pt = receipt.trx.get<chain::packed_transaction>();
                            auto mtrx = std::make_shared<chain::transaction_metadata>(pt);
                            // Now set the deadline to infinity is fine since voter has already voted for this one.
                            trace = chain.push_transaction(mtrx, fc::time_point::maximum(), receipt.cpu_usage_us);
                        } else if (receipt.trx.contains<chain::transaction_id_type>()) {
                            trace = chain.push_scheduled_transaction(receipt.trx.get<chain::transaction_id_type>(),
                                                             fc::time_point::maximum(), receipt.cpu_usage_us);
                            scheduled = true;
                        } else if(receipt.trx.contains<chain::packed_generated_transaction>()) {
                            trace = chain.push_generated_transaction(receipt.trx.get<chain::packed_generated_transaction>(),
                                                              fc::time_point::maximum(), receipt.cpu_usage_us);
                            scheduled = true;
                        }

                        bool transaction_failed =  trace && trace->except;
                        bool transaction_can_fail = (receipt.status == chain::transaction_receipt_header::hard_fail && scheduled);
                        if (transaction_failed && !transaction_can_fail) {
                            // So we can terminate early
                            throw *trace->except;
                        }
                    }
                    chain.finish_block_hack();

                    chain.finalize_block();
                    ULTRAIN_ASSERT(pbs->header.action_mroot == block->action_mroot,
                                   chain::chain_exception,
                                   "Pre-run Ba0 block not generating expected action_mroot");
                    ULTRAIN_ASSERT(pbs->header.transaction_mroot == block->transaction_mroot,
                                   chain::chain_exception,
                                   "Pre-run Ba0 block not generating expected transaction_mroot");
                    chain.assign_header_to_block();
                    chain.commit_block();
                    needs_push_whole_block = false;
                } catch (const fc::exception &e) {
                    ilog("------ error in finish pre-running block ${s}", ("s", e.to_detail_string()));
                    edump((e.to_detail_string()));
                    // We should not get here, but if so, we need to re-push the whole block.
                    needs_push_whole_block = true;
                }
            }
        }

        // TODO(yufengshen) : if push_block fails, what to do ?
        if (needs_push_whole_block) {
            ilog("-------- Actually needs to push_whole_block");
            chain.abort_block();
            if (Node::getInstance()->getNonProducingNode()) {
                chain.set_emit_signal();
                chain.start_receive_event();
            }

            chain.push_block(block);

            if (Node::getInstance()->getNonProducingNode()) {
                chain.clear_emit_signal();
                chain.notify_event();
                chain.stop_receive_event();
            }
        }

        m_currentPreRunBa0TrxIndex = -1;
        m_voterPreRunBa0InProgress = false;

        clearTrxQueue();

        chain::block_state_ptr new_bs = chain.head_block_state();
        ilog("-----------produceBlock timestamp ${timestamp} block num ${num} id ${id} trx count ${count}",
             ("timestamp", block->timestamp)
             ("num", block->block_num())
             ("id", short_hash(block->id()))
             ("count", new_bs->block->transactions.size()));
        m_lightClientProducer->acceptNewHeader(chain.head_block_header(), m_currentBlsVoterSet);
        m_lightClientProducer->saveCurrentBlsVoterSet(m_currentBlsVoterSet);
        MsgMgr::getInstance()->moveToNewStep(Node::getInstance()->getBlockNum(), kPhaseBA0, 0);
    }

    void Scheduler::clearTrxQueue() {
        chain::controller &chain = appbase::app().get_plugin<chain_plugin>().chain();

        // TODO(yufengshen):
        // Non-producing node has no chance to run through unapplied trxs
        // so as to eliminate invalid/expired trxs and this could lead to
        // memleak, so lets just clear them here. This might drop some trxs.
        // So maybe better to find a timing to re-run these trx and send
        // valid and unexpired trx to other nodes.
        if (Node::getInstance()->getNonProducingNode()) {
            chain.clear_unapplied_transaction();
        }

        std::list<chain::transaction_metadata_ptr> *pending_trxs = chain.get_pending_transactions();
        auto block_time = chain.head_block_state()->header.timestamp.to_time_point();
        auto it = pending_trxs->begin();
        while (it != pending_trxs->end()) {
            if (chain.is_known_unexpired_transaction((*it)->id) ||
                fc::time_point((*it)->trx.expiration) < block_time ||
                m_blacklistTrx.find((*it)->signed_id) != m_blacklistTrx.end()) {
                chain.drop_unapplied_transaction(*it);
                chain.drop_pending_transaction_from_set(*it);
                it = pending_trxs->erase(it);
            } else {
                it++;
            }
        }
        // Clean up old malicious trx in m_blacklistTrx;
        auto it2 = m_blacklistTrx.begin();
        uint32_t head_num = chain.head_block_num();
        while (it2 != m_blacklistTrx.end()) {
            if (head_num - it2->second > 1024) {
                it2 = m_blacklistTrx.erase(it2);
            } else {
                it2++;
            }
        }
    }

    void Scheduler::init() {
        m_proposerMsgMap.clear();
        m_echoMsgMap.clear();
        m_cacheProposeMsgMap.clear();
        m_cacheEchoMsgMap.clear();
        m_echoMsgAllPhase.clear();
        startSyncTaskTimer();
    }

    const Block* Scheduler::getBa0Block() {
        return &m_ba0Block;
    }

    BlockIdType Scheduler::getPreviousBlockhash() const {
        const chain::controller &chain = appbase::app().get_plugin<chain_plugin>().chain();
        return chain.head_block_id();
    }

    void Scheduler::moveEchoMsg2AllPhaseMap() {
        if (m_echoMsgAllPhase.size() >= m_maxCachedAllPhaseKeys) {
            wlog("echo all phase msgs exceeds ${max} blockNum: ${b} phase: ${p} baxcount: ${bx}",
                 ("max", m_maxCachedAllPhaseKeys)
                 ("b", Node::getInstance()->getBlockNum())
                 ("p", (int)Node::getInstance()->getPhase())
                 ("bx", Node::getInstance()->getBaxCount()));
            return;
        }

        RoundInfo info(Node::getInstance()->getBlockNum(), Node::getInstance()->getPhase() + Node::getInstance()->getBaxCount());
        m_echoMsgAllPhase.insert(make_pair(info, std::move(m_echoMsgMap)));
    }

    void Scheduler::startSyncTaskTimer() {
        m_syncTaskTimer->expires_from_now(m_syncTaskPeriod);
        m_syncTaskTimer->async_wait([this](boost::system::error_code ec) {
            if (ec.value() == boost::asio::error::operation_aborted) {
                ilog("sync task timer be canceled.");
            } else {
                processSyncTask();
                startSyncTaskTimer();
            }
        });
    }

    void Scheduler::processSyncTask() {
        if (m_syncTaskQueue.empty()) {
            return;
        }

        SyncBlockMsg sync_block;
        chain::controller &chain = appbase::app().get_plugin<chain_plugin>().chain();
        uint32_t last_num = getLastBlocknum();
        uint32_t max_count = m_maxPacketsOnce / m_syncTaskQueue.size() + 1;
        uint32_t send_count = 0;
        for (std::list<SyncTask>::iterator it = m_syncTaskQueue.begin(); it != m_syncTaskQueue.end();) {
            sync_block.seqNum = it->seqNum;
            send_count = 0;
            while (send_count < max_count && it->startBlock <= it->endBlock && it->startBlock <= last_num) {
                auto b = chain.fetch_block_by_number(it->startBlock);
                if (b) {
                    sync_block.block = *b;
                    if (it->startBlock == it->checkBlock) {
                        sync_block.proof = it->bvs.toString();
                        ilog("send checked block in task: ${s}", ("s", sync_block.proof));
                    } else if (it->startBlock == last_num) {
                        sync_block.proof = m_currentBlsVoterSet.toString();
                        ilog("send last block in task: ${s}", ("s", sync_block.proof));
                    }
                    Node::getInstance()->sendMessage(it->nodeId, sync_block);
                } else if (it->startBlock == last_num) { // try to send last block next time
                    break;
                } // else: skip the block if not exist

                it->startBlock++;
                send_count++;
            }

            if (it->startBlock > it->endBlock) {
                it = m_syncTaskQueue.erase(it);
            } else {
                ++it;
            }
        }

    }

    //NOTE: The template T must be type of map because the erase operation is not generalized.
    template<class T>
    void Scheduler::clearMsgCache(T &cache, uint32_t blockNum) {
        for (auto msg_it = cache.begin(); msg_it != cache.end();) {
            if (msg_it->first.blockNum <= blockNum) {
                cache.erase(msg_it++);
            } else {
                ++msg_it;
            }
        }
    }

    uint32_t Scheduler::getFastTimestamp() {
        return m_fastTimestamp;
    }

    void Scheduler::resetTimestamp() {
        m_fastTimestamp = 0;
    }

    void Scheduler::clearOldCachedProposeMsg() {
        uint32_t old_block_num = std::numeric_limits<uint32_t>::max();
        for (auto &it : m_cacheProposeMsgMap) {
            if (it.first.blockNum < old_block_num) {
                old_block_num = it.first.blockNum;
            }
        }

        wlog("m_cacheProposeMsgMap exceeds ${max}, echo msgs with block num ${num} will be cleared",
             ("max", m_maxCachePropose)("num", old_block_num));
        for (auto it = m_cacheProposeMsgMap.begin(); it != m_cacheProposeMsgMap.end();) {
            if (it->first.blockNum == old_block_num) {
                m_cacheProposeMsgMap.erase(it++);
            } else {
                ++it;
            }
        }
    }

    void Scheduler::clearOldCachedEchoMsg() {
        uint32_t old_block_num = std::numeric_limits<uint32_t>::max();
        for (auto &it : m_cacheEchoMsgMap) {
            if (it.first.blockNum < old_block_num) {
                old_block_num = it.first.blockNum;
            }
        }

        wlog("m_cacheEchoMsgMap exceeds ${max}, echo msgs with block num ${num} will be cleared",
             ("max", m_maxCacheEcho)("num", old_block_num));
        for (auto it = m_cacheEchoMsgMap.begin(); it != m_cacheEchoMsgMap.end();) {
            if (it->first.blockNum == old_block_num) {
                m_cacheEchoMsgMap.erase(it++);
            } else {
                ++it;
            }
        }
    }

    bool Scheduler::isEmpty(const BlockIdType& blockId) {
        return emptyBlock().id() == blockId;
    }

    bool Scheduler::isBlank(const BlockIdType& blockId) {
        return Block().id() == blockId;
    }

    std::shared_ptr<Block> Scheduler::generateEmptyBlock() {
        ilog("Scheduler::generateEmptyBlock()");
        chain::controller &chain = appbase::app().get_plugin<chain_plugin>().chain();
        chain.abort_block();
        // get_proper_next_block_timestamp() probably can't be used here, we have to be
        // deterministic about the empty block's timestamp.
        auto block_timestamp = chain.head_block_time() + fc::milliseconds(chain::config::block_interval_ms);
        chain.start_block(block_timestamp, getCommitteeMroot(chain.head_block_num() + 1), "");
        if (EpochEndPoint::isEpochEndPoint(chain.head_block_header())) {
            std::shared_ptr<StakeVoteBase> stakeVotePtr = MsgMgr::getInstance()->getStakeVote(chain.head_block_num() + 1);
            m_lightClientProducer->handleCheckPoint(chain, stakeVotePtr->getCommitteeSet());
        }
        chain.finish_block_hack();
        chain.set_action_merkle_hack();
        // empty block does not have trx, so we don't need this?
        chain.set_trx_merkle_hack();
        std::shared_ptr<Block> blockPtr = std::make_shared<Block>();
        const auto &pbs = chain.pending_block_state();
        const auto &bh = pbs->header;
        blockPtr->timestamp = bh.timestamp;
        blockPtr->previous = bh.previous;
        blockPtr->transaction_mroot = bh.transaction_mroot;
        blockPtr->action_mroot = bh.action_mroot;
        blockPtr->committee_mroot = bh.committee_mroot;
        blockPtr->header_extensions = bh.header_extensions;
        blockPtr->proposer = name("utrio.empty");
        chain.abort_block();
        return blockPtr;
    }

    Block Scheduler::blankBlock() {
        return Block();
    }

    void Scheduler::setBa0Block(const Block& block) {
        m_ba0Block = block;
    }

    Block Scheduler::emptyBlock() {
        static std::shared_ptr<Block> emptyBlock = nullptr; // static variant for cache
        if (!emptyBlock || emptyBlock->block_num() != Node::getInstance()->getBlockNum()) {
            emptyBlock = generateEmptyBlock();
        }
        return *emptyBlock;
    }

    void Scheduler::insertAccount(VoterSet& voterSet, const EchoMsg &echo) {
        auto pkItor = std::find(voterSet.accountPool.begin(), voterSet.accountPool.end(), echo.account);
        if (pkItor == voterSet.accountPool.end()) {
            voterSet.accountPool.push_back(echo.account);
        }
    }

    void Scheduler::enableEventRegister(bool v) {
        chain::controller &chain = appbase::app().get_plugin<chain_plugin>().chain();
        chain.enable_event_register(v);
    }

    bool Scheduler::setBlsVoterSet(const std::string& bls) {
        BlsVoterSet b(bls);
        if (!b.valid()) {
            elog("receive invalid bls voter set: ${bls}", ("bls", bls));
            return false;
        }

        uint32_t last_num = getLastBlocknum();
        if (BlockHeader::num_from_id(b.commonEchoMsg.blockId) == last_num + 1) {
            m_currentBlsVoterSet = b;
            ilog("before save bls voter set. last num: ${num} bls: ${bls}", ("num", last_num)("bls", m_currentBlsVoterSet.toString()));
            m_lightClientProducer->saveCurrentBlsVoterSet(m_currentBlsVoterSet);
            return true;
        } else {
            elog("error block num in bls voter set: ${num}, but last num: ${last}", ("num", BlockHeader::num_from_id(b.commonEchoMsg.blockId))("last", last_num));
            return false;
        }
    }

    bool Scheduler::verifyMyBlsSignature(const EchoMsg& echo) const {
        uint32_t blockNum = BlockHeader::num_from_id(echo.blockId);
        std::shared_ptr<StakeVoteBase> voterSysPtr = MsgMgr::getInstance()->getStakeVote(blockNum);
        unsigned char blsPk[Bls::BLS_PUB_KEY_COMPRESSED_LENGTH];
        bool res = voterSysPtr->getCommitteeBlsPublicKey(StakeVoteBase::getMyAccount(), blsPk, Bls::BLS_PUB_KEY_COMPRESSED_LENGTH);
        if (!res) {
            elog("account ${account} is not in committee", ("account", std::string(StakeVoteBase::getMyAccount())));
            return false;
        }
        return Validator::verify<CommonEchoMsg>(echo.blsSignature, echo, blsPk);
    }

    BlsVoterSet Scheduler::toBlsVoterSetAndFindEvil(const VoterSet& voterSet, const CommitteeSet& committeeSet,
            bool genesisPeriod, int weight) const {
        BlsVoterSet blsVoterSet = voterSet.toBlsVoterSet(weight);
        if (!genesisPeriod && !committeeSet.verify(blsVoterSet)) {
            // there are evil node
            VoterSet newVoterSet;
            std::vector<AccountName> evilAccounts;
            EvilBlsDetector detector;
            detector.detect(voterSet, committeeSet, newVoterSet, evilAccounts);
            // TODO(xiaofen) punish
            for (auto evil : evilAccounts) {
                elog("evil account : ${evil}", ("evil", std::string(evil)));
            }
            blsVoterSet = newVoterSet.toBlsVoterSet(weight);
        }
        return blsVoterSet;
    }

    void Scheduler::invokeDeduceWhenBax() {
        uint32_t blockNum = Node::getInstance()->getBlockNum();
        std::shared_ptr<StakeVoteBase> stakeVotePtr = MsgMgr::getInstance()->getStakeVote(blockNum);
        m_evilDDosDetector.deduceWhenBax(stakeVotePtr->getSendEchoThreshold() + 1,
                Node::getInstance()->getRoundCount(), blockNum, Node::getInstance()->getPhase());
    }

    bool Scheduler::theSameOne(const ultrainio::chain::signed_block& lhs, const ultrainio::chain::signed_block_ptr& rhs) {
        if (&lhs == rhs.get()) {
            return true;
        }
        return lhs.id() == rhs->id();
    }

}  // namespace ultrainio
