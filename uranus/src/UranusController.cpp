#include <uranus/UranusController.h>

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
#include <ultrainio/chain/controller.hpp>
#include <ultrainio/chain/exceptions.hpp>
#include <ultrainio/chain/global_property_object.hpp>
#include <ultrainio/chain/name.hpp>
#include <ultrainio/chain/exceptions.hpp>

#include <uranus/MessageManager.h>
#include <uranus/Node.h>
#include <log/Log.h>
#include <appbase/application.hpp>

using namespace boost::asio;
using namespace std;
using namespace appbase;

namespace fc {
    extern std::unordered_map<std::string, logger> &get_logger_map();
}

namespace {
    const fc::string logger_name("UranusController");
    fc::logger _log;

    // TODO(shenyufeng) need more precise compare
    bool IsBa0TheRightBlock(const ultrainio::chain::signed_block &ba0_block,
                            const ultrainio::chain::signed_block_ptr &block) {
        return  (ba0_block.proposerPk == block->proposerPk &&
                 ba0_block.proposerProof == block->proposerProof &&
                 ba0_block.timestamp == block->timestamp &&
                 ba0_block.transaction_mroot == block->transaction_mroot &&
                 ba0_block.action_mroot == block->action_mroot &&
                 ba0_block.previous == block->previous);
    }
}

namespace ultrainio {

    std::string UranusController::signature(const EchoMsg &echo) {
        fc::sha256 echoSHA256 = fc::sha256::hash(echo);
        uint8_t signature[VRF_PROOF_LEN];
        Vrf::prove(signature, (const uint8_t *) (echoSHA256.data()), echoSHA256.data_size(),
                            UranusNode::URANUS_PRIVATE_KEY);
        return std::string((char *) (signature), VRF_PROOF_LEN);
    }

    EchoMsg UranusController::constructMsg(const Block &block) {
        EchoMsg echo;
        echo.blockHeader = block;
        echo.phase = UranusNode::getInstance()->getPhase();
        echo.baxCount = UranusNode::getInstance()->getBaxCount();
        echo.pk = std::string((char *) UranusNode::URANUS_PUBLIC_KEY, VRF_PUBLIC_KEY_LEN);
        echo.proof = std::string(
                (char *) MessageManager::getInstance()->getVoterProof(block.block_num(), echo.phase, echo.baxCount),
                VRF_PROOF_LEN);
        echo.signature = signature(echo);
        return echo;
    }

    EchoMsg UranusController::constructMsg(const ProposeMsg &propose) {
        EchoMsg echo;
        echo.blockHeader = propose.block;
        echo.phase = UranusNode::getInstance()->getPhase();
        echo.baxCount = UranusNode::getInstance()->getBaxCount();
        echo.pk = std::string((char *) UranusNode::URANUS_PUBLIC_KEY, VRF_PUBLIC_KEY_LEN);
        echo.proof = std::string(
                (char *) MessageManager::getInstance()->getVoterProof(propose.block.block_num(), echo.phase,
                                                                      echo.baxCount), VRF_PROOF_LEN);
        echo.signature = signature(echo);
        return echo;
    }

    EchoMsg UranusController::constructMsg(const EchoMsg &echo) {
        EchoMsg myEcho = echo;
        myEcho.pk = std::string((char *) UranusNode::URANUS_PUBLIC_KEY, VRF_PUBLIC_KEY_LEN);
        myEcho.proof = std::string(
                (char *) MessageManager::getInstance()->getVoterProof(echo.blockHeader.block_num(), echo.phase,
                                                                      echo.baxCount), VRF_PROOF_LEN);
        myEcho.signature = signature(echo);
        return myEcho;
    }

    UranusController::UranusController() : m_ba0Block(), m_proposerMsgMap(), m_echoMsgMap(),
                                           m_cacheProposeMsgMap(), m_cacheEchoMsgMap(),
                                           m_echoMsgAllPhase() {
        m_syncTaskPeriod = {std::chrono::seconds{1}};
        m_syncTaskTimer.reset(new boost::asio::steady_timer(app().get_io_service()));
        m_fast_timestamp = fc::time_point::now();
    }

    void UranusController::reset() {
        m_ba0Block = Block();
        m_proposerMsgMap.clear();
        m_echoMsgMap.clear();
        clearMsgCache(m_cacheProposeMsgMap, getLastBlocknum());
        clearMsgCache(m_cacheEchoMsgMap, getLastBlocknum());
        clearMsgCache(m_echoMsgAllPhase, getLastBlocknum());
    }

    void UranusController::resetEcho() {
        m_echoMsgMap.clear();
    }

    bool UranusController::insert(const EchoMsg &echo) {
        VoterSystem voter;
        int stakes = UranusNode::getInstance()->getStakes(echo.pk);

        auto itor = m_echoMsgMap.find(echo.blockHeader.id());
        if (itor != m_echoMsgMap.end()) {
            auto pkItor = std::find(itor->second.pkPool.begin(), itor->second.pkPool.end(), echo.pk);
            if (pkItor == itor->second.pkPool.end()) {
                itor->second.pkPool.push_back(echo.pk);
                itor->second.proofPool.push_back(echo.proof);
                itor->second.totalVoter += voter.vote((uint8_t *) echo.proof.data(), stakes, VoterSystem::VOTER_RATIO);
            }
        } else {
            echo_message_info echo_info;
            echo_info.echo = echo;
            echo_info.pkPool.push_back(echo.pk);
            echo_info.proofPool.push_back(echo.proof);
            echo_info.hasSend = true;
            echo_info.totalVoter = voter.vote((uint8_t *) echo.proof.data(), stakes, VoterSystem::VOTER_RATIO);
            m_echoMsgMap.insert(make_pair(echo.blockHeader.id(), echo_info));
        }
        return true;
    }

    bool UranusController::insert(const ProposeMsg &propose) {
        dlog("insert.save propose msg.blockhash = ${blockhash}", ("blockhash", propose.block.id()));
        m_proposerMsgMap.insert(make_pair(propose.block.id(), propose));
        return true;
    }

