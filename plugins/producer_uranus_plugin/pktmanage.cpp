#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <boost/asio.hpp>

#include <ultrainio/producer_uranus_plugin/define.hpp>
#include <ultrainio/producer_uranus_plugin/node.hpp>
#include <ultrainio/producer_uranus_plugin/pktmanage.hpp>
#include <ultrainio/producer_uranus_plugin/security.hpp>

using namespace boost::asio;
using namespace std;

namespace ultrain {

    static string strLocalSign; // TODO

    PktManager::PktManager() : _proposer_msg_map(), _echo_msg_map(), preblock(), preBA() {

    }

    void PktManager::reset() {
        _proposer_msg_map.clear();
        _echo_msg_map.clear();
        preblock.clear();
        preBA.clear();
    }

    bool PktManager::isValidMsg(MsgInfo &stMsgInfo, string &signature) {
        if (signature == stMsgInfo.signature) {
            std::cout << "loopback msg:" << stMsgInfo.txs << std::endl;
            return false;
        }

        if (ultrain::security::vrf_verify((const uint8_t *) stMsgInfo.txs.data(), stMsgInfo.txs.length(),
                                          (const uint8_t *) stMsgInfo.proof.data(),
                                          (const uint8_t *) stMsgInfo.proposerSignature.data())) {
            std::cout << "valid msg fail. proof = " <<  stMsgInfo.proof << std::endl;
            return false;
        }
        return true;
    }

    bool PktManager::formatMsg(MsgInfo &stMsgInfo, const char *buf, size_t size, unsigned short local_phase) {
        unsigned short usType;
        unsigned int ulMsgType;
        unsigned int ulMsgLen;
        const char *tmpBuffer = buf;
        char tmpStr[BUFSIZE] = {0};

        //lenth check 100:min msg head
        if ((size >= BUFSIZE) || (NULL == buf) || (size < MIN_MSG_HEAD)) {
            //log
            return false;
        }

        usType = *(unsigned short *) tmpBuffer;
        tmpBuffer += sizeof(unsigned short);
        stMsgInfo.eType = usType;
        if ((usType != MSG_TYPE_PROPOSE) && (usType != MSG_TYPE_ECHO) && (usType != MSG_TYPE_EMPTY)) {
            std::cout << "usType : " << usType << "not found." << std::endl;
            return false;
        }

        uint16_t phase = *(uint16_t*)tmpBuffer;
        tmpBuffer += sizeof(uint16_t);
        if (phase != local_phase) {
            std::cerr << "phase = " << phase << " while local phase = " << local_phase << std::endl;
            return false;
        }

        stMsgInfo.phase = phase;

        //proof
        ulMsgType = *(unsigned int *) tmpBuffer;
        tmpBuffer += sizeof(unsigned int);

        ulMsgLen = *(unsigned int *) tmpBuffer;
        tmpBuffer += sizeof(unsigned int);

        memcpy(tmpStr, tmpBuffer, ulMsgLen);
        tmpBuffer += ulMsgLen;

        stMsgInfo.proof = tmpStr;
        memset(tmpStr, 0, BUFSIZE);

        if (stMsgInfo.proof.length() != security::VRF_PROOF_LEN)
            return false;

        //sign
        ulMsgType = *(unsigned int *) tmpBuffer;
        tmpBuffer += sizeof(unsigned int);

        ulMsgLen = *(unsigned int *) tmpBuffer;
        tmpBuffer += sizeof(unsigned int);

        memcpy(tmpStr, tmpBuffer, ulMsgLen);
        tmpBuffer += ulMsgLen;

        stMsgInfo.signature = tmpStr;
        memset(tmpStr, 0, BUFSIZE);

        if (stMsgInfo.signature.length() != MSG_LENGTH_KEY)
            return false;

        //empty,propose proof
        ulMsgType = *(unsigned int *) tmpBuffer;
        tmpBuffer += sizeof(unsigned int);

        ulMsgLen = *(unsigned int *) tmpBuffer;
        tmpBuffer += sizeof(unsigned int);

        memcpy(tmpStr, tmpBuffer, ulMsgLen);
        tmpBuffer += ulMsgLen;

        if (ulMsgType == TXSEMPTY) {
            //empty block process
            stMsgInfo.eType = MSG_TYPE_EMPTY;
            return true;
        } else {
            stMsgInfo.proposerProof = tmpStr;
            memset(tmpStr, 0, BUFSIZE);
        }

        if (stMsgInfo.proposerProof.length() != security::VRF_PROOF_LEN)
            return false;

        //propose sign
        ulMsgType = *(unsigned int *) tmpBuffer;
        tmpBuffer += sizeof(unsigned int);

        ulMsgLen = *(unsigned int *) tmpBuffer;
        tmpBuffer += sizeof(unsigned int);

        memcpy(tmpStr, tmpBuffer, ulMsgLen);
        tmpBuffer += ulMsgLen;

        stMsgInfo.proposerSignature = tmpStr;
        memset(tmpStr, 0, BUFSIZE);

        if (stMsgInfo.proposerSignature.length() != MSG_LENGTH_KEY)
            return false;

        //txshash
        ulMsgType = *(unsigned int *) tmpBuffer;
        tmpBuffer += sizeof(unsigned int);

        ulMsgLen = *(unsigned int *) tmpBuffer;
        tmpBuffer += sizeof(unsigned int);

        memcpy(tmpStr, tmpBuffer, ulMsgLen);
        tmpBuffer += ulMsgLen;

        stMsgInfo.txsHash = tmpStr;
        memset(tmpStr, 0, BUFSIZE);

        if (stMsgInfo.txsHash.length() != MSG_LENGTH_HASH)
            return false;

        if (stMsgInfo.eType == MSG_TYPE_PROPOSE) {

            //txs
            ulMsgType = *(unsigned int *) tmpBuffer;
            tmpBuffer += sizeof(unsigned int);

            ulMsgLen = *(unsigned int *) tmpBuffer;
            tmpBuffer += sizeof(unsigned int);

            memcpy(tmpStr, tmpBuffer, ulMsgLen);
            tmpBuffer += ulMsgLen;

            stMsgInfo.txs = tmpStr;
            memset(tmpStr, 0, BUFSIZE);

        }
        return true;
    }

