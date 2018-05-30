#include "pktmanage.hpp"

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>

#include <boost/asio.hpp>

#include "define.hpp"
#include "node.hpp"
#include "security.hpp"
#include "log.hpp"

using namespace boost::asio;
using namespace std;

namespace ultrainio {

    MsgInfo PktManager::construct_msg_info(const TxsBlock& block) {
        MsgInfo msg_info;
        msg_info.txs_signature = block.txs_signature;
        msg_info.proposer_pk = block.proposer_pk;
        msg_info.proposer_role_vrf = block.proposer_role_vrf;
        msg_info.txs_hash = block.txs_hash;
        msg_info.txs = block.txs;
        return msg_info;
    }

    PktManager::PktManager(UranusNode* node) : _node(node), _proposer_msg_map(), _echo_msg_map() {

    }

    void PktManager::reset() {
        _proposer_msg_map.clear();
        _echo_msg_map.clear();
    }

    bool PktManager::isValidMsg(MsgInfo &stMsgInfo, string &signature) {
        if (signature == stMsgInfo.pk) {
            LOG_INFO << "loopback msg. type = " << stMsgInfo.type << " txs = " << stMsgInfo.txs << std::endl;
            return false;
        }

        if (stMsgInfo.type == MSG_TYPE_PROPOSE) {
            if (!ultrainio::security::vrf_verify((const uint8_t*)stMsgInfo.txs.data(), stMsgInfo.txs.length(),
                                              (const uint8_t*)stMsgInfo.txs_signature.data(),
                                              (const uint8_t*)stMsgInfo.proposer_pk.data())) {
                LOG_INFO << "valid msg fail. txs_signature = " <<  stMsgInfo.txs_signature << std::endl;
                return false;
            }
            // TODO verify proposer role
        }
        // TODO verify txs_hash, role
        return true;
    }