    bool UranusController::isLaterMsg(const EchoMsg &echo) {
        uint32_t currentBlockNum = UranusNode::getInstance()->getBlockNum();
        ConsensusPhase current_phase = UranusNode::getInstance()->getPhase();
        uint32_t current_bax_count = UranusNode::getInstance()->getBaxCount();

        if (echo.blockHeader.block_num() > currentBlockNum) {
            return true;
        }

        if (echo.blockHeader.block_num() == currentBlockNum) {
            if (echo.phase > current_phase) {
                return true;
            } else if ((echo.phase == current_phase) && (echo.baxCount > current_bax_count)) {
                return true;
            }
        }

        return false;
    }

    bool UranusController::isLaterMsg(const ProposeMsg &propose) {
        uint32_t currentBlockNum = UranusNode::getInstance()->getBlockNum();
        ConsensusPhase current_phase = UranusNode::getInstance()->getPhase();

        if (propose.block.block_num() > currentBlockNum) {
            return true;
        }

        if ((propose.block.block_num() == currentBlockNum)
            // Default genesis block is #1, so the first block
            // nodes are working on is #2.
            && (currentBlockNum == 2)
            && (current_phase == kPhaseInit)) {
            return true;
        }

        return false;
    }

    bool UranusController::isLaterMsgAndCache(const EchoMsg &echo, bool &duplicate) {
        duplicate = false;
        if (isLaterMsg(echo)) {
            dlog("isLaterMsgAndCache. later msg.");
            msgkey key;
            key.blockNum = echo.blockHeader.block_num();
            key.phase = echo.phase + echo.baxCount;

            auto itor = m_cacheEchoMsgMap.find(key);
            if (itor == m_cacheEchoMsgMap.end()) {
                if (m_cacheEchoMsgMap.size() >= m_maxCachedKeys) {
                    //NOTE: itor will be invalid after the operation below.
                    clearOldCachedEchoMsg();
                }
                std::vector<EchoMsg> echo_vector;
                echo_vector.push_back(echo);
                m_cacheEchoMsgMap.insert(make_pair(key, echo_vector));
            } else {
                auto id = echo.blockHeader.id();
                std::vector<EchoMsg> &ev = itor->second;
                for (size_t i = 0; i < ev.size(); i++) {
                    if (ev[i].pk == echo.pk && ev[i].blockHeader.id() == id) {
                        ilog("duplicate echo msg!!! id:${id} pk:${pk} blockNum:${b} phase:${p}",
                             ("id", echo.blockHeader.id())("pk", UltrainLog::convert2Hex(echo.pk))("b", key.blockNum)("p", key.phase));
                        duplicate = true;
                        return true;
                    }
                }
                ev.push_back(echo);
            }
            dlog("next phase echo msg. blockNum = ${id}, phase = ${phase},baxcount = ${baxcount}",
                 ("id", echo.blockHeader.block_num())("phase", (uint32_t) echo.phase)("baxcount",echo.baxCount));
            return true;
        }
        return false;
    }

    bool UranusController::isLaterMsgAndCache(const ProposeMsg &propose, bool &duplicate) {
        duplicate = false;
        if (isLaterMsg(propose)) {
            msgkey key;
            key.blockNum = propose.block.block_num();
            key.phase = kPhaseBA0;

            auto itor = m_cacheProposeMsgMap.find(key);
            if (itor == m_cacheProposeMsgMap.end()) {
                if (m_cacheProposeMsgMap.size() >= m_maxCachedKeys) {
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
                    if (pv[i].block.proposerPk == propose.block.proposerPk && pv[i].block.id() == id) {
                        ilog("duplicate propose msg!!! id:${id} pk:${pk} blockNum:${b} phase:${p}",
                             ("id", propose.block.id())("pk", propose.block.proposerPk)("b", key.blockNum)("p",
                                                                                                           key.phase));
                        duplicate = true;
                        return true;
                    }
                }
                pv.push_back(propose);
            }
            dlog("next phase propose msg. blockNum = ${id}", ("id", key.blockNum));
            return true;
        }
        return false;
    }

    bool UranusController::isBeforeMsg(const EchoMsg &echo) {
        std::string this_pk = std::string((const char *) UranusNode::URANUS_PUBLIC_KEY, VRF_PUBLIC_KEY_LEN);

        if (this_pk == echo.pk) {
            elog("loopback echo. pk : ${pk}", ("pk", this_pk));
            return false;
        }
        if (echo.blockHeader.block_num() != UranusNode::getInstance()->getBlockNum()) {
            elog("invalid echo msg . blockNum = ${id1}. local blockNum = ${id2}",
                 ("id1", echo.blockHeader.block_num())("id2", UranusNode::getInstance()->getBlockNum()));
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

    bool UranusController::processBeforeMsg(const EchoMsg &echo) {
        msgkey msg_key;
        msg_key.blockNum = echo.blockHeader.block_num();
        msg_key.phase = echo.phase + echo.baxCount;
        echo_msg_buff echo_msg_map;

        dlog("processBeforeMsg.");

        auto map_it = m_echoMsgAllPhase.find(msg_key);
        if (map_it == m_echoMsgAllPhase.end()) {
            if (m_echoMsgAllPhase.size() >= m_maxCachedAllPhaseKeys) {
                dlog("processBeforeMsg.map reach the up limit. size = ${size}",("size",m_echoMsgAllPhase.size()));
                clearOldCachedAllPhaseMsg();
            }
            auto result = m_echoMsgAllPhase.insert(make_pair(msg_key, echo_msg_map));
            map_it = result.first;
        }

        //dlog("processBeforeMsg.insert ok,cal voters begin.");

        auto itor = map_it->second.find(echo.blockHeader.id());
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
                map_it->second.insert(make_pair(echo.blockHeader.id(), info));
                if (isMinEcho(info,map_it->second) || isMinFEcho(info,map_it->second)) {
                    return true;
                }
            }
        }
        return false;
    }

    bool UranusController::updateAndMayResponse(echo_message_info &info, const EchoMsg &echo, bool response) {
        auto pkItor = std::find(info.pkPool.begin(), info.pkPool.end(), echo.pk);
        if (pkItor == info.pkPool.end()) {
            info.pkPool.push_back(echo.pk);
            info.proofPool.push_back(echo.proof);
            VoterSystem voter;
            int stakes = UranusNode::getInstance()->getStakes(echo.pk);
            info.totalVoter += voter.vote((uint8_t *) echo.proof.data(), stakes, VoterSystem::VOTER_RATIO);
            if (response && info.totalVoter >= THRESHOLD_SEND_ECHO && !info.hasSend
                && UranusNode::getInstance()->getPhase() == kPhaseBA0 && isMinFEcho(info)) {
                if (MessageManager::getInstance()->isVoter(UranusNode::getInstance()->getBlockNum(), echo.phase,
                                                           echo.baxCount)) {
                    ilog("send echo when > f + 1");
                    info.hasSend = true;
                    EchoMsg myEcho = constructMsg(echo);
                    insert(myEcho);
                    UranusNode::getInstance()->sendMessage(myEcho);
                }
            }
            //ilog("updateAndMayResponse new pk insert.");
            return true;
        }
        return false;
    }