    bool PktManager::insertMsg(MsgInfo &stMsgInfo) {
        ProposeMsg stProposeMsg;
        stProposeMsg.phase = PHASE_BA0;
        stProposeMsg.usSendFlag = MSG_ECHO_SEND_OK;
        stProposeMsg.txs = stMsgInfo.txs;
        stProposeMsg.txsHash = stMsgInfo.txsHash;
        stProposeMsg.proof = stMsgInfo.proposerProof;
        stProposeMsg.signature = stMsgInfo.proposerSignature;

        _proposer_msg_map.insert(make_pair(stProposeMsg.txsHash, stProposeMsg));
        return true;
    }

    processresult PktManager::processMsg(const char* buf, size_t size, unsigned short usLocalRound) {
        ProposeMsg stProposeMsg;
        EchoMsg stEchoMsg;
        std::map<std::string, ProposeMsg>::iterator propose_it;
        std::map<std::string, EchoMsg>::iterator echo_it;
        std::vector<std::string>::iterator sign_it;
        std::string strSign;
        std::string strMinProof;
        size_t uiSignNum = 0;
        MsgInfo msg_info;
        bool bResult = formatMsg(msg_info, buf, size, usLocalRound);
        if (!bResult)
            return processErr;

        if (!isValidMsg(msg_info, strLocalSign))
            return processErr;

        cout << "signature :" << msg_info.signature << endl;
        //analyze msg
        if (msg_info.eType == MSG_TYPE_PROPOSE) {
            //auth
            stProposeMsg.phase = msg_info.phase;
            stProposeMsg.txs = msg_info.txs;
            stProposeMsg.txsHash = msg_info.txsHash;
            stProposeMsg.proof = msg_info.proposerProof;
            stProposeMsg.signature = msg_info.proposerSignature;

            propose_it = _proposer_msg_map.find(stProposeMsg.txsHash);

            if (_proposer_msg_map.end() == propose_it) {
                stProposeMsg.usSendFlag = MSG_ECHO_NEED_SEND;
                _proposer_msg_map.insert(make_pair(stProposeMsg.txsHash, stProposeMsg));
                return SendMsg;
            }
        } else {
            //echo msg
            //auth
            stEchoMsg.phase = msg_info.phase;
            stEchoMsg.txsHash = msg_info.txsHash;
            stEchoMsg.proposerProof = msg_info.proposerProof;
            stEchoMsg.proposerSignature = msg_info.proposerSignature;
            strSign = msg_info.signature;

            echo_it = _echo_msg_map.find(stEchoMsg.txsHash);
            if (_echo_msg_map.end() != echo_it) {
                //modify
                //signature process
                sign_it = std::find(echo_it->second.signpool.begin(), echo_it->second.signpool.end(), strSign);
                //sign_it = echo_it->second.signpool.end();
                if (sign_it == echo_it->second.signpool.end()) {
                    echo_it->second.signpool.push_back(strSign);
                    uiSignNum = echo_it->second.signpool.size();
                    strMinProof = echo_it->second.proposerProof;

                    if ((uiSignNum == THRESHOLD_SEND_ECHO) && (usLocalRound == PHASE_BA0)) {
                        echo_it->second.usSendFlag = MSG_ECHO_NEED_SEND;
                        return SendMsg;
                    } else if (uiSignNum == THRESHOLD_NEXT_ROUND) {
                        for (echo_it = _echo_msg_map.begin(); echo_it != _echo_msg_map.end(); echo_it++) {
                            if (echo_it->second.proposerProof < strMinProof) {
                                return processOk;
                            }
                        }
                        propose_it = _proposer_msg_map.find(stEchoMsg.txsHash);
                        if (propose_it == _proposer_msg_map.end()) {
                            return processOk;
                        }
                        if (usLocalRound == PHASE_BA1) {
                            preblock.proposerProof = echo_it->second.proposerProof;
                            preblock.proposerSignature = echo_it->second.proposerSignature;
                            preblock.signpool = echo_it->second.signpool;
                            preblock.txsHash = echo_it->second.txsHash;
                            preblock.txs = propose_it->second.txs;
                            return Block;
                        } else if (usLocalRound == PHASE_BA0) {
                            preBA.proposerProof = echo_it->second.proposerProof;
                            preBA.proposerSignature = echo_it->second.proposerSignature;
                            preBA.signpool = echo_it->second.signpool;
                            preBA.txsHash = echo_it->second.txsHash;
                            preBA.txs = propose_it->second.txs;
                            return IntoBA;
                        }
                    }
                }
            } else {
                //insert
                stEchoMsg.usSendFlag = MSG_ECHO_SEND_WAIT;
                _echo_msg_map.insert(make_pair(stEchoMsg.txsHash, stEchoMsg));
            }
        }
        return processOk;
    }

