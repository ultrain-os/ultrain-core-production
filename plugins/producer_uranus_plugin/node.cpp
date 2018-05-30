#include "node.hpp"

#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <chrono>
#include "define.hpp"

#include "connect.hpp"
#include "log.hpp"
#include "node_state.hpp"
#include "pktmanage.hpp"
#include "security.hpp"

using namespace boost::asio;
using namespace std;

namespace ultrainio {

    uint8_t UranusNode::URANUS_PUBLIC_KEY[VRF_PUBLIC_KEY_LEN] = {0};
    uint8_t UranusNode::URANUS_PRIVATE_KEY[VRF_PRIVATE_KEY_LEN] = {0};

    const boost::chrono::milliseconds UranusNode::MS_PER_PHASE{5000};
    const boost::chrono::seconds UranusNode::SECONDS_PER_ROUND(12);
    boost::chrono::system_clock::time_point UranusNode::GENESIS;

    const uint64_t UranusNode::MAX_HASH_LEN_VALUE = 0xFFFFFFFFFFFFFFFF;

    bool UranusNode::isTimeout() {
        boost::chrono::steady_clock::time_point now = boost::chrono::steady_clock::now();
        auto round_dur = boost::chrono::duration_cast<boost::chrono::milliseconds>(now - _start);

        if (round_dur > MS_PER_PHASE)
            return true;
        return false;
    }

    uint32_t UranusNode::get_block_id() const {
        return _block_id;
    }

    const uint8_t* UranusNode::get_role_proof() const {
        return _role_proof;
    }

    uranus_role UranusNode::get_role() const {
        return _role;
    }

    uranus_role UranusNode::generate_own_role(uint16_t phase) {
        char msg[64] = {0};
        LOG_INFO << "generate_own_role block_id " << _block_id << " phase = " << _phase << std::endl;
        snprintf(msg, 64, "%d%d", _block_id, _phase);
        if (!security::vrf_prove(_role_proof, (uint8_t*)msg, strlen(msg), UranusNode::URANUS_PRIVATE_KEY)) {
            LOG_INFO << "generate_own_role failed. _block_id = " << _block_id << " _phase = " << _phase << std::endl;
            return NONE;
        }
        uint64_t priority = proof_to_priority(_role_proof);
        LOG_INFO << "priority : " << priority << std::endl;
        uint64_t commitee_value = COMMITTEE_NODE_PERCENT * MAX_HASH_LEN_VALUE;
        if (PHASE_BA0 == phase) {
            if (priority < commitee_value/3 * 2) {
                return PROPOSER;
            } else if (priority < commitee_value) {
                return VOTER;
            } else {
                return LISTENER;
            }
        } else {
            if (priority < commitee_value) {
                return VOTER;
            } else {
                return LISTENER;
            }
        }
    }

    bool UranusNode::verifyRole(uint32_t block_id, uint16_t phase, const std::string& role_proof,
                                const std::string& pk) {
        if (role_proof.empty()) {
            return false;
        }
        char msg[64] = {0};
        snprintf(msg, 64, "%d%d", block_id, phase);
        if (security::vrf_verify((const uint8_t*)msg, strlen(msg), (const uint8_t*)role_proof.data(), (const uint8_t*)pk.data())) {
            return true;
        }
        return false;
    }

    uint64_t UranusNode::proof_to_priority(const uint8_t proof[VRF_PROOF_LEN]) {
        // 193:256 bit
        uint64_t priority = MAX_HASH_LEN_VALUE;
        size_t start_index = 24;
        size_t byte_num = 8;
        for (size_t i = 0; i < byte_num; i++) {
            priority += proof[start_index + i];
            if (i != byte_num - 1) {
                priority = priority << 8;
            }
        }
        return priority;
    }
    
    processresult UranusNode::Listen() {
        char recv_buffer[BUFSIZE] = {0};
        std::size_t dataLenth = 0;

        while (true) {
            dataLenth = connection.rcvMsg(recv_buffer, BUFSIZE);
            if (dataLenth > 0) {
                LOG_INFO << "receive msg length = " << dataLenth << std::endl;
                pktmng.processMsg(recv_buffer, dataLenth, _phase, _block_id);
            }
            if (isTimeout()) {
                LOG_INFO << "timeout" << std::endl;
                return processOk;
            }
        }
    }

