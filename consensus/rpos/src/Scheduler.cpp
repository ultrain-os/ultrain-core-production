#include <rpos/Scheduler.h>

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>

#include <boost/asio.hpp>
#include <fc/log/logger.hpp>
#include <fc/crypto/sha256.hpp>
#include <fc/scoped_exit.hpp>

#include <appbase/application.hpp>
#include <ultrainio/chain_plugin/chain_plugin.hpp>
#include <ultrainio/chain/block_timestamp.hpp>
#include <ultrainio/chain/config.hpp>
#include <ultrainio/chain/controller.hpp>
#include <ultrainio/chain/exceptions.hpp>
#include <ultrainio/chain/global_property_object.hpp>
#include <ultrainio/chain/name.hpp>
#include <ultrainio/chain/exceptions.hpp>
#include <ultrainio/chain/config.hpp>

#include <rpos/Config.h>
#include <rpos/MsgBuilder.h>
#include <rpos/MsgMgr.h>
#include <rpos/Node.h>
#include <rpos/Seed.h>
#include <rpos/Signer.h>
#include <rpos/StakeVoteBase.h>
#include <rpos/Validator.h>
#include <rpos/Vrf.h>

#include <appbase/application.hpp>

using namespace boost::asio;
using namespace std;
using namespace appbase;

namespace fc {
    extern std::unordered_map<std::string, logger> &get_logger_map();
}

namespace {
    const fc::string logger_name("Scheduler");
    fc::logger _log;

    // TODO(shenyufeng) need more precise compare
    bool IsBa0TheRightBlock(const ultrainio::chain::signed_block &ba0_block,
                            const ultrainio::chain::signed_block_ptr &block) {
        return  (ba0_block.proposer == block->proposer &&
                 ba0_block.timestamp == block->timestamp &&
                 ba0_block.transaction_mroot == block->transaction_mroot &&
                 ba0_block.action_mroot == block->action_mroot &&
                 ba0_block.committee_mroot == block->committee_mroot &&
                 ba0_block.previous == block->previous);
    }
}

namespace ultrainio {

    Scheduler::Scheduler() : m_ba0Block(), m_proposerMsgMap(), m_echoMsgMap(),
                                           m_cacheProposeMsgMap(), m_cacheEchoMsgMap(),
                                           m_echoMsgAllPhase() {
        m_syncTaskPeriod = {std::chrono::seconds{1}};
        m_syncTaskTimer.reset(new boost::asio::steady_timer(app().get_io_service()));
        m_memleakCheck.reset(new boost::asio::steady_timer(app().get_io_service()));
        start_memleak_check();
        m_fast_timestamp = 0;
    }

    chain::checksum256_type Scheduler::getCommitteeMroot(uint32_t block_num) {
        std::shared_ptr<StakeVoteBase> voterSysPtr = MsgMgr::getInstance()->getVoterSys(block_num);
        if (voterSysPtr) {
            return voterSysPtr->getCommitteeMroot();
        } else {
            return chain::checksum256_type();
        }
    }