    bool PktManager::getMsgInfo(MsgInfo &msg_info) {
        for (auto propose_it = _proposer_msg_map.begin(); propose_it != _proposer_msg_map.end(); propose_it++) {
            if (propose_it->second.usSendFlag == MSG_ECHO_NEED_SEND) {
                propose_it->second.usSendFlag = MSG_ECHO_SEND_OK;
                //make echo msg
                msg_info.phase = propose_it->second.phase;
                msg_info.eType = MSG_TYPE_ECHO;
                msg_info.proposerProof = propose_it->second.proof;
                msg_info.proposerSignature = propose_it->second.signature;
                msg_info.txsHash = propose_it->second.txsHash;
                return true;
            }
        }
        for (auto echo_it = _echo_msg_map.begin(); echo_it != _echo_msg_map.end(); echo_it++) {
            if (echo_it->second.usSendFlag == MSG_ECHO_NEED_SEND) {
                echo_it->second.usSendFlag = MSG_ECHO_SEND_OK;
                //make echo msg
                msg_info.phase = echo_it->second.phase;
                msg_info.eType = MSG_TYPE_ECHO;
                msg_info.proposerProof = echo_it->second.proposerProof;
                msg_info.proposerSignature = echo_it->second.proposerSignature;
                msg_info.txsHash = echo_it->second.txsHash;
                return true;
            }
        }
        return false;
    }

    bool PktManager::getMsgForBA(MsgInfo &stMsgInfo) {
        if (!preBA.txsHash.empty()) {
            stMsgInfo.phase = PHASE_BA1;
            stMsgInfo.eType = MSG_TYPE_ECHO;
            stMsgInfo.proposerProof = preBA.proposerProof;
            stMsgInfo.proposerSignature = preBA.proposerSignature;
            stMsgInfo.txsHash = preBA.txsHash;
            stMsgInfo.txs = preBA.txs;
        } else {
            stMsgInfo.phase = PHASE_BA1;
            stMsgInfo.eType = MSG_TYPE_EMPTY;
        }
        return true;
    }