    void UranusNode::run() {
        // BA0
        _phase = PHASE_BA0;
        _role = generate_own_role(_phase);
        LOG_INFO << "start BA0. role = " << _role << std::endl;
        processresult eResult = processOk;

        _start = boost::chrono::steady_clock::now();

        if (_role == PROPOSER) {
            MsgInfo msg_info;
            msg_info.txs = "txs_from_" + Connect::s_local_host + "_" + std::to_string(_block_id);
            msg_info.type = MSG_TYPE_PROPOSE;
            msg_info.block_id = _block_id;
            msg_info.proposer_role_vrf = std::string((char*)_role_proof, VRF_PROOF_LEN);
            msg_info.role_vrf = msg_info.proposer_role_vrf;
            pktmng.MsgInit(msg_info);

            LOG_INFO << "txs::" << msg_info.txs << std::endl;
            insertMsg(msg_info);
            sendMsg(msg_info);
            msg_info.type = MSG_TYPE_ECHO;
            sendMsg(msg_info);
        }
        node_state ba0_state;
        ba0_state.block_id = _block_id;
        ba0_state.phase = _phase;
        ba0_state.role = _role;
        ba0_state.self_proof = std::string((char*)_role_proof);

        eResult = Listen();
        LOG_INFO << "BA0 ok." << std::endl;

        TxsBlock ba0_block = pktmng.produce_tentative_block();

        _phase = PHASE_BA1;
        _role = generate_own_role(_phase);

        LOG_INFO << "start BA1. role = " << _role << std::endl;
        if (_role == VOTER && !ba0_block.txs_hash.empty()) {
            MsgInfo msg_info = PktManager::construct_msg_info(ba0_block);
            msg_info.phase = PHASE_BA1;
            msg_info.type = MSG_TYPE_ECHO;
            msg_info.block_id = _block_id;
            sendMsg(msg_info);
        }

        _start = boost::chrono::steady_clock::now();

        eResult = Listen();
        LOG_INFO << "BA1 ok." << std::endl;

        // produce block
        pktmng.block();
        _block_id++;
        sleep(2);
        LOG_INFO << "##############################_block_id = " << _block_id << std::endl;
    }

    bool UranusNode::startup() {
        if (!ultrainio::security::vrf_keypair(UranusNode::URANUS_PUBLIC_KEY, UranusNode::URANUS_PRIVATE_KEY)) {
            LOG_INFO << "generate key error. " << std::endl;
            return false;
        }
        return true;
    }

    void UranusNode::reset() {
        _phase = PHASE_INIT;
        _role = NONE;
        pktmng.reset();
    }

    bool UranusNode::insertMsg(MsgInfo &stMsgInfo) {
        return pktmng.insertMsg(stMsgInfo);;
    }