    bool PktManager::formatMsg(MsgInfo &stMsgInfo, const char *buf, size_t size, uint16_t local_phase, uint32_t local_block_id) {
        const char *tmpBuffer = buf;
        char tmpStr[BUFSIZE] = {0};

        //lenth check 100:min msg head
        if ((buf == nullptr) || (size >= BUFSIZE) || (size < MIN_MSG_HEAD)) {
            LOG_INFO << "check error. size = " << size << std::endl;
            return false;
        }

        // type
        stMsgInfo.type = *(uint16_t*)tmpBuffer;
        tmpBuffer += sizeof(uint16_t);
        if ((stMsgInfo.type != MSG_TYPE_PROPOSE) && (stMsgInfo.type != MSG_TYPE_ECHO)
            && (stMsgInfo.type != MSG_TYPE_EMPTY)) {
            LOG_INFO << "type : " << stMsgInfo.type << "not found." << std::endl;
            return false;
        }

        // phase
        stMsgInfo.phase = *(uint16_t*)tmpBuffer;
        tmpBuffer += sizeof(uint16_t);
        if (stMsgInfo.phase != local_phase) {
            LOG_ERROR << "type : " << stMsgInfo.type << " phase = " << stMsgInfo.phase << " while local phase = " << local_phase << std::endl;
            return false;
        }

        // block_id
        stMsgInfo.block_id = *(uint32_t*)tmpBuffer;
        tmpBuffer += sizeof(uint32_t);
        if (stMsgInfo.block_id != local_block_id) {
            LOG_ERROR << "type : " << stMsgInfo.type << " phase : " << stMsgInfo.phase << " block_id : " << stMsgInfo.block_id
                      << " while local_block_id = " << local_block_id << std::endl;
            return false;
        }

        uint32_t segment_type = 0;
        uint32_t segment_len = 0;

        // txs_hash
        segment_type = *(uint32_t*)tmpBuffer;
        tmpBuffer += sizeof(uint32_t);

        if (segment_type != TXS_HASH) {
            LOG_ERROR << "TXS_HASH error. block_id : " << stMsgInfo.block_id << " phase : " << stMsgInfo.phase << std::endl;
            return false;
        }

        segment_len = *(uint32_t*)tmpBuffer;
        tmpBuffer += sizeof(uint32_t);

        memcpy(tmpStr, tmpBuffer, segment_len);
        tmpBuffer += segment_len;

        stMsgInfo.txs_hash = std::string(tmpStr, segment_len);
        memset(tmpStr, 0, BUFSIZE);

        if (stMsgInfo.txs_hash.length() != SHA256_DIGEST_LEN) {
            LOG_ERROR << "txs hash length = " << stMsgInfo.txs_hash.length() << std::endl;
            return false;
        }

        // txs_hash_signature
        segment_type = *(uint32_t*)tmpBuffer;
        tmpBuffer += sizeof(uint32_t);

        if (segment_type != TXS_HASH_SIGN) {
            LOG_ERROR << "TXS_HASH_SIGN error. block_id : " << stMsgInfo.block_id
                      << " phase : " << stMsgInfo.phase << std::endl;
            return false;
        }

        segment_len = *(uint32_t*)tmpBuffer;
        tmpBuffer += sizeof(uint32_t);

        memcpy(tmpStr, tmpBuffer, segment_len);
        tmpBuffer += segment_len;

        stMsgInfo.txs_hash_signature = std::string(tmpStr, segment_len);
        memset(tmpStr, 0, BUFSIZE);

        if (stMsgInfo.txs_hash_signature.length() != VRF_PROOF_LEN) {
            LOG_ERROR << "txs_hash_signature length = " << stMsgInfo.txs_hash_signature.length() << std::endl;
            return false;
        }

        // pk
        segment_type = *(uint32_t*)tmpBuffer;
        tmpBuffer += sizeof(uint32_t);

        if (segment_type != PK) {
            LOG_ERROR << "need PK while " << segment_type << " block_id " << stMsgInfo.block_id
                      << " phase : " << stMsgInfo.phase << std::endl;
            return false;
        }

        segment_len = *(uint32_t*)tmpBuffer;
        tmpBuffer += sizeof(uint32_t);

        memcpy(tmpStr, tmpBuffer, segment_len);
        tmpBuffer += segment_len;

        stMsgInfo.pk = std::string(tmpStr, segment_len);
        memset(tmpStr, 0, BUFSIZE);

        if (stMsgInfo.pk.length() != VRF_PUBLIC_KEY_LEN) {
            LOG_ERROR << "public key length = " << stMsgInfo.pk.length() << std::endl;
            return false;
        }

        // role_vrf
        segment_type = *(uint32_t*)tmpBuffer;
        tmpBuffer += sizeof(uint32_t);

        if (segment_type != ROLE_VRF) {
            LOG_ERROR << "need ROLE_VRF while " << segment_type << " block_id " << stMsgInfo.block_id
                      << " phase : " << stMsgInfo.phase << std::endl;
            return false;
        }

        segment_len = *(uint32_t*)tmpBuffer;
        tmpBuffer += sizeof(uint32_t);

        memcpy(tmpStr, tmpBuffer, segment_len);
        tmpBuffer += segment_len;

        stMsgInfo.role_vrf = std::string(tmpStr, segment_len);
        memset(tmpStr, 0, BUFSIZE);

        if (stMsgInfo.role_vrf.length() != VRF_PROOF_LEN) {
            LOG_ERROR << " role_vrf length = " << stMsgInfo.role_vrf.length()
                      << " expected : " << VRF_PROOF_LEN << std::endl;
            return false;
        }

        if (stMsgInfo.type == MSG_TYPE_EMPTY) {
            segment_type = *(uint32_t*)tmpBuffer;
            tmpBuffer += sizeof(uint32_t);

            if (segment_type != TXS_EMPTY) {
                LOG_ERROR << "need TXS_EMPTY while is " << segment_type << " block_id " << stMsgInfo.block_id
                          << " phase : " << stMsgInfo.phase << std::endl;
                return false;
            }

            if (*(uint32_t*)tmpBuffer != TAIL) {
                LOG_ERROR << "need TAIL while is " << segment_type << " block_id " << stMsgInfo.block_id
                          << " phase : " << stMsgInfo.phase << std::endl;
                return false;
            }
            return true;
        } else if (stMsgInfo.type == MSG_TYPE_PROPOSE) {
            segment_type = *(uint32_t*)tmpBuffer;
            tmpBuffer += sizeof(uint32_t);

            if (segment_type != TXS) {
                LOG_ERROR << "need TXS while is " << segment_type << " block_id " << stMsgInfo.block_id
                          << " phase : " << stMsgInfo.phase << std::endl;
                return false;
            }

            segment_len = *(uint32_t*)tmpBuffer;
            tmpBuffer += sizeof(uint32_t);

            memcpy(tmpStr, tmpBuffer, segment_len);
            tmpBuffer += segment_len;

            stMsgInfo.txs = std::string(tmpStr, segment_len);
            memset(tmpStr, 0, BUFSIZE);
        }

        // txs_signature
        segment_type = *(uint32_t*)tmpBuffer;
        tmpBuffer += sizeof(uint32_t);

        if (segment_type != TXS_SIGN) {
            LOG_ERROR << "need TXS_SIGN while is " << segment_type << " block_id " << stMsgInfo.block_id
                      << " phase : " << stMsgInfo.phase << std::endl;
            return false;
        }

        segment_len = *(uint32_t*)tmpBuffer;
        tmpBuffer += sizeof(uint32_t);

        memcpy(tmpStr, tmpBuffer, segment_len);
        tmpBuffer += segment_len;

        stMsgInfo.txs_signature = std::string(tmpStr, segment_len);
        memset(tmpStr, 0, BUFSIZE);

        if (stMsgInfo.txs_signature.length() != VRF_PROOF_LEN) {
            LOG_ERROR << " txs_signature length = " << stMsgInfo.txs_signature.length()
                      << " expected " << VRF_PROOF_LEN << std::endl;
            return false;
        }
        // proposer_pk
        segment_type = *(uint32_t*)tmpBuffer;
        tmpBuffer += sizeof(uint32_t);

        if (segment_type != PROPOSER_PK) {
            LOG_ERROR << "need PROPOSER_PK while is " << segment_type << " block_id " << stMsgInfo.block_id
                      << " phase : " << stMsgInfo.phase << std::endl;
            return false;
        }

        segment_len = *(uint32_t*)tmpBuffer;
        tmpBuffer += sizeof(uint32_t);

        memcpy(tmpStr, tmpBuffer, segment_len);
        tmpBuffer += segment_len;

        stMsgInfo.proposer_pk = std::string(tmpStr, segment_len);
        memset(tmpStr, 0, BUFSIZE);

        if (stMsgInfo.proposer_pk.length() != VRF_PUBLIC_KEY_LEN) {
            LOG_ERROR << "proposer_pk length = " << stMsgInfo.proposer_pk.length() << std::endl;
            return false;
        }

        // proposer_role_vrf
        segment_type = *(uint32_t*)tmpBuffer;
        tmpBuffer += sizeof(uint32_t);

        if (segment_type != PROPOSER_ROLE_VRF) {
            LOG_ERROR << "need PROPOSER_ROLE_VRF while is " << segment_type << " block_id " << stMsgInfo.block_id
                      << " phase : " << stMsgInfo.phase << std::endl;
        }

        segment_len = *(uint32_t*)tmpBuffer;
        tmpBuffer += sizeof(uint32_t);

        memcpy(tmpStr, tmpBuffer, segment_len);
        tmpBuffer += segment_len;

        stMsgInfo.proposer_role_vrf = std::string(tmpStr, segment_len);
        memset(tmpStr, 0, BUFSIZE);

        if (stMsgInfo.proposer_role_vrf.length() != VRF_PROOF_LEN) {
            LOG_ERROR << "proposer_role_vrf length = " << stMsgInfo.proposer_role_vrf.length()
                      << " expected : " << VRF_PROOF_LEN << std::endl;
            return false;
        }

        LOG_INFO << "format msg ok." << endl;

        LOG_INFO << "txsHash = " << UltrainLog::get_unprintable(stMsgInfo.txs_hash) << std::endl;
        
        return true;
    }