    bool UranusController::isBeforeMsgAndProcess(const EchoMsg &echo) {
        if (isBeforeMsg(echo)) {
            return processBeforeMsg(echo);
        }

        return false;
    }

    uint32_t UranusController::isSyncing() {
        uint32_t maxBlockNum = UranusNode::getInstance()->getBlockNum();
        std::string this_pk = std::string((const char *) UranusNode::URANUS_PUBLIC_KEY, VRF_PUBLIC_KEY_LEN);

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
                    if (this_pk == echo.pk) {
                        elog("loopback echo. pk : ${pk}", ("pk", this_pk));
                        continue;
                    }

                    auto itor = echo_msg_map.find(echo.blockHeader.id());
                    if (itor != echo_msg_map.end()) {
                        updateAndMayResponse(itor->second, echo, false);
                    } else {
                        echo_message_info info;
                        info.echo = echo;
                        updateAndMayResponse(info, echo, false);
                        echo_msg_map.insert(make_pair(echo.blockHeader.id(), info));
                    }
                }

                for (auto echo_itor = echo_msg_map.begin(); echo_itor != echo_msg_map.end(); ++echo_itor) {
                    if (echo_itor->second.totalVoter >= THRESHOLD_SYNCING) {
                        maxBlockNum = vector_itor->first.blockNum;
                        break;
                    }
                }
            }
        }

        if (maxBlockNum > UranusNode::getInstance()->getBlockNum()) {
            return maxBlockNum;
        }

        return INVALID_BLOCK_NUM;
    }

    bool UranusController::isValid(const EchoMsg &echo) {
        std::string this_pk = std::string((const char *) UranusNode::URANUS_PUBLIC_KEY, VRF_PUBLIC_KEY_LEN);

        if (this_pk == echo.pk) {
            elog("loopback echo. pk : ${pk}", ("pk", this_pk));
            return false;
        }
        if (echo.blockHeader.block_num() != UranusNode::getInstance()->getBlockNum()) {
            elog("invalid echo msg . blockNum = ${id1}. local blockNum = ${id2}",
                 ("id1", echo.blockHeader.block_num())("id2", UranusNode::getInstance()->getBlockNum()));
            return false;
        }
        if (static_cast<ConsensusPhase>(echo.phase) != UranusNode::getInstance()->getPhase()) {
            elog("invalid echo msg . phase = ${phase1}. local phase = ${phase2}",
                 ("phase1", (uint32_t) echo.phase)("phase2", (uint32_t) UranusNode::getInstance()->getPhase()));
            return false;
        }
        return true;
    }

    bool UranusController::isValid(const ProposeMsg &propose) {
        std::string this_pk = std::string((const char *) UranusNode::URANUS_PUBLIC_KEY, VRF_PUBLIC_KEY_LEN);

        if (this_pk == propose.block.proposerPk) {
            elog("loopback propose. pk : ${pk}", ("pk", this_pk));
            return false;
        }
//        if (!ultrainio::Vrf::verify((const uint8_t*)propose.txs.data(), propose.txs.length(),
//                                             (const uint8_t*)propose.txs_signature.data(),
//                                             (const uint8_t*)propose.proposer_pk.data())) {
//            LOG_ERROR << "valid msg fail. txs_signature = " <<  propose.txs_signature << std::endl;
//            return false;
//        }
        if (propose.block.block_num() != UranusNode::getInstance()->getBlockNum()) {
            elog("invalid propose msg . blockNum = ${id1}. local blockNum = ${id2}",
                 ("id1", propose.block.block_num())("id2", UranusNode::getInstance()->getBlockNum()));
            return false;
        }
//        if (kPhaseBA0 != UranusNode::getInstance()->getPhase()) {
////            elog("invalid propose msg . phase = ${phase1}. local phase = ${phase2}",("phase1", (uint32_t)propose.phase)("phase2",(uint32_t)UranusNode::getInstance()->getPhase()));
////            return false;
////        }
        return true;
    }

    bool UranusController::fastHandleMessage(const ProposeMsg &propose) {
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

    bool UranusController::fastHandleMessage(const EchoMsg &echo) {
        if (!isValid(echo)) {
            return false;
        }

        auto itor = m_echoMsgMap.find(echo.blockHeader.id());
        if (itor != m_echoMsgMap.end()) {
            updateAndMayResponse(itor->second, echo, false);
        } else {
            echo_message_info info;
            info.echo = echo;
            updateAndMayResponse(info, echo, false);
            m_echoMsgMap.insert(make_pair(echo.blockHeader.id(), info));
        }
        return true;
    }

    bool UranusController::handleMessage(const ProposeMsg &propose) {
        bool duplicate = false;
        if (isLaterMsgAndCache(propose, duplicate)) {
            return (!duplicate);
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
                if (MessageManager::getInstance()->isVoter(propose.block.block_num(), kPhaseBA0, 0)) {
                    EchoMsg echo = constructMsg(propose);
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

    bool UranusController::handleMessage(const EchoMsg &echo) {
        bool duplicate = false;
        bool bret = false;

        if (isLaterMsgAndCache(echo, duplicate)) {
            return (!duplicate);
        }

        if (isBeforeMsg(echo)) {
            return processBeforeMsg(echo);
        }

        if (!isValid(echo)) {
            return false;
        }

        if ((UranusNode::getInstance()->getSyncingStatus()) && (UranusNode::getInstance()->getPhase() != kPhaseBAX)) {
            dlog("receive echo msg. node is syncing. blockhash = ${blockhash} echo'pk = ${pk}",
                 ("blockhash", echo.blockHeader.id())("pk", UltrainLog::convert2Hex(echo.pk)));
            return true;
        }

        dlog("receive echo msg.blockhash = ${blockhash} echo'pk = ${pk}",
             ("blockhash", echo.blockHeader.id())("pk", UltrainLog::convert2Hex(echo.pk)));
        auto itor = m_echoMsgMap.find(echo.blockHeader.id());
        if (itor != m_echoMsgMap.end()) {
            bret = updateAndMayResponse(itor->second, echo, true);
            if ((isMinEcho(itor->second) || isMinFEcho(itor->second)) && bret) {
                return true;
            }
        } else {
            echo_message_info info;
            info.echo = echo;
            bret = updateAndMayResponse(info, echo, true);
            m_echoMsgMap.insert(make_pair(echo.blockHeader.id(), info));
            if ((isMinEcho(info) || isMinFEcho(itor->second)) && bret) {
                return true;
            }
        }
        return false;
    }

    bool UranusController::handleMessage(const string &peer_addr, const SyncRequestMessage &msg) {
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

        for (; num <= end_block_num && send_count < max_count; num++, send_count++) {
            auto b = chain.fetch_block_by_number(num);
            if (b) {
                UranusNode::getInstance()->sendMessage(peer_addr, *b);
            } else if (num == end_block_num) { // try to send last block next time
                break;
            } // else: skip the block if not exist
        }

        if (num <= end_block_num) {
            m_syncTaskQueue.emplace_back(peer_addr, num, end_block_num);
        }

        return true;
    }

    bool UranusController::handleMessage(const string &peer_addr, const ReqLastBlockNumMsg &msg) {
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

    uint32_t UranusController::getLastBlocknum() {
        const chain::controller &chain = appbase::app().get_plugin<chain_plugin>().chain();
        return chain.head_block_num();
    }

    bool UranusController::handleMessage(const Block &block) {
        uint32_t last_num = getLastBlocknum();
        ilog("@@@@@@@@@@@@@@@ last block num in local chain:${last}", ("last", last_num));
        chain::controller &chain = appbase::app().get_plugin<chain_plugin>().chain();
        auto b = chain.fetch_block_by_number(last_num);
        if (b) {
            if (block.previous == b->id()) {
                // TODO(yufengshen) -- Do not copy here, should have shared_ptr at the first place.
                produceBlock(std::make_shared<chain::signed_block>(block), true);
                return true;
            } else {
                elog("block error. block in local chain: num = ${n} hash = ${hash}", ("n", last_num)("hash", b->id()));
                elog("comming block num:${n} hash:${h} previous hash:${ph}",
                     ("n", block.block_num())("h", block.id())("ph", block.previous));
                return false;
            }
        }

        elog("Error!!! Get last block failed!!! num:${n}", ("n", last_num));
        return false;
    }

    bool UranusController::isMinPropose(const ProposeMsg &propose_msg) {
        VoterSystem voter;
        uint32_t priority = voter.proof2Priority((const uint8_t *) propose_msg.block.proposerProof.data());
        for (auto propose_itor = m_proposerMsgMap.begin(); propose_itor != m_proposerMsgMap.end(); ++propose_itor) {
            uint32_t p = voter.proof2Priority((const uint8_t *) propose_itor->second.block.proposerProof.data());
            if (p < priority) {
                return false;
            }
        }
        return true;
    }

    bool UranusController::isMinFEcho(const echo_message_info &info, const echo_msg_buff &msgbuff) {
        VoterSystem voter;
        uint32_t priority = voter.proof2Priority((const uint8_t *) info.echo.blockHeader.proposerProof.data());
        for (auto echo_itor = msgbuff.begin(); echo_itor != msgbuff.end(); ++echo_itor) {
            if (echo_itor->second.totalVoter >= THRESHOLD_SEND_ECHO) {
                if (voter.proof2Priority((const uint8_t *) echo_itor->second.echo.blockHeader.proposerProof.data()) <
                    priority) {
                    return false;
                }
            }
        }
        return true;
    }

    bool UranusController::isMinFEcho(const echo_message_info &info) {
        VoterSystem voter;
        uint32_t priority = voter.proof2Priority((const uint8_t *) info.echo.blockHeader.proposerProof.data());
        for (auto echo_itor = m_echoMsgMap.begin(); echo_itor != m_echoMsgMap.end(); ++echo_itor) {
            if (echo_itor->second.totalVoter >= THRESHOLD_SEND_ECHO) {
                if (voter.proof2Priority((const uint8_t *) echo_itor->second.echo.blockHeader.proposerProof.data()) <
                    priority) {
                    return false;
                }
            }
        }
        return true;
    }

    bool UranusController::isMinEcho(const echo_message_info &info, const echo_msg_buff &msgbuff) {
        VoterSystem voter;
        uint32_t priority = voter.proof2Priority((const uint8_t *) info.echo.blockHeader.proposerProof.data());
        for (auto echo_itor = msgbuff.begin(); echo_itor != msgbuff.end(); ++echo_itor) {
            if (voter.proof2Priority((const uint8_t *) echo_itor->second.echo.blockHeader.proposerProof.data()) <
                priority) {
                return false;
            }
        }
        return true;
    }

    bool UranusController::isMinEcho(const echo_message_info &info) {
        VoterSystem voter;
        uint32_t priority = voter.proof2Priority((const uint8_t *) info.echo.blockHeader.proposerProof.data());
        for (auto echo_itor = m_echoMsgMap.begin(); echo_itor != m_echoMsgMap.end(); ++echo_itor) {
            if (voter.proof2Priority((const uint8_t *) echo_itor->second.echo.blockHeader.proposerProof.data()) <
                priority) {
                return false;
            }
        }
        return true;
    }

    size_t UranusController::runUnappliedTrxs(const std::vector<chain::transaction_metadata_ptr> &trxs,
                                              fc::time_point start_timestamp) {
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

            // TODO -- yufengshen -- We still need this.
            //	       if (trx->packed_trx.expiration() > pbs->header.timestamp.to_time_point()) {
            // expired, drop it
            //		 ilog("-----------initProposeMsg expired trx exp ${exp}, blocktime ${bt}",
            //		      ("exp",trx->packed_trx.expiration())("bt",pbs->header.timestamp));
            //                 chain.drop_unapplied_transaction(trx);
            //                 continue;
            //	       }

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
                //  TODO(yufengshen) : Do we still need this ? will max block cpu limit already cover this ?
                //  Every 100 trxs we check if we have exceeds the allowed trx running time.
                // if (m_initTrxCount % 100 == 0 &&
                //    (fc::time_point::now() - start_timestamp) > fc::seconds(CODE_EXEC_MAX_TIME_S)) {
                //   ilog("----- code exec exceeds the max allowed time, break");
                //   break;
                //}
                if (m_initTrxCount >= MAX_PROPOSE_TRX_COUNT) {
                    break;
                }
            } FC_LOG_AND_DROP();
        }
        return count;
    }

    size_t UranusController::runPendingTrxs(std::list<chain::transaction_metadata_ptr> *trxs,
                                            fc::time_point start_timestamp) {
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
                chain.drop_unapplied_transaction(trx);
                trxs->pop_front();
                continue;
            }

            // TODO -- yufengshen -- We still need this.
            //	       if (trx->packed_trx.expiration() > pbs->header.timestamp.to_time_point()) {
            // expired, drop it
            //		 ilog("-----------initProposeMsg expired trx exp ${exp}, blocktime ${bt}",
            //		      ("exp",trx->packed_trx.expiration())("bt",pbs->header.timestamp));
            //                 chain.drop_unapplied_transaction(trx);
            //                 continue;
            //	       }

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
                trxs->pop_front();

                m_initTrxCount++;
                count++;
                //  TODO(yufengshen) : Do we still need this ? will max block cpu limit already cover this ?
                //  Every 100 trxs we check if we have exceeds the allowed trx running time.
                //                if (m_initTrxCount % 100 == 0 &&
                //                    (fc::time_point::now() - start_timestamp) > fc::seconds(CODE_EXEC_MAX_TIME_S)) {
                //                    ilog("----- code exec exceeds the max allowed time, break");
                //                    break;
                //                }
                // Do we still need this ?
                if (m_initTrxCount >= MAX_PROPOSE_TRX_COUNT) {
                    break;
                }
            } FC_LOG_AND_DROP();
        }
        return count;
    }

    bool UranusController::initProposeMsg(ProposeMsg *propose_msg) {
        auto &block = propose_msg->block;
        auto start_timestamp = fc::time_point::now();
        chain::controller &chain = app().get_plugin<chain_plugin>().chain();
        try {
            if (!chain.pending_block_state()) {
                auto block_timestamp = chain.get_proper_next_block_timestamp();
                ilog("initProposeMsg: start block at ${time} and block_timestamp is ${timestamp}",
                     ("time", fc::time_point::now())("timestamp", block_timestamp));
                chain.start_block(block_timestamp, 0);
            }

            // TODO(yufengshen): We have to cap the block size, cpu/net resource when packing a block.
            // Refer to the subjective and exhausted design.  
            std::list<chain::transaction_metadata_ptr> *pending_trxs = chain.get_pending_transactions();
            const auto &unapplied_trxs = chain.get_unapplied_transactions();

            m_initTrxCount = 0;
            size_t count1 = runPendingTrxs(pending_trxs, start_timestamp);
            size_t count2 = runUnappliedTrxs(unapplied_trxs, start_timestamp);

            // We are under very heavy pressure, lets drop transactions.
            if (m_initTrxCount >= MAX_PROPOSE_TRX_COUNT) {
                pending_trxs->clear();
                chain.clear_unapplied_transaction();
            }
            ilog("------- run ${count1} ${count2}  trxs, taking time ${time}",
                 ("count1", count1)
                         ("count2", count2)
                         ("time", fc::time_point::now() - start_timestamp));
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
            block.producer = "ultrainio";
            block.proposerPk = std::string((char *) UranusNode::URANUS_PUBLIC_KEY, VRF_PUBLIC_KEY_LEN);
            block.proposerProof = std::string(
                    (char *) MessageManager::getInstance()->getProposerProof(UranusNode::getInstance()->getBlockNum()),
                    VRF_PROOF_LEN);
            block.version = 0;
            block.confirmed = 1;
            block.previous = bh.previous;
            block.transaction_mroot = bh.transaction_mroot;
            block.action_mroot = bh.action_mroot;
            block.transactions = pbs->block->transactions;
            ilog("-------- propose a block, trx num ${num}", ("num", block.transactions.size()));

            uint8_t signature[VRF_PROOF_LEN] = {0};
            std::string blockHash = (block.id()).str();
            if (!Vrf::prove(signature, (uint8_t *) blockHash.c_str(), blockHash.length(),
                                                UranusNode::URANUS_PRIVATE_KEY)) {
                elog("vrf_prove error");
                return false;
            }
            block.signature = std::string((char *) signature, VRF_PROOF_LEN);
            /*
              ilog("----------propose block current header is ${t} ${p} ${pk} ${pf} ${v} ${c} ${prv} ${ma} ${mt} ${id}",
              ("t", block.timestamp)
              ("p", block.producer)
              ("pk", block.proposerPk)
              ("pf", block.proposerProof)
              ("v", block.version)
              ("c", block.confirmed)
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
              chain.sign_block( [&]( const digest_type& d ) {
              auto debug_logger = maybe_make_debug_time_logger();
              return signature_provider_itr->second(d);
              } );
            */
        } catch (const fc::exception &e) {
            edump((e.to_detail_string()));
            chain.abort_block();
            throw;
        }
        return true;
    }

    void UranusController::processCache(const msgkey &msg_key) {
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

    void UranusController::fastProcessCache(const msgkey &msg_key) {
        auto propose_itor = m_cacheProposeMsgMap.find(msg_key);
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

    bool UranusController::findEchoCache(const msgkey &msg_key) {
        auto echo_itor = m_cacheEchoMsgMap.find(msg_key);
        if (echo_itor != m_cacheEchoMsgMap.end()) {
            return true;
        }
        return false;
    }

    Block UranusController::produceBaxBlock() {
        VoterSystem voter;
        uint32_t min_priority = std::numeric_limits<uint32_t>::max();
        echo_message_info *echo_info = nullptr;

        dlog("produceBaxBlock begin.");

        for (auto map_itor = m_echoMsgAllPhase.begin(); map_itor != m_echoMsgAllPhase.end(); ++map_itor) {
            echo_msg_buff &echo_msg_map = map_itor->second;
            for (auto echo_itor = echo_msg_map.begin(); echo_itor != echo_msg_map.end(); ++echo_itor) {
                if (echo_itor->second.totalVoter >= THRESHOLD_NEXT_ROUND) {
                    ilog("found >= 2f + 1 echo");
                    uint32_t priority = voter.proof2Priority(
                            (const uint8_t *) echo_itor->second.echo.blockHeader.proposerProof.data());
                    if (min_priority >= priority) {
                        echo_info = &(echo_itor->second);
                        min_priority = priority;
                    }
                }
            }

            if (!echo_info || isBlank(echo_info->echo.blockHeader)) {
                continue;
            }

            if (isEmpty(echo_info->echo.blockHeader)) {
                return emptyBlock();
            }
            auto propose_itor = m_proposerMsgMap.find(echo_info->echo.blockHeader.id());
            if (propose_itor != m_proposerMsgMap.end()) {
                return propose_itor->second.block;
            }
        }

        return Block();
    }

    /**
     *
     * @return
     * empty block or normal block when ba0 while other phase may return blank, empty, normal block.
     */
    Block UranusController::produceTentativeBlock() {
        VoterSystem voter;
        uint32_t minPriority = std::numeric_limits<uint32_t>::max();
        BlockIdType minBlockId = BlockIdType();
        for (auto echo_itor = m_echoMsgMap.begin(); echo_itor != m_echoMsgMap.end(); ++echo_itor) {
            dlog("finish display_echo. phase = ${phase} size = ${size} totalVoter = ${totalVoter} txs_hash : ${txs_hash}",
                 ("phase", (uint32_t) echo_itor->second.echo.phase)("size", echo_itor->second.pkPool.size())(
                         "totalVoter", echo_itor->second.totalVoter)("txs_hash",
                                                                     echo_itor->second.echo.blockHeader.id()));
            if (echo_itor->second.totalVoter >= THRESHOLD_NEXT_ROUND) {
                ilog("found >= 2f + 1 echo");
                uint32_t priority = voter.proof2Priority(
                        (const uint8_t *) echo_itor->second.echo.blockHeader.proposerProof.data());
                if (minPriority >= priority) {
                    minBlockId = echo_itor->second.echo.blockHeader.id();
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

    bool UranusController::isProcessNow() {
        VoterSystem voter;
        uint32_t minPriority = std::numeric_limits<uint32_t>::max();
        BlockIdType minBlockId = BlockIdType();
        for (auto echo_itor = m_echoMsgMap.begin(); echo_itor != m_echoMsgMap.end(); ++echo_itor) {
            if (echo_itor->second.totalVoter >= THRESHOLD_NEXT_ROUND) {
                ilog("isProcessNow.found >= 2f + 1 echo");
                uint32_t priority = voter.proof2Priority(
                        (const uint8_t *) echo_itor->second.echo.blockHeader.proposerProof.data());
                if (minPriority >= priority) {
                    minBlockId = echo_itor->second.echo.blockHeader.id();
                    minPriority = priority;
                    break;
                }
            }
        }

        if (minBlockId == BlockIdType()) { // not found > 2f + 1 echo
            dlog("isProcessNow.can not find >= 2f + 1");
            return false;
        }

        return true;
    }

    std::shared_ptr<AggEchoMsg> UranusController::generateAggEchoMsg(std::shared_ptr<Block> blockPtr) {
        std::shared_ptr<AggEchoMsg> aggEchoMsgPtr = std::make_shared<AggEchoMsg>();
        aggEchoMsgPtr->blockHeader = *blockPtr;
        aggEchoMsgPtr->pk = std::string((char*)UranusNode::URANUS_PUBLIC_KEY, VRF_PUBLIC_KEY_LEN);
        aggEchoMsgPtr->proof = std::string((char*)MessageManager::getInstance()->getVoterProof(blockPtr->block_num(), kPhaseBA1, 0), VRF_PROOF_LEN);
        auto itor = m_echoMsgMap.find(blockPtr->id());
        aggEchoMsgPtr->pkPool = itor->second.pkPool;
        aggEchoMsgPtr->proofPool = itor->second.proofPool;
        aggEchoMsgPtr->phase = UranusNode::getInstance()->getPhase();
        aggEchoMsgPtr->baxCount = UranusNode::getInstance()->getBaxCount();
        //TODO(qinxiaofen)
        //aggEchoMsgPtr->signature
        return aggEchoMsgPtr;
    }

    void UranusController::clearPreRunStatus() {
        m_voterPreRunBa0InProgress = false;
        m_currentPreRunBa0TrxIndex = -1;
        chain::controller &chain = appbase::app().get_plugin<chain_plugin>().chain();
        chain.abort_block();
    }

    bool UranusController::verifyBa0Block() {
        chain::controller &chain = appbase::app().get_plugin<chain_plugin>().chain();
        const auto& cfg = chain.get_global_properties().configuration;
        chain.abort_block();
        const chain::signed_block &block = m_ba0Block;
        if (isBlank(block))
            return false;

        auto id = block.id();
        auto existing = chain.fetch_block_by_id(id);
        if (existing) {
            ULTRAIN_ASSERT(!existing, chain::chain_exception, "Produced block is already in the chain");
            return false;
        }
        ilog("---- UranusController::verifyBa0Block with trx number ${count}",
             ("count", block.transactions.size()));
        // Here is the hack, we are actually using the template of ba0_block, but we don't use
        // chain's push_block, so we have to copy some members of ba0_block into the head state,
        // e.g. pk, proof, producer.
        chain.start_block(block.timestamp, block.confirmed);
        chain::block_state_ptr pbs = chain.pending_block_state_hack();
        chain::signed_block_ptr bp = pbs->block;
        chain::signed_block_header *hp = &(pbs->header);
        // TODO(yufengshen): Move all this into start_block() to remove dup codes.
        bp->producer = block.producer;
        bp->proposerPk = block.proposerPk;
        bp->proposerProof = block.proposerProof;
        hp->producer = block.producer;
        hp->proposerPk = block.proposerPk;
        hp->proposerProof = block.proposerProof;
        bp->confirmed = block.confirmed;
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
        return true;
    }

    bool UranusController::preRunBa0BlockStart() {
        chain::controller &chain = appbase::app().get_plugin<chain_plugin>().chain();
        chain.abort_block();
        const chain::signed_block &block = m_ba0Block;
        if (isBlank(block) || block.transactions.empty()) {
            return false;
        }

        auto id = block.id();
        auto existing = chain.fetch_block_by_id(id);
        if (existing) {
            ULTRAIN_ASSERT(!existing, chain::chain_exception, "Produced block is already in the chain");
            return false;
        }
        ilog("---- UranusController::preRunBa0BlockStart() start block");
        // Here is the hack, we are actually using the template of ba0_block, but we don't use
        // chain's push_block, so we have to copy some members of ba0_block into the head state,
        // e.g. pk, proof, producer.
        chain.start_block(block.timestamp, block.confirmed);
        chain::block_state_ptr pbs = chain.pending_block_state_hack();
        chain::signed_block_ptr bp = pbs->block;
        chain::signed_block_header *hp = &(pbs->header);
        bp->producer = block.producer;
        bp->proposerPk = block.proposerPk;
        bp->proposerProof = block.proposerProof;
        hp->producer = block.producer;
        hp->proposerPk = block.proposerPk;
        hp->proposerProof = block.proposerProof;
        bp->confirmed = block.confirmed;
        m_currentPreRunBa0TrxIndex = 0;
        return true;
    }

    bool UranusController::preRunBa0BlockStep() {
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
                   trx_count <= 1000; m_currentPreRunBa0TrxIndex++, trx_count++) {
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

    void UranusController::produceBlock(const chain::signed_block_ptr &block, bool force_push_whole_block) {
        chain::controller &chain = appbase::app().get_plugin<chain_plugin>().chain();

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
                chain.sign_block([&](const chain::digest_type &d) { return b.producer_signature; });
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
                 ("s1", b.producer)("s2", block->producer));
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
                    chain.sign_block([&](const chain::digest_type &d) { return b.producer_signature; });
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
            chain.push_block(block);
        }
        m_currentPreRunBa0TrxIndex = -1;
        m_voterPreRunBa0InProgress = false;

        chain::block_state_ptr new_bs = chain.head_block_state();
        if (MessageManager::getInstance()->isProposer(block->block_num())) {
            MessageManager::getInstance()->insert(generateAggEchoMsg(block));
        }
        ilog("-----------produceBlock timestamp ${timestamp} block num ${num} id ${id} trx count ${count}--------------",
             ("timestamp", block->timestamp)
             ("num", block->block_num())
             ("id", block->id())
             ("count", new_bs->block->transactions.size()));
    }

    void UranusController::init() {
        m_proposerMsgMap.clear();
        m_echoMsgMap.clear();
        m_cacheProposeMsgMap.clear();
        m_cacheEchoMsgMap.clear();
        m_echoMsgAllPhase.clear();
        startSyncTaskTimer();
    }

    const Block* UranusController::getBa0Block() {
        return &m_ba0Block;
    }

    ultrainio::chain::block_id_type UranusController::getPreviousBlockhash() {
        const chain::controller &chain = appbase::app().get_plugin<chain_plugin>().chain();
        return chain.head_block_id();
    }

    void UranusController::saveEchoMsg() {
        if (m_echoMsgAllPhase.size() >= m_maxCachedAllPhaseKeys) {
            clearOldCachedAllPhaseMsg();
        }

        msgkey msg_key;
        msg_key.blockNum = UranusNode::getInstance()->getBlockNum();
        msg_key.phase = UranusNode::getInstance()->getPhase();
        msg_key.phase += UranusNode::getInstance()->getBaxCount();
        m_echoMsgAllPhase.insert(make_pair(msg_key, m_echoMsgMap));
    }

    void UranusController::startSyncTaskTimer() {
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

    void UranusController::processSyncTask() {
        if (m_syncTaskQueue.empty()) {
            return;
        }

        chain::controller &chain = appbase::app().get_plugin<chain_plugin>().chain();
        uint32_t last_num = getLastBlocknum();
        uint32_t max_count = m_maxPacketsOnce / m_syncTaskQueue.size() + 1;
        uint32_t send_count = 0;
        for (std::list<SyncTask>::iterator it = m_syncTaskQueue.begin(); it != m_syncTaskQueue.end();) {
            while (send_count < max_count && it->startBlock <= it->endBlock && it->startBlock <= last_num) {
                auto b = chain.fetch_block_by_number(it->startBlock);
                if (b) {
                    UranusNode::getInstance()->sendMessage(it->peerAddr, *b);
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
    void UranusController::clearMsgCache(T cache, uint32_t blockNum) {
        for (auto msg_it = cache.begin(); msg_it != cache.end();) {
            if (msg_it->first.blockNum <= blockNum) {
                cache.erase(msg_it++);
            } else {
                ++msg_it;
            }
        }
    }

    fc::time_point UranusController::getFastTimestamp() {
        return m_fast_timestamp;
    }

    void UranusController::resetTimestamp() {
        m_fast_timestamp = fc::time_point::now();
    }

    void UranusController::clearOldCachedProposeMsg() {
        uint32_t old_block_num = std::numeric_limits<uint32_t>::max();
        for (auto &it : m_cacheProposeMsgMap) {
            if (it.first.blockNum < old_block_num) {
                old_block_num = it.first.blockNum;
            }
        }

        wlog("m_cacheProposeMsgMap exceeds ${max}, echo msgs with block num ${num} will be cleared",
             ("max", m_maxCachedKeys)("num", old_block_num));
        for (auto it = m_cacheProposeMsgMap.begin(); it != m_cacheProposeMsgMap.end();) {
            if (it->first.blockNum == old_block_num) {
                m_cacheProposeMsgMap.erase(it++);
            } else {
                ++it;
            }
        }
    }

    void UranusController::clearOldCachedEchoMsg() {
        uint32_t old_block_num = std::numeric_limits<uint32_t>::max();
        for (auto &it : m_cacheEchoMsgMap) {
            if (it.first.blockNum < old_block_num) {
                old_block_num = it.first.blockNum;
            }
        }

        wlog("m_cacheEchoMsgMap exceeds ${max}, echo msgs with block num ${num} will be cleared",
             ("max", m_maxCachedKeys)("num", old_block_num));
        for (auto it = m_cacheEchoMsgMap.begin(); it != m_cacheEchoMsgMap.end();) {
            if (it->first.blockNum == old_block_num) {
                m_cacheEchoMsgMap.erase(it++);
            } else {
                ++it;
            }
        }
    }

    void UranusController::clearOldCachedAllPhaseMsg() {
        if (m_echoMsgAllPhase.empty()) {
            return;
        }

        msgkey key = m_echoMsgAllPhase.begin()->first;
        for (auto &it : m_echoMsgAllPhase) {
            if (it.first.phase < key.phase) {
                key.phase = it.first.phase;
            }
        }

        wlog("m_echoMsgAllPhase exceeds ${max}, echo all phase msgs with block num ${num} and phase ${p} will be cleared",
             ("max", m_maxCachedAllPhaseKeys)("num", key.blockNum)("p", key.phase));
        auto itor = m_echoMsgAllPhase.find(key);
        if (itor != m_echoMsgAllPhase.end()) {
            m_echoMsgAllPhase.erase(itor);
        }
    }

    void UranusController::getContainersSize(uint32_t& proposeNum, uint32_t& echoNum, uint32_t& proposeCacheSize, uint32_t& echoCacheSize, uint32_t& allPhaseEchoNum) const{
        proposeNum = m_proposerMsgMap.size();
        echoNum = m_echoMsgMap.size();
        proposeCacheSize = m_cacheProposeMsgMap.size();
        echoCacheSize = m_cacheEchoMsgMap.size();
        allPhaseEchoNum = m_echoMsgAllPhase.size();
    }

    BlockHeaderDigest UranusController::findProposeMsgByBlockId(const chain::block_id_type& bid) const{
        BlockHeaderDigest tempHeaderDigest;
        auto ite = m_proposerMsgMap.find(bid);

        if(ite != m_proposerMsgMap.end()){
            tempHeaderDigest.digestFromBlockHeader(ite->second.block);
        }
        else {
            ULTRAIN_THROW(chain::msg_not_found_exception, "Propose msg not found by id." );
        }
        return tempHeaderDigest;
    }

    EchoMsgInfoDigest UranusController::findEchoMsgByBlockId(const chain::block_id_type& bid) const{
        EchoMsgInfoDigest tempEchoDigest;
        auto ite = m_echoMsgMap.find(bid);

        if(ite != m_echoMsgMap.end()) {
            tempEchoDigest.digestFromeEchoMsgInfo(ite->second);
        }
        else {
            ULTRAIN_THROW(chain::msg_not_found_exception, "Echo msg info not found by id." );
        }

        return tempEchoDigest;
    }

    std::vector<BlockHeaderDigest> UranusController::findProposeCacheByKey(const msgkey& msg_key) const {
        std::vector<BlockHeaderDigest> tempDigestVect;
        auto ite = m_cacheProposeMsgMap.find(msg_key);
        if(ite != m_cacheProposeMsgMap.end()) {
            for(auto& proposeMsg : ite->second){
                BlockHeaderDigest tempHeader;
                tempHeader.digestFromBlockHeader(proposeMsg.block);
                tempDigestVect.push_back(tempHeader);
            }
        }
        else {
            ULTRAIN_THROW(chain::msg_not_found_exception, "Propose msg not found by key." );
        }
        return tempDigestVect;
    }

    std::vector<EchoMsgDigest> UranusController::findEchoCacheByKey(const msgkey& msg_key) const {
        std::vector<EchoMsgDigest> tempEchoDigestVect;
        auto ite = m_cacheEchoMsgMap.find(msg_key);
        if(ite != m_cacheEchoMsgMap.end()) {
            for(auto& echoMsg : ite->second) {
                EchoMsgDigest tempEchoMsg;
                tempEchoMsg.digestFromeEchoMsg(echoMsg);
                tempEchoDigestVect.push_back(tempEchoMsg);
            }
        }
        else {
            ULTRAIN_THROW(chain::msg_not_found_exception, "Echo msg not found by key." );
        }
        return tempEchoDigestVect;
    }

    echo_msg_digest_vect UranusController::findEchoApMsgByKey(const msgkey& msg_key) const {
        echo_msg_digest_vect tempEchoMsgInfoDigestVect;
        auto ite = m_echoMsgAllPhase.find(msg_key);
        if(ite != m_echoMsgAllPhase.end()) {
            for(auto& echoMsgInfoPair : ite->second) {
                EchoMsgInfoDigest tempEchoMsgInfo;
                tempEchoMsgInfo.digestFromeEchoMsgInfo(echoMsgInfoPair.second);
                std::pair<chain::block_id_type, EchoMsgInfoDigest> tempPair(echoMsgInfoPair.first, tempEchoMsgInfo);
                tempEchoMsgInfoDigestVect.push_back(tempPair);
            }
        }
        else{
            ULTRAIN_THROW(chain::msg_not_found_exception, "Echo msg info not found by key." );
        }
        return tempEchoMsgInfoDigestVect;
    }
    bool UranusController::isEmpty(const BlockHeader& blockHeader) {
        // TODO: please find a way to cache the empty block's id
        return emptyBlock().id() == blockHeader.id();
    }

    bool UranusController::isBlank(const BlockHeader& blockHeader) {
        return Block().id() == blockHeader.id();
    }

    std::shared_ptr<Block> UranusController::generateEmptyBlock() {
        chain::controller &chain = appbase::app().get_plugin<chain_plugin>().chain();
        chain.abort_block();
        auto block_timestamp = chain.head_block_time() + fc::milliseconds(10000);
        chain.start_block(block_timestamp, 0);

        chain.set_action_merkle_hack();
        // empty block does not have trx, so we don't need this?
        chain.set_trx_merkle_hack();
        std::shared_ptr<Block> blockPtr = std::make_shared<Block>();
        const auto &pbs = chain.pending_block_state();
        const auto &bh = pbs->header;
        blockPtr->timestamp = bh.timestamp;
        blockPtr->producer = "ultrainio";
        blockPtr->previous = bh.previous;
        blockPtr->confirmed = 1;
        blockPtr->previous = bh.previous;
        blockPtr->transaction_mroot = bh.transaction_mroot;
        blockPtr->action_mroot = bh.action_mroot;
        // Discard the temp block.
        chain.abort_block();
        return blockPtr;
    }

    Block UranusController::blankBlock() {
        return Block();
    }

    void UranusController::setBa0Block(const Block& block) {
        m_ba0Block = block;
    }

    Block UranusController::emptyBlock() {
        static std::shared_ptr<Block> emptyBlock = nullptr;
        if (!emptyBlock || emptyBlock->block_num() != UranusNode::getInstance()->getBlockNum()) {
            emptyBlock = generateEmptyBlock();
        }
        return *emptyBlock;
    }
}  // namespace ultrainio