    bool UranusNode::sendMsg(MsgInfo &stMsgInfo) {
        char msg[BUFSIZE] = {0};
        char *pTmp = msg;
        size_t size = 0;
        if (stMsgInfo.type == MSG_TYPE_NOMSG) {
            LOG_ERROR << "need MSG_TYPE_NOMSG while is " << stMsgInfo.type << std::endl;
            return false;
        }
        // type
        *(uint16_t*)pTmp = stMsgInfo.type;
        pTmp += sizeof(uint16_t);

        // phase
        *(uint16_t*)pTmp = stMsgInfo.phase;
        pTmp += sizeof(uint16_t);

        // block_id
        *(uint32_t*)pTmp = stMsgInfo.block_id;
        pTmp += sizeof(uint32_t);

        // txs_hash
        *(uint32_t*)pTmp = TXS_HASH;
        pTmp += sizeof(uint32_t);

        *(uint32_t*)pTmp = stMsgInfo.txs_hash.length();
        pTmp += sizeof(uint32_t);

        memcpy(pTmp, stMsgInfo.txs_hash.c_str(), stMsgInfo.txs_hash.length());
        pTmp += stMsgInfo.txs_hash.length();

        // txs_hash_signature
        *(uint32_t*)pTmp = TXS_HASH_SIGN;
        pTmp += sizeof(uint32_t);

        *(uint32_t*)pTmp = stMsgInfo.txs_hash_signature.length();
        pTmp += sizeof(uint32_t);

        memcpy(pTmp, stMsgInfo.txs_hash_signature.c_str(), stMsgInfo.txs_hash_signature.length());
        pTmp += stMsgInfo.txs_hash_signature.length();

        // pk
        *(uint32_t*)pTmp = PK;
        pTmp += sizeof(uint32_t);

        *(uint32_t*)pTmp = stMsgInfo.pk.length();
        pTmp += sizeof(uint32_t);

        memcpy(pTmp, stMsgInfo.pk.c_str(), stMsgInfo.pk.length());
        pTmp += stMsgInfo.pk.length();

        // role_vrf
        *(uint32_t*)pTmp = ROLE_VRF,
        pTmp += sizeof(uint32_t);

        *(uint32_t*)pTmp = stMsgInfo.role_vrf.length();
        pTmp += sizeof(uint32_t);

        memcpy(pTmp, stMsgInfo.role_vrf.c_str(), stMsgInfo.role_vrf.length());
        pTmp += stMsgInfo.role_vrf.length();

        if (stMsgInfo.type == MSG_TYPE_EMPTY) {
            //empty
            *(uint32_t*)pTmp = TXS_EMPTY;
            pTmp += sizeof(uint32_t);

            //tail
            *(uint32_t*)pTmp = TAIL;
            pTmp += sizeof(uint32_t);

            size = pTmp - msg;
            if (size >= BUFSIZE) {
                return false;
            }
            connection.sendBrdCast(msg, size);
            return true;
        } else if (stMsgInfo.type == MSG_TYPE_PROPOSE) {
            //txs
            *(uint32_t*)pTmp = TXS;
            pTmp += sizeof(uint32_t);

            *(uint32_t*)pTmp = stMsgInfo.txs.length();
            pTmp += sizeof(uint32_t);

            memcpy(pTmp, stMsgInfo.txs.c_str(), stMsgInfo.txs.length());
            pTmp += stMsgInfo.txs.length();
        }

        // txs_sign
        *(uint32_t*)pTmp = TXS_SIGN;
        pTmp += sizeof(uint32_t);

        *(uint32_t*)pTmp = stMsgInfo.txs_signature.length();
        pTmp += sizeof(unsigned int);

        memcpy(pTmp, stMsgInfo.txs_signature.c_str(), stMsgInfo.txs_signature.length());
        pTmp += stMsgInfo.txs_signature.length();

        // proposer_pk
        *(uint32_t*)pTmp = PROPOSER_PK;
        pTmp += sizeof(uint32_t);

        *(uint32_t*)pTmp = stMsgInfo.proposer_pk.length();
        pTmp += sizeof(uint32_t);

        memcpy(pTmp, stMsgInfo.proposer_pk.c_str(), stMsgInfo.proposer_pk.length());
        pTmp += stMsgInfo.proposer_pk.length();

        // proposer_role_vrf
        *(uint32_t*)pTmp = PROPOSER_ROLE_VRF;
        pTmp += sizeof(uint32_t);

        *(uint32_t*)pTmp = stMsgInfo.proposer_role_vrf.length();
        pTmp += sizeof(uint32_t);

        memcpy(pTmp, stMsgInfo.proposer_role_vrf.c_str(), stMsgInfo.proposer_role_vrf.length());
        pTmp += stMsgInfo.proposer_role_vrf.length();

        // tail
        *(uint32_t*)pTmp = TAIL;
        pTmp += sizeof(uint32_t);
        size = pTmp - msg;

        if (size >= BUFSIZE) {
            return false;
        }
        connection.sendBrdCast(msg, size);
        return true;
    }
    
    bool UranusNode::ready_to_join() {
        boost::chrono::system_clock::time_point current_time = boost::chrono::system_clock::now();

        if (current_time <= GENESIS) {
            return false;
        } else if (GENESIS < current_time && current_time < (GENESIS + SECONDS_PER_ROUND)) {
            _block_id = 0;
            LOG_INFO << "genesis block " << std::endl;
            return true;
        } else {
            boost::chrono::seconds pass_time_to_genesis
                    = boost::chrono::duration_cast<boost::chrono::seconds>(current_time - GENESIS);
            LOG_INFO << "pass_time_to_genesis = " << pass_time_to_genesis << std::endl;
            _block_id = pass_time_to_genesis / SECONDS_PER_ROUND;
            boost::chrono::system_clock::time_point next_block_time
                    = (SECONDS_PER_ROUND - pass_time_to_genesis%SECONDS_PER_ROUND) + current_time;
            LOG_INFO << "wait second = " << SECONDS_PER_ROUND - pass_time_to_genesis%SECONDS_PER_ROUND
                      << " _block_id = " << _block_id << std::endl;
            bool ready;
            while (true) {
                if (boost::chrono::system_clock::now() > next_block_time) {
                    _block_id++;
                    if (ready) {
                        LOG_INFO << "ready to join Ultrain.IO " << std::endl;
                        return true;
                    } else {
                        LOG_INFO << "still not ready to join Ultrain.IO. wait another seconds = "
                                      << SECONDS_PER_ROUND << std::endl;
                        next_block_time += SECONDS_PER_ROUND;
                        ready = true;
                    }
                }
            }

        }
    }
} 