    bool PktManager::insertMsg(MsgInfo &stMsgInfo) {
        ProposeMsg stProposeMsg;
        stProposeMsg.phase = PHASE_BA0;
        stProposeMsg.txs = stMsgInfo.txs;
        stProposeMsg.txs_hash = stMsgInfo.txs_hash;
        stProposeMsg.txs_signature = stMsgInfo.txs_signature;
        stProposeMsg.proposer_pk = stMsgInfo.proposer_pk;
        stProposeMsg.proposer_role_vrf = stMsgInfo.proposer_role_vrf;

        _proposer_msg_map.insert(make_pair(stProposeMsg.txs_hash, stProposeMsg));
        return true;
    }

    processresult PktManager::processMsg(const char* buf, size_t size, uint16_t local_phase, uint32_t local_block_id) {
        MsgInfo msg_info;
        bool bResult = formatMsg(msg_info, buf, size, local_phase, local_block_id);
        if (!bResult)
            return processErr;

        std::string this_pk = std::string((const char*)UranusNode::URANUS_PUBLIC_KEY, VRF_PUBLIC_KEY_LEN);
        if (!isValidMsg(msg_info, this_pk))
            return processErr;

        LOG_INFO << "pk :" << UltrainLog::get_unprintable(msg_info.pk) << std::endl;
        //analyze msg
        if (msg_info.type == MSG_TYPE_PROPOSE) {
            ProposeMsg stProposeMsg;
            LOG_INFO << "receive msg MSG_TYPE_PROPOSE" << endl;
            stProposeMsg.phase = static_cast<consensus_phase>(msg_info.phase);
            stProposeMsg.txs = msg_info.txs;
            stProposeMsg.txs_hash = msg_info.txs_hash;
            stProposeMsg.txs_signature = msg_info.txs_signature;
            stProposeMsg.proposer_pk = msg_info.proposer_pk;
            stProposeMsg.proposer_role_vrf = msg_info.proposer_role_vrf;

            auto propose_it = _proposer_msg_map.find(stProposeMsg.txs_hash);
            if (_proposer_msg_map.end() == propose_it) {
                if (is_min_propose(stProposeMsg)) {
                    MsgInfo msg_info;
                    msg_info.block_id = _node->get_block_id();
                    msg_info.role_vrf = std::string((char*)_node->get_role_proof(), VRF_PROOF_LEN);
                    msg_info.phase = stProposeMsg.phase;
                    msg_info.type = MSG_TYPE_ECHO;
                    msg_info.txs_signature = stProposeMsg.txs_signature;
                    msg_info.proposer_pk = stProposeMsg.proposer_pk;
                    msg_info.proposer_role_vrf = stProposeMsg.proposer_role_vrf;
                    msg_info.txs_hash = stProposeMsg.txs_hash;
                    msg_info.pk = std::string((char*)ultrainio::UranusNode::URANUS_PUBLIC_KEY, VRF_PUBLIC_KEY_LEN);
                    uint8_t txs_hash_signature[VRF_PROOF_LEN];
                    ultrainio::security::vrf_prove(txs_hash_signature, (uint8_t*)msg_info.txs_hash.data(),
                                                   msg_info.txs_hash.length(), ultrainio::UranusNode::URANUS_PRIVATE_KEY);
                    msg_info.txs_hash_signature = std::string((char*)txs_hash_signature, VRF_PROOF_LEN);
                    if (_node->get_role() != LISTENER) {
                        LOG_INFO << "receive min propose message, and send echo message" << std::endl;
                        _node->sendMsg(msg_info);
                    }
                }
                _proposer_msg_map.insert(make_pair(stProposeMsg.txs_hash, stProposeMsg));
                return SendMsg;
            }
        } else { // echo message
            LOG_INFO << "receive msg MSG_TYPE_ECHO" << endl;
            std::string txs_hash = msg_info.txs_hash;
            auto echo_it = _echo_msg_map.find(txs_hash);
            if (_echo_msg_map.end() != echo_it) {
                // TODO check echo sign
                auto sign_it = std::find(echo_it->second.pk_pool.begin(), echo_it->second.pk_pool.end(), msg_info.pk);
                if (sign_it == echo_it->second.pk_pool.end()) { // new pk
                    echo_it->second.pk_pool.push_back(msg_info.pk);
                    size_t sign_num = echo_it->second.pk_pool.size();
                    if ((sign_num == THRESHOLD_SEND_ECHO) && (local_phase == PHASE_BA0) && is_min_2f_echo(echo_it->second)) {
                        msg_info.block_id = _node->get_block_id();
                        msg_info.role_vrf = std::string((char*)_node->get_role_proof(), VRF_PROOF_LEN);
                        msg_info.phase = echo_it->second.phase;
                        msg_info.type = MSG_TYPE_ECHO;
                        msg_info.txs_signature = echo_it->second.txs_signature;
                        msg_info.proposer_pk = echo_it->second.proposer_pk;
                        msg_info.proposer_role_vrf = echo_it->second.proposer_role_vrf;
                        msg_info.txs_hash = echo_it->second.txs_hash;
                        msg_info.pk = std::string((char*)ultrainio::UranusNode::URANUS_PUBLIC_KEY, VRF_PUBLIC_KEY_LEN);
                        uint8_t txs_hash_signature[VRF_PROOF_LEN];
                        ultrainio::security::vrf_prove(txs_hash_signature, (uint8_t*)msg_info.txs_hash.data(), msg_info.txs_hash.length(), ultrainio::UranusNode::URANUS_PRIVATE_KEY);
                        msg_info.txs_hash_signature = std::string((char*)txs_hash_signature, VRF_PROOF_LEN);
                        if (_node->get_role() != LISTENER) {
                            LOG_INFO << "receive the " << THRESHOLD_SEND_ECHO << "th echo message, and broadcast echo" << std::endl;
                            _node->sendMsg(msg_info);
                        }
                        return SendMsg;
                    }
                }
            } else {
                EchoMsg stEchoMsg;
                stEchoMsg.phase = static_cast<consensus_phase>(msg_info.phase);
                stEchoMsg.txs_hash = msg_info.txs_hash;
                stEchoMsg.txs_signature = msg_info.txs_signature;
                stEchoMsg.proposer_pk = msg_info.proposer_pk;
                stEchoMsg.proposer_role_vrf = msg_info.proposer_role_vrf;
                stEchoMsg.pk_pool.push_back(msg_info.pk);
                _echo_msg_map.insert(make_pair(stEchoMsg.txs_hash, stEchoMsg));
            }
        }
        return processOk;
    }