    void Scheduler::start_memleak_check() {
      m_memleakCheck->expires_from_now(m_memleakCheckPeriod);
      m_memleakCheck->async_wait( [this](boost::system::error_code ec) {
            if( !ec) {
                start_memleak_check();
                int s1 = m_proposerMsgMap.size();

                int s2 = m_echoMsgMap.size();
                int s3 = 0;
                for(const auto& it : m_echoMsgMap) {
                    s3 += it.second.accountPool.size();
                    s3 += it.second.proofPool.size();
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
                        s9 += it2.second.proofPool.size();
                        s9 += it2.second.sigPool.size();
                        s9 += it2.second.timePool.size();
                    }
                }

                int s10 = 0;
                int s11 = 0;
                int s12 = 0;
                int s13 = 0;
                auto mm = MsgMgr::getInstance();
                if (mm) {
                    s10 = mm->blockMessageMap.size();
                    for(const auto& it :  mm->blockMessageMap) {
                        const auto& it2 = *(it.second);
                        s11 += it2.m_proposeMsgList.size();
                        s12 += it2.m_phaseMessageMap.size();
                        s13 += it2.m_aggEchoMsgV.size();
                    }
                }
                ilog("memleak check m_proposerMsgMap ${1}, m_echoMsgMap ${2} ${3} m_cacheProposeMsgMap ${4} ${5} m_cacheEchoMsgMap ${6} ${7} m_echoMsgAllPhase ${8} ${9} blockMessageMap ${10} m_proposeMsgList ${11} m_phaseMessageMap ${12} m_aggEchoMsgV ${13}",
                     ("1", s1)("2", s2)("3", s3)("4", s4)("5", s5)("6", s6)
                     ("7", s7)("8", s8)("9", s9)("10", s10)("11", s11)("12", s12)
                     ("13",s13));
            }
            else {
               elog( "Error from memleak check monitor: ${m}",( "m", ec.message()));
               start_memleak_check( );
            }
         });
    }

    void Scheduler::reset() {
        uint32_t blockNum = getLastBlocknum();
        m_ba0Block = Block();
        m_ba0VerifiedBlkId = BlockIdType();
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
            }
        } else {
            echo_message_info echoMessageInfo;
            echoMessageInfo.echo = echo;
            echoMessageInfo.accountPool.push_back(echo.account);
            echoMessageInfo.sigPool.push_back(echo.signature);
            echoMessageInfo.timePool.push_back(echo.timestamp);
            echoMessageInfo.hasSend = true;
            m_echoMsgMap.insert(make_pair(echo.blockId, echoMessageInfo));
        }
        return true;
    }

    bool Scheduler::insert(const ProposeMsg &propose) {
        dlog("insert.save propose msg.blockhash = ${blockhash}", ("blockhash", propose.block.id()));
        m_proposerMsgMap.insert(make_pair(propose.block.id(), propose));
        return true;
    }

    bool Scheduler::isLaterMsg(const EchoMsg &echo) {
        uint32_t currentBlockNum = UranusNode::getInstance()->getBlockNum();
        ConsensusPhase current_phase = UranusNode::getInstance()->getPhase();
        uint32_t current_bax_count = UranusNode::getInstance()->getBaxCount();

        uint32_t blockNum = BlockHeader::num_from_id(echo.blockId);
        if (blockNum > currentBlockNum) {
            return true;
        }

        if (blockNum == currentBlockNum) {
            if (echo.phase > current_phase) {
                return true;
            } else if ((echo.phase == current_phase) && (echo.baxCount > current_bax_count)) {
                return true;
            }
        }

        return false;
    }

    bool Scheduler::isLaterMsg(const ProposeMsg &propose) {
        uint32_t currentBlockNum = UranusNode::getInstance()->getBlockNum();
        ConsensusPhase current_phase = UranusNode::getInstance()->getPhase();

        if (propose.block.block_num() > currentBlockNum) {
            return true;
        }

        if ((propose.block.block_num() == currentBlockNum)
            // Default genesis block is #1, so the first block
            // nodes are working on is #2.
            // && (currentBlockNum == 2)
            && (current_phase == kPhaseInit)) {
            return true;
        }

        return false;
    }

    bool Scheduler::isLaterMsgAndCache(const EchoMsg &echo, bool &duplicate) {
        duplicate = false;
        if (isLaterMsg(echo)) {
            dlog("isLaterMsgAndCache. later msg.");
            msgkey key;
            key.blockNum = BlockHeader::num_from_id(echo.blockId);
            key.phase = echo.phase + echo.baxCount;

            auto itor = m_cacheEchoMsgMap.find(key);
            if (itor == m_cacheEchoMsgMap.end()) {
                if (m_cacheEchoMsgMap.size() >= m_maxCacheEcho) {
                    //NOTE: itor will be invalid after the operation below.
                    clearOldCachedEchoMsg();
                }
                std::vector<EchoMsg> echo_vector;
                echo_vector.push_back(echo);
                m_cacheEchoMsgMap.insert(make_pair(key, echo_vector));
            } else {
                auto id = echo.blockId;
                std::vector<EchoMsg> &ev = itor->second;
                for (size_t i = 0; i < ev.size(); i++) {
                    if (ev[i].account == echo.account && ev[i].blockId == id) {
                        ilog("duplicate echo msg!!! id:${id} account:${account} blockNum:${b} phase:${p}",
                             ("id", echo.blockId)("account", std::string(echo.account))("b", key.blockNum)("p", key.phase));
                        duplicate = true;
                        return true;
                    }
                }

                if (ev.size() < m_maxCommitteeSize) {
                    ev.emplace_back(echo);
                } else {
                    ilog("Size of vector in m_cacheEchoMsgMap exceeds ${mcs}", ("mcs", m_maxCommitteeSize));
                }
            }
            dlog("next phase echo msg. blockNum = ${id}, phase = ${phase},baxcount = ${baxcount}",
                 ("id", key.blockNum)("phase", (uint32_t) echo.phase)("baxcount",echo.baxCount));
            return true;
        }
        return false;
    }

    bool Scheduler::isLaterMsgAndCache(const ProposeMsg &propose, bool &duplicate) {
        duplicate = false;
        if (isLaterMsg(propose)) {
            msgkey key;
            key.blockNum = propose.block.block_num();
            key.phase = kPhaseBA0;

            auto itor = m_cacheProposeMsgMap.find(key);
            if (itor == m_cacheProposeMsgMap.end()) {
                if (m_cacheProposeMsgMap.size() >= m_maxCachePropose) {
                    //NOTE: itor will be invalid after the operation below.
                    clearOldCachedProposeMsg();
                }
                std::vector<ProposeMsg> propose_vector;
                propose_vector.push_back(propose);
                m_cacheProposeMsgMap.insert(make_pair(key, propose_vector));
            } else {
                auto id = propose.block.id();
                std::vector<ProposeMsg> &pv = itor->second;
                for (size_t i = 0; i < pv.size(); i++) {
                    if (pv[i].block.proposer == propose.block.proposer && pv[i].block.id() == id) {
                        ilog("duplicate propose msg!!! id:${id} pk:${pk} blockNum:${b} phase:${p}",
                             ("id", propose.block.id())("pk", propose.block.proposer)("b", key.blockNum)("p",
                                                                                                           key.phase));
                        duplicate = true;
                        return true;
                    }
                }

                if (pv.size() < m_maxCommitteeSize) {
                    pv.emplace_back(propose);
                }
                else {
                    wlog("Size of vector in cacheProposeMsgMap exceeds ${mcs}", ("mcs", m_maxCommitteeSize));
                }
            }
            dlog("next phase propose msg. blockNum = ${id}", ("id", key.blockNum));
            return true;
        }
        return false;
    }

    bool Scheduler::isBeforeMsg(const EchoMsg &echo) {
        AccountName myAccount = StakeVoteBase::getMyAccount();

        if (myAccount == echo.account) {
            elog("loopback echo. account : ${account}", ("account", std::string(myAccount)));
            return false;
        }
        uint32_t blockNum = BlockHeader::num_from_id(echo.blockId);
        if (blockNum != UranusNode::getInstance()->getBlockNum()) {
            elog("invalid echo msg . blockNum = ${id1}. local blockNum = ${id2}",
                 ("id1", blockNum)("id2", UranusNode::getInstance()->getBlockNum()));
            return false;
        }
        if ((UranusNode::getInstance()->getPhase() == kPhaseBAX) && (echo.phase != kPhaseBA0)) {
            if (UranusNode::getInstance()->getBaxCount() > echo.baxCount) {
                dlog("isBeforeMsg. before msg.");
                return true;
            }
        }

        return false;
    }

    bool Scheduler::processBeforeMsg(const EchoMsg &echo) {
        msgkey msg_key;
        msg_key.blockNum = BlockHeader::num_from_id(echo.blockId);
        msg_key.phase = echo.phase + echo.baxCount;
        echo_msg_buff echo_msg_map;

        dlog("processBeforeMsg.");

        auto map_it = m_echoMsgAllPhase.find(msg_key);
        if (map_it == m_echoMsgAllPhase.end()) {
            if (m_echoMsgAllPhase.size() >= m_maxCachedAllPhaseKeys) {
                dlog("processBeforeMsg.map reach the up limit. size = ${size}",("size",m_echoMsgAllPhase.size()));
                return false;
            }
            auto result = m_echoMsgAllPhase.insert(make_pair(msg_key, echo_msg_map));
            map_it = result.first;
        }

        //dlog("processBeforeMsg.insert ok,cal voters begin.");

        auto itor = map_it->second.find(echo.blockId);
        if (itor != map_it->second.end()) {
            //dlog("processBeforeMsg.blockhash is already exist.");
            if (updateAndMayResponse(itor->second, echo, false)) {
                if (isMinEcho(itor->second,map_it->second) || isMinFEcho(itor->second,map_it->second)) {
                    return true;
                }
            }
        } else {
            echo_message_info info;
            info.echo = echo;
            //dlog("processBeforeMsg.new blockhash.");
            if (updateAndMayResponse(info, echo, false)) {
                map_it->second.insert(make_pair(echo.blockId, info));
                if (isMinEcho(info,map_it->second) || isMinFEcho(info,map_it->second)) {
                    return true;
                }
            }
        }
        return false;
    }

    bool Scheduler::updateAndMayResponse(echo_message_info &info, const EchoMsg &echo, bool response) {
        uint32_t blockNum = BlockHeader::num_from_id(echo.blockId);
        ilog("update echo blockId: ${id}", ("id", echo.blockId));
        auto pkItor = std::find(info.accountPool.begin(), info.accountPool.end(), echo.account);
        if (pkItor == info.accountPool.end()) {
            info.accountPool.push_back(echo.account);
            info.sigPool.push_back(echo.signature);
            info.timePool.push_back(echo.timestamp);
            std::shared_ptr<StakeVoteBase> stakeVotePtr = MsgMgr::getInstance()->getVoterSys(blockNum);
            if (response && info.accountPool.size() >= stakeVotePtr->getSendEchoThreshold() && !info.hasSend
                && UranusNode::getInstance()->getPhase() == kPhaseBA0 && isMinFEcho(info)) {
                if (MsgMgr::getInstance()->isVoter(UranusNode::getInstance()->getBlockNum(), echo.phase,
                                                           echo.baxCount)) {
                    ilog("send echo when > f + 1");
                    info.hasSend = true;
                    EchoMsg myEcho = MsgBuilder::constructMsg(echo);
                    insert(myEcho);
                    //myEcho.timestamp = UranusNode::getInstance()->getRoundCount();
                    UranusNode::getInstance()->sendMessage(myEcho);
                }
            }
            //ilog("updateAndMayResponse new pk insert.");
            return true;
        }
        return false;
    }

    uint32_t Scheduler::isSyncing() {
        uint32_t maxBlockNum = UranusNode::getInstance()->getBlockNum();
        AccountName myAccount = StakeVoteBase::getMyAccount();

        if (m_cacheEchoMsgMap.empty()) {
            return INVALID_BLOCK_NUM;
        }

        if (UranusNode::getInstance()->getSyncingStatus()) {
            elog("node is syncing.");
            return INVALID_BLOCK_NUM;
        }

        for (auto vector_itor = m_cacheEchoMsgMap.begin(); vector_itor != m_cacheEchoMsgMap.end(); ++vector_itor) {
            if (vector_itor->first.blockNum > maxBlockNum) {
                echo_msg_buff echo_msg_map;
                //echo_message_info echo_info;

                for (auto &echo : vector_itor->second) {
                    if (myAccount == echo.account) {
                        elog("loopback echo. account : ${account}", ("account", std::string(myAccount)));
                        continue;
                    }

                    auto itor = echo_msg_map.find(echo.blockId);
                    if (itor != echo_msg_map.end()) {
                        insertAccount(itor->second, echo);
                    } else {
                        echo_message_info info;
                        info.echo = echo;
                        insertAccount(info, echo);
                        echo_msg_map.insert(make_pair(echo.blockId, info));
                    }
                }

                for (auto echo_itor = echo_msg_map.begin(); echo_itor != echo_msg_map.end(); ++echo_itor) {
                    if (echo_itor->second.accountPool.size() >= THRESHOLD_SYNCING) {
                        maxBlockNum = vector_itor->first.blockNum;
                        return maxBlockNum;
                    }
                }
            }
        }

        return INVALID_BLOCK_NUM;
    }

    bool Scheduler::isChangePhase() {
        AccountName myAccount = StakeVoteBase::getMyAccount();
        std::shared_ptr<StakeVoteBase> stakeVotePtr = MsgMgr::getInstance()->getVoterSys(UranusNode::getInstance()->getBlockNum());

        if (m_cacheEchoMsgMap.empty()) {
            return false;
        }

        for (auto vector_itor = m_cacheEchoMsgMap.begin(); vector_itor != m_cacheEchoMsgMap.end(); ++vector_itor) {
            if ((vector_itor->first.blockNum == UranusNode::getInstance()->getBlockNum())
                && (vector_itor->first.phase >= Config::kMaxBaxCount)) {
                echo_msg_buff echo_msg_map;

                for (auto &echo : vector_itor->second) {
                    if (myAccount == echo.account) {
                        elog("loopback echo. account : ${account}", ("account", std::string(myAccount)));
                        continue;
                    }

                    auto itor = echo_msg_map.find(echo.blockId);
                    if (itor != echo_msg_map.end()) {
                        updateAndMayResponse(itor->second, echo, false);
                    } else {
                        echo_message_info info;
                        info.echo = echo;
                        updateAndMayResponse(info, echo, false);
                        echo_msg_map.insert(make_pair(echo.blockId, info));
                    }
                }

                for (auto echo_itor = echo_msg_map.begin(); echo_itor != echo_msg_map.end(); ++echo_itor) {
                    if (echo_itor->second.accountPool.size() >= stakeVotePtr->getSendEchoThreshold()) {
                        return true;
                    }
                }
            }
        }

        return false;
    }

    bool Scheduler::isBroadcast(const EchoMsg &echo) {
        uint32_t blockNum = BlockHeader::num_from_id(echo.blockId);

        if (blockNum != UranusNode::getInstance()->getBlockNum()) {
            dlog("invalid echo msg, only broadcast. blockNum = ${id1}. local blockNum = ${id2}",
                 ("id1", blockNum)("id2", UranusNode::getInstance()->getBlockNum()));
            return true;
        }

        if (static_cast<ConsensusPhase>(echo.phase) != UranusNode::getInstance()->getPhase()) {
            dlog("invalid echo msg, only broadcast. phase = ${phase1}. local phase = ${phase2}",
                 ("phase1", (uint32_t) echo.phase)("phase2", (uint32_t) UranusNode::getInstance()->getPhase()));
            return true;
        }

        return false;
    }

    bool Scheduler::isValid(const EchoMsg &echo) {
        uint32_t blockNum = BlockHeader::num_from_id(echo.blockId);
        AccountName myAccount = StakeVoteBase::getMyAccount();
        if (myAccount == echo.account) {
            elog("loopback echo. account : ${account}", ("account", std::string(myAccount)));
            return false;
        }

        if (echo.phase == kPhaseInit) {
            elog("invalid echo msg . phase = kPhaseInit");
            return false;
        }

        std::shared_ptr<StakeVoteBase> stakeVotePtr = MsgMgr::getInstance()->getVoterSys(blockNum);
        PublicKey publicKey = stakeVotePtr->getPublicKey(echo.account);
        if (!Validator::verify<UnsignedEchoMsg>(Signature(echo.signature), echo, publicKey)) {
            elog("validator echo error. account : ${account} pk : ${pk} signature : ${signature}",
                    ("account", std::string(echo.account))("pk", std::string(publicKey))("signature", echo.signature));
            return false;
        }

        if (!stakeVotePtr->isVoter(echo.account, UranusNode::getInstance()->getNonProducingNode())) {
            elog("send echo by no voter. account : ${account}", ("account", std::string(echo.account)));
            return false;
        }
        return true;
    }

    bool Scheduler::isBroadcast(const ProposeMsg &propose) {
        uint32_t blockNum = propose.block.block_num();
        uint32_t myBlockNum = UranusNode::getInstance()->getBlockNum();

        if (blockNum != myBlockNum) {
            dlog("invalid propose msg, only broadcast. blockNum = ${id1}. local blockNum = ${id2}",
                 ("id1", blockNum)("id2", myBlockNum));
            return true;
        }

        if (kPhaseBA0 != UranusNode::getInstance()->getPhase()) {
            dlog("invalid propose msg, only broadcast. phase = ${phase}", ("phase", uint32_t(UranusNode::getInstance()->getPhase())));
            return true;
        }

        return false;
    }

    bool Scheduler::isValid(const ProposeMsg &propose) {
        AccountName myAccount = StakeVoteBase::getMyAccount();
        if (myAccount == propose.block.proposer) {
            elog("invalid propose. msg loopback. account : ${account}", ("account", std::string(myAccount)));
            return false;
        }

        std::shared_ptr<StakeVoteBase> stakeVotePtr = MsgMgr::getInstance()->getVoterSys(propose.block.block_num());
        PublicKey publicKey = stakeVotePtr->getPublicKey(propose.block.proposer);
        if (!Validator::verify<BlockHeader>(Signature(propose.block.signature), propose.block, publicKey)) {
            elog("validator proposer error. proposer : ${proposer}", ("proposer", std::string(propose.block.proposer)));
            return false;
        }

        const auto c_mroot = stakeVotePtr->getCommitteeMroot();
        if (c_mroot != propose.block.committee_mroot) {
            elog("verify committee mroot error. own c_mroot : ${m1}, block mroot ${m2}",
                 ("m1", c_mroot) ("m2", propose.block.committee_mroot));
            return false;
        }

        if (!stakeVotePtr->isProposer(propose.block.proposer, UranusNode::getInstance()->getNonProducingNode())) {
            elog("send propose by non proposer. account : ${account}", ("account", std::string(propose.block.proposer)));
            return false;
        }
        return true;
    }

    bool Scheduler::fastHandleMessage(const ProposeMsg &propose) {
        if (isBroadcast(propose)) {
            return false;
        }

        if (!isValid(propose)) {
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
    }

    bool Scheduler::fastHandleMessage(const EchoMsg &echo) {
        if (isBroadcast(echo)) {
            return false;
        }

        if (!isValid(echo)) {
            return false;
        }

        if (m_fast_timestamp < echo.timestamp) {
            m_fast_timestamp = echo.timestamp;
        }

        auto itor = m_echoMsgMap.find(echo.blockId);
        if (itor != m_echoMsgMap.end()) {
            updateAndMayResponse(itor->second, echo, false);
        } else {
            echo_message_info info;
            info.echo = echo;
            updateAndMayResponse(info, echo, false);
            m_echoMsgMap.insert(make_pair(echo.blockId, info));
        }
        return true;
    }

    bool Scheduler::handleMessage(const ProposeMsg &propose) {
        bool duplicate = false;
        if (isLaterMsgAndCache(propose, duplicate)) {
            return (!duplicate);
        }

        if (isBroadcast(propose)) {
            return false;
        }

        if (isDuplicate(propose)) {
            return false;
        }

        if (!isValid(propose)) {
            return false;
        }

        if ((UranusNode::getInstance()->getSyncingStatus()) && (UranusNode::getInstance()->getPhase() != kPhaseBAX)) {
            dlog("receive propose msg. node is syncing. blockhash = ${blockhash}", ("blockhash", propose.block.id()));
            return true;
        }

        dlog("receive propose msg.blockhash = ${blockhash}", ("blockhash", propose.block.id()));
        auto itor = m_proposerMsgMap.find(propose.block.id());
        if (itor == m_proposerMsgMap.end()) {
            if (isMinPropose(propose)) {
                if (MsgMgr::getInstance()->isVoter(propose.block.block_num(), kPhaseBA0, 0)) {
                    EchoMsg echo = MsgBuilder::constructMsg(propose);
                    //echo.timestamp = UranusNode::getInstance()->getRoundCount();
                    UranusNode::getInstance()->sendMessage(echo);
                    insert(echo);
                }
                dlog("save propose msg.blockhash = ${blockhash}", ("blockhash", propose.block.id()));
                m_proposerMsgMap.insert(make_pair(propose.block.id(), propose));
                return true;
            }
        }
        return false;
    }

    bool Scheduler::handleMessage(const EchoMsg &echo) {
        bool duplicate = false;
        bool bret = false;

        if (isLaterMsgAndCache(echo, duplicate)) {
            return (!duplicate);
        }

        if (isBeforeMsg(echo)) {
            return processBeforeMsg(echo);
        }

        if (isBroadcast(echo)) {
            return false;
        }

        if (!isValid(echo)) {
            return false;
        }

        if ((UranusNode::getInstance()->getSyncingStatus()) && (UranusNode::getInstance()->getPhase() != kPhaseBAX)) {
            dlog("receive echo msg. node is syncing. blockhash = ${blockhash} echo'account = ${account}",
                 ("blockhash", echo.blockId)("account", std::string(echo.account)));
            return true;
        }

        dlog("receive echo msg.blockhash = ${blockhash} echo'account = ${account} signature : ${signature}",
             ("blockhash", echo.blockId)("account", std::string(echo.account))("signature", echo.signature));
        auto itor = m_echoMsgMap.find(echo.blockId);
        if (itor != m_echoMsgMap.end()) {
            bret = updateAndMayResponse(itor->second, echo, true);
            if ((isMinEcho(itor->second) || isMinFEcho(itor->second)) && bret) {
                return true;
            }
        } else {
            echo_message_info info;
            info.echo = echo;
            bret = updateAndMayResponse(info, echo, true);
            m_echoMsgMap.insert(make_pair(echo.blockId, info));
            if ((isMinEcho(info) || isMinFEcho(info)) && bret) {
                return true;
            }
        }
        return false;
    }

    bool Scheduler::handleMessage(const string &peer_addr, const ReqSyncMsg &msg) {
        if (UranusNode::getInstance()->getSyncingStatus()) {
            return true;
        }

        if (peer_addr.empty() || m_syncTaskQueue.size() >= m_maxSyncClients) {
            ilog("peer addr is empty or sync task queue is full. peer addr:${p} queue size:${qz}",
                 ("p", peer_addr)("qz", m_syncTaskQueue.size()));
            return false;
        }

        for (std::list<SyncTask>::iterator l_it = m_syncTaskQueue.begin(); l_it != m_syncTaskQueue.end(); ++l_it) {
            if (l_it->peerAddr == peer_addr) {
                ilog("peer addr ${p} has been already in sync queue.", ("p", peer_addr));
                return false;
            }
        }

        uint32_t end_block_num = msg.endBlockNum <= getLastBlocknum() + 1 ? msg.endBlockNum : getLastBlocknum() + 1;
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
                UranusNode::getInstance()->sendMessage(peer_addr, sync_block);
            } else if (num == end_block_num) { // try to send last block next time
                break;
            } // else: skip the block if not exist
        }

        if (num <= end_block_num) {
            if (end_block_num > num + m_maxSyncBlocks) {
                end_block_num = num + m_maxSyncBlocks;
            }
            m_syncTaskQueue.emplace_back(peer_addr, num, end_block_num, msg.seqNum);
        }

        return true;
    }

    bool Scheduler::handleMessage(const string &peer_addr, const ReqLastBlockNumMsg &msg) {
        RspLastBlockNumMsg rsp_msg;
        rsp_msg.seqNum = msg.seqNum;
        rsp_msg.blockNum = getLastBlocknum();

        chain::controller &chain = appbase::app().get_plugin<chain_plugin>().chain();
        auto b = chain.fetch_block_by_number(rsp_msg.blockNum);
        if (b) {
            rsp_msg.blockHash = b->id();
            rsp_msg.prevBlockHash = b->previous;
        } else {
            rsp_msg.blockNum = INVALID_BLOCK_NUM;
        }

        UranusNode::getInstance()->sendMessage(peer_addr, rsp_msg);
        return true;
    }

    uint32_t Scheduler::getLastBlocknum() {
        const chain::controller &chain = appbase::app().get_plugin<chain_plugin>().chain();
        return chain.head_block_num();
    }

    bool Scheduler::handleMessage(const Block &block) {
        uint32_t last_num = getLastBlocknum();
        dlog("@@@@@@@@@@@@@@@ last block num in local chain:${last}", ("last", last_num));
        chain::controller &chain = appbase::app().get_plugin<chain_plugin>().chain();
        auto b = chain.fetch_block_by_number(last_num);
        if (b) {
            if (block.previous == b->id()) {
                // TODO(yufengshen) -- Do not copy here, should have shared_ptr at the first place.
                produceBlock(std::make_shared<chain::signed_block>(block), true);
                dlog("apply block finish blockNum = ${block_num}, hash = ${hash}, head_hash = ${head_hash}",
                     ("block_num", getLastBlocknum())("hash", block.id())("head_hash", block.previous));
                return true;
            } else {
                elog("block error. block in local chain: blockNum = ${n} hash = ${hash}", ("n", last_num)("hash", b->id()));
                elog("comming block num:${n} hash:${h} previous hash:${ph}",
                     ("n", block.block_num())("h", block.id())("ph", block.previous));
                return false;
            }
        }

        elog("Error!!! Get last block failed!!! num:${n}", ("n", last_num));
        return false;
    }

    bool Scheduler::handleMessage(const string &peer_addr, const SyncStopMsg &msg) {
        ilog("Stop sync msg to ${pa}, seqNum: ${sn}", ("pa", peer_addr)("sn", msg.seqNum));
        if (m_syncTaskQueue.empty()) {
            return true;
        }

        for (std::list<SyncTask>::iterator it = m_syncTaskQueue.begin(); it != m_syncTaskQueue.end(); ++it) {
            if (it->peerAddr == peer_addr) {
                if (it->seqNum != msg.seqNum) {
                    elog("seqNum in task:${snt} != seqNum in msg:${snm}, but we still stop sync msg.", ("snt", it->seqNum)("snm", msg.seqNum));
                }
                m_syncTaskQueue.erase(it);
                break;
            }
        }

        return true;
    }

    bool Scheduler::isMin2FEcho(int totalVoterWeight, uint32_t phasecnt) {
        std::shared_ptr<StakeVoteBase> stakeVotePtr = MsgMgr::getInstance()->getVoterSys(UranusNode::getInstance()->getBlockNum());
        ULTRAIN_ASSERT(stakeVotePtr, chain::chain_exception, "stakeVotePtr is null");
        if ((totalVoterWeight >= stakeVotePtr->getEmptyBlockThreshold()) && (phasecnt >= Config::kMaxBaxCount)) {
            return true;
        }

        if ((totalVoterWeight >= stakeVotePtr->getNextRoundThreshold()) && (phasecnt < Config::kMaxBaxCount)) {
            return true;
        }

        if ((totalVoterWeight >= stakeVotePtr->getEmptyBlock2Threshold()) && (phasecnt >= Config::kDeadlineCnt)) {
            return true;
        }

        return false;
    }

    bool Scheduler::isMinPropose(const ProposeMsg &proposeMsg) {
        std::shared_ptr<StakeVoteBase> stakeVotePtr = MsgMgr::getInstance()->getVoterSys(proposeMsg.block.block_num());
        ULTRAIN_ASSERT(stakeVotePtr, chain::chain_exception, "stakeVotePtr is null");
        int priority = stakeVotePtr->proposerPriority(proposeMsg.block.proposer);
        for (auto itor = m_proposerMsgMap.begin(); itor != m_proposerMsgMap.end(); ++itor) {
            if (stakeVotePtr->proposerPriority(itor->second.block.proposer) < priority) {
                return false;
            }
        }
        return true;
    }

    bool Scheduler::isMinFEcho(const echo_message_info &info, const echo_msg_buff &msgbuff) {
        std::shared_ptr<StakeVoteBase> stakeVotePtr = MsgMgr::getInstance()->getVoterSys(BlockHeader::num_from_id(info.echo.blockId));
        ULTRAIN_ASSERT(stakeVotePtr, chain::chain_exception, "stakeVotePtr is null");
        int priority = stakeVotePtr->proposerPriority(info.echo.proposer);
        for (auto itor = msgbuff.begin(); itor != msgbuff.end(); ++itor) {
            if (itor->second.accountPool.size() >= stakeVotePtr->getSendEchoThreshold()) {
                if (stakeVotePtr->proposerPriority(itor->second.echo.proposer) < priority) {
                    return false;
                }
            }
        }
        return true;
    }

    bool Scheduler::isMinFEcho(const echo_message_info &info) {
        std::shared_ptr<StakeVoteBase> stakeVotePtr = MsgMgr::getInstance()->getVoterSys(BlockHeader::num_from_id(info.echo.blockId));
        ULTRAIN_ASSERT(stakeVotePtr, chain::chain_exception, "stakeVotePtr is null");
        int priority = stakeVotePtr->proposerPriority(info.echo.proposer);
        for (auto itor = m_echoMsgMap.begin(); itor != m_echoMsgMap.end(); ++itor) {
            if (itor->second.accountPool.size() >= stakeVotePtr->getSendEchoThreshold()) {
                if (stakeVotePtr->proposerPriority(itor->second.echo.proposer) < priority) {
                    return false;
                }
            }
        }
        return true;
    }

    bool Scheduler::isMinEcho(const echo_message_info &info, const echo_msg_buff &msgbuff) {
        std::shared_ptr<StakeVoteBase> stakeVotePtr = MsgMgr::getInstance()->getVoterSys(BlockHeader::num_from_id(info.echo.blockId));
        ULTRAIN_ASSERT(stakeVotePtr, chain::chain_exception, "stakeVotePtr is null");
        int priority = stakeVotePtr->proposerPriority(info.echo.proposer);
        for (auto itor = msgbuff.begin(); itor != msgbuff.end(); ++itor) {
            if (stakeVotePtr->proposerPriority(itor->second.echo.proposer) < priority) {
                return false;
            }
        }
        return true;
    }

    bool Scheduler::isMinEcho(const echo_message_info &info) {
        std::shared_ptr<StakeVoteBase> stakeVotePtr = MsgMgr::getInstance()->getVoterSys(BlockHeader::num_from_id(info.echo.blockId));
        ULTRAIN_ASSERT(stakeVotePtr, chain::chain_exception, "stakeVotePtr is null");
        int priority = stakeVotePtr->proposerPriority(info.echo.proposer);
        for (auto itor = m_echoMsgMap.begin(); itor != m_echoMsgMap.end(); ++itor) {
            if (stakeVotePtr->proposerPriority(itor->second.echo.proposer) < priority) {
                return false;
            }
        }
        return true;
    }

    size_t Scheduler::runScheduledTrxs(const std::vector<chain::transaction_id_type> &trxs,
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
                auto trace = chain.push_scheduled_transaction(trx, deadline);
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
                    if (m_initTrxCount % (Config::s_maxPhaseSeconds * 100) == 0 && fc::time_point::now() > hard_cpu_deadline) {
                        ilog("-----runScheduledTrxs code exec exceeds the hard cpu deadline, break");
                        break;
                    }
                    if (m_initTrxCount >= chain::config::default_max_propose_trx_count) {
                        break;
                    }
                }
            }FC_LOG_AND_DROP();
        }
        return count;
    }

    size_t Scheduler::runUnappliedTrxs(const std::vector<chain::transaction_metadata_ptr> &trxs,
                                              fc::time_point hard_cpu_deadline,
                                              fc::time_point block_time) {
        chain::controller &chain = app().get_plugin<chain_plugin>().chain();
        const auto& cfg = chain.get_global_properties().configuration;
        const auto& max_trx_cpu = cfg.max_transaction_cpu_usage;
        ilog("------- start running unapplied ${num} trxs", ("num", trxs.size()));
        size_t count = 0;
        for (const auto &trx : trxs) {
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
                if (m_initTrxCount % (Config::s_maxPhaseSeconds * 100) == 0 && fc::time_point::now() > hard_cpu_deadline) {
                    ilog("----- code exec exceeds the hard cpu deadline, break");
                    break;
                }
                if (m_initTrxCount >= chain::config::default_max_propose_trx_count) {
                    break;
                }
            } FC_LOG_AND_DROP();
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
            const auto &trx = trxs->front();
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
                if (m_initTrxCount % (Config::s_maxPhaseSeconds * 100) == 0 && fc::time_point::now() > hard_cpu_deadline) {
                    ilog("----- code exec exceeds the hard cpu deadline, break");
                    break;
                }
                if (m_initTrxCount >= chain::config::default_max_propose_trx_count) {
                    break;
                }
            } FC_LOG_AND_DROP();
        }
        return count;
    }

    bool Scheduler::initProposeMsg(ProposeMsg *propose_msg) {
        auto &block = propose_msg->block;
        auto start_timestamp = fc::time_point::now();
        chain::controller &chain = app().get_plugin<chain_plugin>().chain();
        size_t count2 = 0,count3 = 0;
  //      uint32_t trx_run_time = 3'000'000 * (Config::s_maxPhaseSeconds/5 +0.1*(Config::s_maxPhaseSeconds%5));
        try {
            if (!chain.pending_block_state()) {
                auto block_timestamp = chain.get_proper_next_block_timestamp();
                ilog("initProposeMsg: start block at ${time} and block_timestamp is ${timestamp}",
                     ("time", fc::time_point::now())("timestamp", block_timestamp));
                chain.start_block(block_timestamp, getCommitteeMroot(chain.head_block_num() + 1));
            }

            // TODO(yufengshen): We have to cap the block size, cpu/net resource when packing a block.
            // Refer to the subjective and exhausted design.
            std::list<chain::transaction_metadata_ptr> *pending_trxs = chain.get_pending_transactions();
            const auto &unapplied_trxs = chain.get_unapplied_transactions();
            const auto &scheduled_trxs = chain.get_scheduled_transactions();

            m_initTrxCount = 0;
            auto block_time = chain.pending_block_state()->header.timestamp.to_time_point();
            // There is case where cpu block limits can't handle, which is when there are huge number
            // of pending trxs that are all from the same user but the user has used up his cpu resources
            // and keep failing the trx execution; so we still need the hard cpu deadline to handle this.
            fc::time_point hard_cpu_deadline =
                fc::time_point::now() + fc::microseconds(Config::s_maxTrxMicroSeconds);/*can change in conig file*/
            size_t count1 = runPendingTrxs(pending_trxs, hard_cpu_deadline, block_time);
            if(fc::time_point::now() < hard_cpu_deadline)
            {
                 count2 = runUnappliedTrxs(unapplied_trxs, hard_cpu_deadline, block_time);
            }
            if(fc::time_point::now() < hard_cpu_deadline)
            {
                 count3 = runScheduledTrxs(scheduled_trxs, hard_cpu_deadline, block_time);
            }

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
            // Construct the block msg from pbs.
            const auto &pbs = chain.pending_block_state();
            FC_ASSERT(pbs, "pending_block_state does not exist but it should, another plugin may have corrupted it");
            const auto &bh = pbs->header;
            block.timestamp = bh.timestamp;
            block.proposer = StakeVoteBase::getMyAccount();
            block.version = 0;
            block.previous = bh.previous;
            block.transaction_mroot = bh.transaction_mroot;
            block.action_mroot = bh.action_mroot;
            block.transactions = pbs->block->transactions;
            block.committee_mroot = bh.committee_mroot;
            block.signature = std::string(Signer::sign<BlockHeader>(block, StakeVoteBase::getMyPrivateKey()));
            ilog("-------- propose a block, trx num ${num} proposer ${proposer} block signature ${signature} committee mroot ${mroot}",
                 ("num", block.transactions.size())
                 ("proposer", std::string(block.proposer))
                 ("signature", block.signature)
                 ("mroot", block.committee_mroot)
                 );
            /*
              ilog("----------propose block current header is ${t} ${p} ${pk} ${pf} ${v} ${prv} ${ma} ${mt} ${id}",
              ("t", block.timestamp)
              ("pk", block.proposerPk)
              ("pf", block.proposerProof)
              ("v", block.version)
              ("prv", block.previous)
              ("ma", block.transaction_mroot)
              ("mt", block.action_mroot)
              ("id", block.id()));
            */
            // Need to sign this block
            /*
              auto signature_provider_itr = _signature_providers.find( pbs->block_signing_key );
              FC_ASSERT(signature_provider_itr != _signature_providers.end(), "Attempting to produce a block for which we don't have the private key");
              //idump( (fc::time_point::now() - chain.pending_block_time()) );
            */
        } catch (const fc::exception &e) {
            edump((e.to_detail_string()));
            chain.abort_block();
            throw;
        }
        return true;
    }

    void Scheduler::processCache(const msgkey &msg_key) {
        auto propose_itor = m_cacheProposeMsgMap.find(msg_key);
        if (propose_itor != m_cacheProposeMsgMap.end()) {
            dlog("cache propose msg num = ${num}. blockNum = ${id}, phase = ${phase}",
                 ("num", propose_itor->second.size())
                         ("id", msg_key.blockNum)("phase", msg_key.phase));
            for (auto &propose : propose_itor->second) {
                handleMessage(propose);
            }
            m_cacheProposeMsgMap.erase(propose_itor);
        }

        auto echo_itor = m_cacheEchoMsgMap.find(msg_key);
        if (echo_itor != m_cacheEchoMsgMap.end()) {
            dlog("cache echo msg num = ${num}. blockNum = ${id}, phase = ${phase}", ("num", echo_itor->second.size())
                    ("id", msg_key.blockNum)("phase", msg_key.phase));
            for (auto &echo : echo_itor->second) {
                handleMessage(echo);
            }
            m_cacheEchoMsgMap.erase(echo_itor);
        }
    }

    void Scheduler::fastProcessCache(const msgkey &msg_key) {
        auto propose_itor = m_cacheProposeMsgMap.find(msg_key);

        resetTimestamp();

        if (propose_itor != m_cacheProposeMsgMap.end()) {
            dlog("fastProcessCache. cache propose msg num = ${num}. blockNum = ${id}, phase = ${phase}",
                 ("num", propose_itor->second.size())
                         ("id", msg_key.blockNum)("phase", msg_key.phase));
            for (auto &propose : propose_itor->second) {
                fastHandleMessage(propose);
            }
            m_cacheProposeMsgMap.erase(propose_itor);
        }

        auto echo_itor = m_cacheEchoMsgMap.find(msg_key);
        if (echo_itor != m_cacheEchoMsgMap.end()) {
            dlog("fastProcessCache. cache echo msg num = ${num}. blockNum = ${id}, phase = ${phase}", ("num", echo_itor->second.size())
                    ("id", msg_key.blockNum)("phase", msg_key.phase));
            for (auto &echo : echo_itor->second) {
                fastHandleMessage(echo);
            }
            m_cacheEchoMsgMap.erase(echo_itor);
        }
    }

    bool Scheduler::findEchoCache(const msgkey &msg_key) {
        auto echo_itor = m_cacheEchoMsgMap.find(msg_key);
        if (echo_itor != m_cacheEchoMsgMap.end()) {
            return true;
        }
        return false;
    }

    bool Scheduler::isFastba0(const msgkey &msg_key) {
        auto echo_itor = m_cacheEchoMsgMap.find(msg_key);
        if (echo_itor != m_cacheEchoMsgMap.end()) {
            if (echo_itor->second.size() > THRESHOLD_SYNCING) {
                return true;
            }
        }

        return false;
    }

    bool Scheduler::findProposeCache(const msgkey &msg_key) {
        auto echo_itor = m_cacheProposeMsgMap.find(msg_key);
        if (echo_itor != m_cacheProposeMsgMap.end()) {
            return true;
        }
        return false;
    }

    Block Scheduler::produceBaxBlock() {
        echo_message_info *echo_info = nullptr;
        std::shared_ptr<StakeVoteBase> stakeVotePtr = MsgMgr::getInstance()->getVoterSys(UranusNode::getInstance()->getBlockNum());
        ULTRAIN_ASSERT(stakeVotePtr, chain::chain_exception, "stakeVotePtr is null");
        int min_priority = stakeVotePtr->getProposerNumber();

        dlog("produceBaxBlock begin.");

        for (auto map_itor = m_echoMsgAllPhase.begin(); map_itor != m_echoMsgAllPhase.end(); ++map_itor) {
            min_priority = std::numeric_limits<uint32_t>::max();
            echo_info = nullptr;

            if ((UranusNode::getInstance()->getPhase() + UranusNode::getInstance()->getBaxCount()) >= Config::kMaxBaxCount) {
                if (map_itor->first.phase < Config::kMaxBaxCount) {
                    continue;
                }
            }

            echo_msg_buff &echo_msg_map = map_itor->second;
            for (auto echo_itor = echo_msg_map.begin(); echo_itor != echo_msg_map.end(); ++echo_itor) {
//                if (((echo_itor->second.totalVoter >= THRESHOLD_NEXT_ROUND) && (map_itor->first.phase < Config::kMaxBaxCount))
//                    || ((echo_itor->second.totalVoter >= THRESHOLD_EMPTY_BLOCK) && (map_itor->first.phase >= Config::kMaxBaxCount))) {
                if (isMin2FEcho(echo_itor->second.getTotalVoterWeight(), map_itor->first.phase)) {
                    dlog("found >= 2f + 1 echo. blocknum = ${blocknum} phase = ${phase}",
                         ("blocknum",map_itor->first.blockNum)("phase",map_itor->first.phase));
                    int priority = stakeVotePtr->proposerPriority(echo_itor->second.echo.proposer);
                    if (min_priority >= priority) {
                        dlog("min proof change.");
                        echo_info = &(echo_itor->second);
                        min_priority = priority;
                    }
                }
            }

            if (!echo_info || isBlank(echo_info->echo.blockId)) {
                continue;
            }

            dlog("produceBaxBlock.min_hash = ${hash}",("hash",echo_info->echo.blockId));

            if (isEmpty(echo_info->echo.blockId)) {
                dlog("produceBaxBlock.produce empty Block");
                return emptyBlock();
            }
            auto propose_itor = m_proposerMsgMap.find(echo_info->echo.blockId);
            if (propose_itor != m_proposerMsgMap.end()
                    && stakeVotePtr->proposerPriority(propose_itor->second.block.proposer) == min_priority) {
                dlog("produceBaxBlock.find propose msg ok. blocknum = ${blocknum} phase = ${phase}",
                     ("blocknum",map_itor->first.blockNum)("phase",map_itor->first.phase));
                return propose_itor->second.block;
            }
            dlog("produceBaxBlock.> 2f + 1 echo. hash = ${hash} can not find it's propose.",("hash",echo_info->echo.blockId));
        }

        return Block();
    }

    /**
     *
     * @return
     * empty block or normal block when ba0 while other phase may return blank, empty, normal block.
     */
    Block Scheduler::produceTentativeBlock() {
        BlockIdType minBlockId = BlockIdType();
        uint32_t phase = UranusNode::getInstance()->getPhase();
        phase += UranusNode::getInstance()->getBaxCount();

        std::shared_ptr<StakeVoteBase> stakeVotePtr = MsgMgr::getInstance()->getVoterSys(UranusNode::getInstance()->getBlockNum());
        ULTRAIN_ASSERT(stakeVotePtr, chain::chain_exception, "stakeVotePtr is null");
        int minPriority = stakeVotePtr->getProposerNumber();

        for (auto echo_itor = m_echoMsgMap.begin(); echo_itor != m_echoMsgMap.end(); ++echo_itor) {
            dlog("finish display_echo. phase = ${phase} size = ${size} totalVoter = ${totalVoter} block_hash : ${block_hash}",
                 ("phase", (uint32_t) echo_itor->second.echo.phase)("size", echo_itor->second.accountPool.size())(
                         "totalVoter", echo_itor->second.getTotalVoterWeight())("block_hash", echo_itor->second.echo.blockId));

//            if (((echo_itor->second.totalVoter >= THRESHOLD_NEXT_ROUND) && (phase < Config::kMaxBaxCount))
//                || ((echo_itor->second.totalVoter >= THRESHOLD_EMPTY_BLOCK) && (phase >= Config::kMaxBaxCount))) {
            if (isMin2FEcho(echo_itor->second.getTotalVoterWeight(), phase)) {
                dlog("found >= 2f + 1 echo, phase+cnt = ${phase}",("phase",phase));
                int priority = stakeVotePtr->proposerPriority(echo_itor->second.echo.proposer);
                if (minPriority >= priority) {
                    minBlockId = echo_itor->second.echo.blockId;
                    minPriority = priority;
                }
            }
        }

        if (minBlockId == BlockIdType()) { // not found > 2f + 1 echo
            dlog("can not find >= 2f + 1");
            if (UranusNode::getInstance()->getPhase() == kPhaseBA0) {
                return emptyBlock();
            }
            if ((!m_echoMsgAllPhase.empty()) && (UranusNode::getInstance()->getPhase() == kPhaseBAX)) {
                dlog("current blockhash is empty. into produceBaxBlock.");
                return produceBaxBlock();
            }
            return blankBlock();
        }
        if (minBlockId == emptyBlock().id()) {
            dlog("produce empty Block");
            return emptyBlock();
        }
        auto propose_itor = m_proposerMsgMap.find(minBlockId);
        if (propose_itor != m_proposerMsgMap.end()) {
            dlog("find propose msg ok.");
            return propose_itor->second.block;
        }
        dlog("> 2f + 1 echo ${hash} can not find it's propose.", ("hash", minBlockId));
        if (kPhaseBA0 == UranusNode::getInstance()->getPhase()) {
            dlog("produce empty Block");
            return emptyBlock();
        }
        return blankBlock();
    }

    echo_message_info Scheduler::findEchoMsg(BlockIdType blockId) {
        std::shared_ptr<StakeVoteBase> stakeVotePtr = MsgMgr::getInstance()->getVoterSys(BlockHeader::num_from_id(blockId));
        ULTRAIN_ASSERT(stakeVotePtr, chain::chain_exception, "stakeVotePtr is null");
        auto itor = m_echoMsgMap.find(blockId);
        if (itor != m_echoMsgMap.end() && itor->second.accountPool.size() >= stakeVotePtr->getNextRoundThreshold()) {
            return itor->second;
        }
        for (auto allPhaseItor = m_echoMsgAllPhase.begin(); allPhaseItor != m_echoMsgAllPhase.end(); ++allPhaseItor) {
            auto itor = allPhaseItor->second.find(blockId);
            if (itor != allPhaseItor->second.end() && itor->second.accountPool.size() >= stakeVotePtr->getNextRoundThreshold()) {
                return itor->second;
            }
        }
        return echo_message_info();
    }

    std::shared_ptr<AggEchoMsg> Scheduler::generateAggEchoMsg(std::shared_ptr<Block> blockPtr) {
        BlockIdType blockId = blockPtr->id();
        echo_message_info echoMessageInfo = findEchoMsg(blockId);
        if (echoMessageInfo.empty()) { // May be empty when sync block
            return nullptr;
        }
        std::shared_ptr<AggEchoMsg> aggEchoMsgPtr = std::make_shared<AggEchoMsg>();
        aggEchoMsgPtr->blockId = blockId;
        aggEchoMsgPtr->account = StakeVoteBase::getMyAccount();
        aggEchoMsgPtr->accountPool = echoMessageInfo.accountPool;
        aggEchoMsgPtr->sigPool = echoMessageInfo.sigPool;
        aggEchoMsgPtr->timePool = echoMessageInfo.timePool;
        aggEchoMsgPtr->phase = UranusNode::getInstance()->getPhase();
        aggEchoMsgPtr->baxCount = UranusNode::getInstance()->getBaxCount();
        aggEchoMsgPtr->signature = std::string(Signer::sign<UnsignedAggEchoMsg>(*aggEchoMsgPtr, StakeVoteBase::getMyPrivateKey()));
        return aggEchoMsgPtr;
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

        chain.abort_block();

        ilog("---- Scheduler::verifyBa0Block with trx number ${count}",
             ("count", block.transactions.size()));
        // Here is the hack, we are actually using the template of ba0_block, but we don't use
        // chain's push_block, so we have to copy some members of ba0_block into the head state,
        // e.g. pk, proof, producer.
        chain.start_block(block.timestamp, getCommitteeMroot(chain.head_block_num() + 1));
        chain::block_state_ptr pbs = chain.pending_block_state_hack();
        chain::signed_block_ptr bp = pbs->block;
        chain::signed_block_header *hp = &(pbs->header);
        // TODO(yufengshen): Move all this into start_block() to remove dup codes.
        bp->proposer = block.proposer;
        hp->proposer = block.proposer;
        auto start_timestamp = fc::time_point::now();
        try {
            for (int i = 0; i < block.transactions.size(); i++) {
                const auto &receipt = block.transactions[i];
                chain::transaction_trace_ptr trace;
                // Malicious producer setting wrong cpu_usage_us.
                ULTRAIN_ASSERT(receipt.cpu_usage_us >= cfg.min_transaction_cpu_usage,
                               chain::block_trx_min_cpu_usage_exception,
                               "trx in proposed block has wrong cpu_usage_us set ${n}",
                               ("n", receipt.cpu_usage_us));
                // This passed in deadline is used to guard for non-stopping while loop.
                auto max_cpu_usage = fc::microseconds(cfg.max_transaction_cpu_usage);
                auto max_deadline = fc::time_point::now() + max_cpu_usage;
                if (receipt.trx.contains<chain::packed_transaction>()) {
                    auto &pt = receipt.trx.get<chain::packed_transaction>();
                    auto mtrx = std::make_shared<chain::transaction_metadata>(pt);
                    trace = chain.push_transaction(mtrx, max_deadline, receipt.cpu_usage_us);
                } else if (receipt.trx.contains<chain::transaction_id_type>()) {
                    trace = chain.push_scheduled_transaction(receipt.trx.get<chain::transaction_id_type>(),
                                                             max_deadline, receipt.cpu_usage_us);
                }
                if (trace && trace->except) {
                    // So we can terminate early
                    throw *trace->except;
                }
                if (i % 100 == 0 &&
                    (fc::time_point::now() - start_timestamp) > fc::seconds(5)) {
                    ilog("----- voter code exec exceeds the max allowed time, break");
                    chain.abort_block();
                    return false;
                }
            }
            chain.set_action_merkle_hack();
            chain.set_trx_merkle_hack();
            ULTRAIN_ASSERT(pbs->header.action_mroot == block.action_mroot,
                           chain::chain_exception,
                           "Verify Ba0 block not generating expected action_mroot");
            ULTRAIN_ASSERT(pbs->header.transaction_mroot == block.transaction_mroot,
                           chain::chain_exception,
                           "Verify Ba0 block not generating expected transaction_mroot");
        } catch (const fc::exception &e) {
            edump((e.to_detail_string()));
            chain.abort_block();
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
            chain.start_block(block.timestamp, getCommitteeMroot(chain.head_block_num() + 1));
            chain::block_state_ptr pbs = chain.pending_block_state_hack();
            chain::signed_block_ptr bp = pbs->block;
            chain::signed_block_header *hp = &(pbs->header);
            bp->proposer = block.proposer;
            hp->proposer = block.proposer;
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
                }

                if (trace && trace->except) {
                    // So we can terminate early
                    throw *trace->except;
                }
            }
        } catch (const fc::exception &e) {
            edump((e.to_detail_string()));
            chain.abort_block();
            m_currentPreRunBa0TrxIndex = -1;
            return false;
        }

        return true;
    }

    void Scheduler::produceBlock(const chain::signed_block_ptr &block, bool force_push_whole_block) {
        chain::controller &chain = appbase::app().get_plugin<chain_plugin>().chain();
        uint32_t last_num = getLastBlocknum();

        dlog("produceBlock. m_proposerMsgMap = ${size}", ("size", m_proposerMsgMap.size()));
        dlog("produceBlock. m_echoMsgMap = ${size}", ("size", m_echoMsgMap.size()));
        dlog("produceBlock. m_cacheProposeMsgMap = ${size}", ("size", m_cacheProposeMsgMap.size()));
        dlog("produceBlock. m_cacheEchoMsgMap = ${size}", ("size", m_cacheEchoMsgMap.size()));
        dlog("produceBlock. m_echoMsgAllPhase = ${size}", ("size", m_echoMsgAllPhase.size()));

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

        if (pbs && m_voterPreRunBa0InProgress && !force_push_whole_block) {
            ULTRAIN_ASSERT(m_currentPreRunBa0TrxIndex == -1,
                           chain::chain_exception,
                           "Voter wont' have ba0 pre-run");
            // first check if ba1 block is indeed ba0 block.
            const chain::signed_block &b = m_ba0Block;
            if (IsBa0TheRightBlock(b, block)) {
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
            // first check if ba1 block is indeed ba0 block.
            const chain::signed_block &b = m_ba0Block;
            /*
            ilog("------ compare ${pk1} ${pk2}, ${pf1} ${pf2}, ${num1} ${num2}, ${t1} ${t2}, ${s1} ${s2}",
                 ("pk1", b.proposerPk)("pk2", block->proposerPk)
                 ("pf1", b.proposerProof)("pf2", block->proposerProof)
                 ("num1", b.block_num())("num2", block->block_num())
                 ("t1", b.timestamp)("t2", block->timestamp)
                 ("s2", block->producer));
            */
            if (IsBa0TheRightBlock(b, block)) {
                ilog("------ Finish pre-running ba0 block from ${count}", ("count", m_currentPreRunBa0TrxIndex));
                try {
                    for (; m_currentPreRunBa0TrxIndex < b.transactions.size(); m_currentPreRunBa0TrxIndex++) {
                        const auto &receipt = b.transactions[m_currentPreRunBa0TrxIndex];
                        if (receipt.trx.contains<chain::packed_transaction>()) {
                            auto &pt = receipt.trx.get<chain::packed_transaction>();
                            auto mtrx = std::make_shared<chain::transaction_metadata>(pt);
                            // Now set the deadline to infinity is fine since voter has already voted for this one.
                            chain.push_transaction(mtrx, fc::time_point::maximum(), receipt.cpu_usage_us);
                        } else if (receipt.trx.contains<chain::transaction_id_type>()) {
                            chain.push_scheduled_transaction(receipt.trx.get<chain::transaction_id_type>(),
                                                             fc::time_point::maximum(), receipt.cpu_usage_us);
                        }
                    }

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
            if (UranusNode::getInstance()->getNonProducingNode()) {
                chain.set_emit_signal();
                chain.start_receive_event();
            }
            chain.push_block(block);
            if (UranusNode::getInstance()->getNonProducingNode()) {
                chain.clear_emit_signal();
                chain.notify_event();
                chain.stop_receive_event();
            }
        }

        m_currentPreRunBa0TrxIndex = -1;
        m_voterPreRunBa0InProgress = false;

        clearTrxQueue();

        chain::block_state_ptr new_bs = chain.head_block_state();
        if (MsgMgr::getInstance()->isProposer(block->block_num())) {
            std::shared_ptr<AggEchoMsg> agg_echo = generateAggEchoMsg(block);
            if (agg_echo) {
                MsgMgr::getInstance()->insert(agg_echo);
            }
        }
        ilog("-----------produceBlock timestamp ${timestamp} block num ${num} id ${id} trx count ${count}",
             ("timestamp", block->timestamp)
             ("num", block->block_num())
             ("id", block->id())
             ("count", new_bs->block->transactions.size()));
        MsgMgr::getInstance()->moveToNewStep(UranusNode::getInstance()->getBlockNum(), kPhaseBA0, 0);
    }

    void Scheduler::clearTrxQueue() {
        chain::controller &chain = appbase::app().get_plugin<chain_plugin>().chain();

        // TODO(yufengshen):
        // Non-producing node has no chance to run through unapplied trxs
        // so as to eliminate invalid/expired trxs and this could lead to
        // memleak, so lets just clear them here. This might drop some trxs.
        // So maybe better to find a timing to re-run these trx and send
        // valid and unexpired trx to other nodes.
        if (UranusNode::getInstance()->getNonProducingNode()) {
            chain.clear_unapplied_transaction();
        }

        std::list<chain::transaction_metadata_ptr> *pending_trxs = chain.get_pending_transactions();
        auto block_time = chain.head_block_state()->header.timestamp.to_time_point();
        auto it = pending_trxs->begin();
        while (it != pending_trxs->end()) {
            if (chain.is_known_unexpired_transaction((*it)->id) ||
                fc::time_point((*it)->trx.expiration) < block_time) {
                chain.drop_unapplied_transaction(*it);
                chain.drop_pending_transaction_from_set(*it);
                it = pending_trxs->erase(it);
            } else {
                it++;
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

    ultrainio::chain::block_id_type Scheduler::getPreviousBlockhash() {
        const chain::controller &chain = appbase::app().get_plugin<chain_plugin>().chain();
        return chain.head_block_id();
    }

    void Scheduler::moveEchoMsg2AllPhaseMap() {
        if (m_echoMsgAllPhase.size() >= m_maxCachedAllPhaseKeys) {
            wlog("echo all phase msgs exceeds ${max} blockNum: ${b} phase: ${p} baxcount: ${bx}",
                 ("max", m_maxCachedAllPhaseKeys)
                 ("b", UranusNode::getInstance()->getBlockNum())
                 ("p", (int)UranusNode::getInstance()->getPhase())
                 ("bx", UranusNode::getInstance()->getBaxCount()));
            return;
        }

        msgkey msg_key;
        msg_key.blockNum = UranusNode::getInstance()->getBlockNum();
        msg_key.phase = UranusNode::getInstance()->getPhase();
        msg_key.phase += UranusNode::getInstance()->getBaxCount();
        m_echoMsgAllPhase.insert(make_pair(msg_key, std::move(m_echoMsgMap)));
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
                    UranusNode::getInstance()->sendMessage(it->peerAddr, sync_block);
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
        return m_fast_timestamp;
    }

    void Scheduler::resetTimestamp() {
        m_fast_timestamp = 0;
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
        // TODO: please find a way to cache the empty block's id
        return emptyBlock().id() == blockId;
    }

    bool Scheduler::isBlank(const BlockIdType& blockId) {
        return Block().id() == blockId;
    }

    std::shared_ptr<Block> Scheduler::generateEmptyBlock() {
        chain::controller &chain = appbase::app().get_plugin<chain_plugin>().chain();
        chain.abort_block();
        // get_proper_next_block_timestamp() probably can't be used here, we have to be
        // deterministic about the empty block's timestamp.
        auto block_timestamp = chain.head_block_time() + fc::milliseconds(chain::config::block_interval_ms);
        chain.start_block(block_timestamp, getCommitteeMroot(chain.head_block_num() + 1));
        chain.set_action_merkle_hack();
        // empty block does not have trx, so we don't need this?
        chain.set_trx_merkle_hack();
        std::shared_ptr<Block> blockPtr = std::make_shared<Block>();
        const auto &pbs = chain.pending_block_state();
        const auto &bh = pbs->header;
        blockPtr->timestamp = bh.timestamp;
        blockPtr->previous = bh.previous;
        blockPtr->previous = bh.previous;
        blockPtr->transaction_mroot = bh.transaction_mroot;
        blockPtr->action_mroot = bh.action_mroot;
        blockPtr->committee_mroot = bh.committee_mroot;
        // Discard the temp block.
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
        static std::shared_ptr<Block> emptyBlock = nullptr;
        if (!emptyBlock || emptyBlock->block_num() != UranusNode::getInstance()->getBlockNum()) {
            emptyBlock = generateEmptyBlock();
        }
        return *emptyBlock;
    }

    void Scheduler::insertAccount(echo_message_info &info, const EchoMsg &echo) {
        auto pkItor = std::find(info.accountPool.begin(), info.accountPool.end(), echo.account);
        if (pkItor == info.accountPool.end()) {
            info.accountPool.push_back(echo.account);
        }
        return;
    }

    void Scheduler::enableEventRegister(bool v) {
        chain::controller &chain = appbase::app().get_plugin<chain_plugin>().chain();
        chain.enable_event_register(v);
    }

    bool Scheduler::isDuplicate(const ProposeMsg& proposeMsg) {
        auto itor = m_proposerMsgMap.find(proposeMsg.block.id());
        return itor != m_proposerMsgMap.end();
    }
}  // namespace ultrainio