    bool PktManager::MsgInit(MsgInfo &stMsgInfo) {
        string strHostName;
        uint8_t proof[64] = {0}; // security::VRF_PROOF_LEN = 64
        uint8_t txshash[32] = {0}; // security::SHA256_DIGEST_LEN = 32
        stMsgInfo.phase = PHASE_BA0;
        stMsgInfo.eType = MSG_TYPE_PROPOSE;
        stMsgInfo.txs = "txs from " + Connect::s_local_host;

        if (ultrain::security::vrf_prove(proof, (unsigned char *) stMsgInfo.txs.data(), stMsgInfo.txs.length(),
                                         uranus_private_key)) {
            std::cout << "vrf_prove" << std::endl;
            return false;
        }

        if (ultrain::security::sha256((unsigned char *) stMsgInfo.txs.data(), stMsgInfo.txs.length(), txshash)) {
            std::cout << "sha256" << std::endl;
            return false;
        }

        stMsgInfo.txsHash = (char *) txshash;
        stMsgInfo.proof = (char *) proof;
        stMsgInfo.proposerProof = (char *) proof;
        stMsgInfo.proposerSignature = (char *) uranus_public_key;
        stMsgInfo.signature = (char *) uranus_public_key;
        strLocalSign = stMsgInfo.signature;

        std::cout << "msg_init!!!" << std::endl;
        std::cout << "txs::" << stMsgInfo.txs << std::endl;
        return true;
    }

    bool PktManager::inToBA() {
        std::map<string, ProposeMsg>::iterator propose_it;
        std::map<string, EchoMsg>::iterator echo_it;
        string strMinKey;
        string strMinProof;
        cout << "into BA round." << endl;

        if (!_echo_msg_map.size()) {
            return false;
        }

        for (echo_it = _echo_msg_map.begin(); echo_it != _echo_msg_map.end(); echo_it++) {
            if (echo_it->second.signpool.size() >= THRESHOLD_NEXT_ROUND) {
                if (strMinKey.empty()) {
                    strMinKey = echo_it->second.txsHash;
                    strMinProof = echo_it->second.proposerProof;
                } else if (echo_it->second.proposerProof < strMinProof) {
                    strMinKey = echo_it->second.txsHash;
                    strMinProof = echo_it->second.proposerProof;
                }
            }
        }

        //produce empty block
        if (strMinKey.empty()) {
            return false;
        }

        echo_it = _echo_msg_map.find(strMinKey);
        propose_it = _proposer_msg_map.find(strMinKey);

        if ((echo_it == _echo_msg_map.end()) || (propose_it == _proposer_msg_map.end())) {
            return false;
        }
        preBA.proposerProof = echo_it->second.proposerProof;
        preBA.proposerSignature = echo_it->second.proposerSignature;
        preBA.signpool = echo_it->second.signpool;
        preBA.txsHash = echo_it->second.txsHash;
        preBA.txs = propose_it->second.txs;
        return true;
    }

    bool PktManager::isFinishBA() {
        std::map<string, ProposeMsg>::iterator propose_it;
        std::map<string, EchoMsg>::iterator echo_it;
        string strMinKey;
        string strMinProof;

        cout << "finish BA" << endl;

        if (!_echo_msg_map.size()) {
            return false;
        }

        for (echo_it = _echo_msg_map.begin(); echo_it != _echo_msg_map.end(); echo_it++) {
            if (echo_it->second.signpool.size() >= THRESHOLD_NEXT_ROUND) {
                if (strMinKey.empty()) {
                    strMinKey = echo_it->second.txsHash;
                    strMinProof = echo_it->second.proposerProof;
                } else if (echo_it->second.proposerProof < strMinProof) {
                    strMinKey = echo_it->second.txsHash;
                    strMinProof = echo_it->second.proposerProof;
                }
            }

        }

        //produce empty block
        if (strMinKey.empty()) {
            return false;
        }

        echo_it = _echo_msg_map.find(strMinKey);
        propose_it = _proposer_msg_map.find(strMinKey);

        if ((echo_it == _echo_msg_map.end()) || (propose_it == _proposer_msg_map.end())) {
            return false;
        }

        preblock.proposerProof = echo_it->second.proposerProof;
        preblock.proposerSignature = echo_it->second.proposerSignature;
        preblock.signpool = echo_it->second.signpool;
        preblock.txsHash = echo_it->second.txsHash;
        preblock.txs = propose_it->second.txs;

        return true;
    }

    void PktManager::block() {
        if (!preblock.txsHash.empty()) {
            cout << "block begin!!!" << endl;
            cout << "proof::" << preblock.proposerProof << endl;
            cout << "proposeSign::" << preblock.proposerSignature << endl;
            cout << "txs::" << preblock.txs << endl;
            cout << "txshash::" << preblock.txsHash << endl;

            for (auto sign_it = preblock.signpool.begin(); sign_it != preblock.signpool.end(); sign_it++) {
                cout << "signature" << *sign_it << endl;
            }
        } else {
            cout << "block is empty!!!" << endl;
        }
    }

}  // namespace ultrain