    bool PktManager::is_min_propose(const ProposeMsg& propose_msg) {
        uint64_t priority = UranusNode::proof_to_priority((const uint8_t*)propose_msg.proposer_role_vrf.data());
        for (auto propose_itor = _proposer_msg_map.begin(); propose_itor != _proposer_msg_map.end(); propose_itor++) {
            if (UranusNode::proof_to_priority((const uint8_t*)propose_itor->second.proposer_role_vrf.data()) < priority) {
                return false;
            }
        }
        return true;
    }

    bool PktManager::is_min_2f_echo(const EchoMsg& echo_msg) {
        uint64_t priority = UranusNode::proof_to_priority((const uint8_t*)echo_msg.proposer_role_vrf.data());
        for (auto echo_itor = _echo_msg_map.begin(); echo_itor != _echo_msg_map.end(); echo_itor++) {
            if (echo_itor->second.pk_pool.size() >= THRESHOLD_SEND_ECHO) {
                if (UranusNode::proof_to_priority((const uint8_t*)echo_itor->second.proposer_role_vrf.data()) < priority) {
                    return false;
                }
            }
        }
        return true;
    }

    bool PktManager::MsgInit(MsgInfo &stMsgInfo) {
        stMsgInfo.phase = PHASE_BA0;
        stMsgInfo.type = MSG_TYPE_PROPOSE;
        stMsgInfo.proposer_pk = std::string((char *) UranusNode::URANUS_PUBLIC_KEY, VRF_PUBLIC_KEY_LEN);
        stMsgInfo.pk = std::string((char *) UranusNode::URANUS_PUBLIC_KEY, VRF_PUBLIC_KEY_LEN);

        uint8_t txs_signature[VRF_PROOF_LEN] = {0};
        if (!ultrainio::security::vrf_prove(txs_signature, (uint8_t*) stMsgInfo.txs.data(), stMsgInfo.txs.length(),
                                         UranusNode::URANUS_PRIVATE_KEY)) {
            LOG_ERROR << "vrf_prove" << std::endl;
            return false;
        }
        stMsgInfo.txs_signature = std::string((char *) txs_signature, VRF_PROOF_LEN);

        uint8_t txs_hash[SHA256_DIGEST_LEN] = {0};
        if (!ultrainio::security::sha256((uint8_t*) stMsgInfo.txs.data(), stMsgInfo.txs.length(), txs_hash)) {
            LOG_ERROR << "sha256 failed" << std::endl;
            return false;
        }
        stMsgInfo.txs_hash = std::string((char *) txs_hash, SHA256_DIGEST_LEN);

        uint8_t txs_hash_signature[VRF_PROOF_LEN] = {0};
        if (!ultrainio::security::vrf_prove(txs_hash_signature, (uint8_t*)stMsgInfo.txs_hash.data(),
                                          stMsgInfo.txs_hash.length(), UranusNode::URANUS_PRIVATE_KEY)) {
            LOG_ERROR << "txs_hash_signature vrf_prove error." << std::endl;
            return false;
        }
        stMsgInfo.txs_hash_signature = std::string((char*)txs_hash_signature, VRF_PROOF_LEN);

        LOG_INFO << endl;
        LOG_INFO << "msg_init!!!" << std::endl;
        return true;
    }

    TxsBlock PktManager::produce_tentative_block() {
        uint64_t min_priority = UranusNode::MAX_HASH_LEN_VALUE;
        std::string min_txs_hash;
        EchoMsg min_echo_msg;
        for (auto echo_itor = _echo_msg_map.begin(); echo_itor != _echo_msg_map.end(); echo_itor++) {
            UltrainLog::displayEcho(echo_itor->second);
            if (echo_itor->second.pk_pool.size() >= THRESHOLD_NEXT_ROUND) {
                uint64_t priority = UranusNode::proof_to_priority((const uint8_t*)echo_itor->second.proposer_role_vrf.data());
                if (min_priority >= priority) {
                    min_txs_hash = echo_itor->second.txs_hash;
                    min_priority = priority;
                    min_echo_msg = echo_itor->second;
                }
            }
        }
        TxsBlock tentative_block;
        LOG_INFO << "min_txs_hash : " << UltrainLog::get_unprintable(min_txs_hash) << std::endl;
        if (min_txs_hash.empty()) {
            return tentative_block;
        }
        std::map<string, ProposeMsg>::iterator propose_itor;
        if ((propose_itor = _proposer_msg_map.find(min_txs_hash)) != _proposer_msg_map.end()) {
            tentative_block.txs_signature = min_echo_msg.txs_signature;
            tentative_block.proposer_pk = min_echo_msg.proposer_pk;
            tentative_block.pk_pool = min_echo_msg.pk_pool;
            tentative_block.txs_hash = min_echo_msg.txs_hash;
            tentative_block.proposer_role_vrf = min_echo_msg.proposer_role_vrf;
            tentative_block.txs = propose_itor->second.txs;
        }
        return tentative_block;
    }

    void PktManager::block() {
        TxsBlock block = produce_tentative_block();
        if (!block.txs_hash.empty()) {
            UltrainLog::displayBlock(block);
        } else {
            LOG_ERROR << "block is empty!!!" << std::endl;
        }
    }

}  // namespace ultrainio


