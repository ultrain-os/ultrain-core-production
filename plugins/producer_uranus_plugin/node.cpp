#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <chrono>

#include <ultrainio/producer_uranus_plugin/define.hpp>
#include <ultrainio/producer_uranus_plugin/node.hpp>
#include <ultrainio/producer_uranus_plugin/pktmanage.hpp>
#include <ultrainio/producer_uranus_plugin/connect.hpp>
#include <ultrainio/producer_uranus_plugin/security.hpp>

uint8_t uranus_public_key[32];  // TODO
uint8_t uranus_private_key[64]; // TODO

using namespace boost::asio;
using namespace std;

namespace ultrain {

    const auto roundTimeout = boost::chrono::milliseconds{5000};

    bool UranusNode::isTimeout() {
        boost::chrono::steady_clock::time_point timenow = boost::chrono::steady_clock::now();
        auto round_dur = boost::chrono::duration_cast<boost::chrono::milliseconds>(timenow - start);

        if (round_dur > roundTimeout)
            return true;
        return false;
    }

    processresult UranusNode::Listen() {
        char recv_buffer[BUFSIZE] = {0};
        std::size_t dataLenth = 0;
        processresult eResult = processOk;
        MsgInfo stMsgInfo;

        while (true) {
            cout << "begin rcv." << endl;
            dataLenth = connection.rcvMsg(recv_buffer, BUFSIZE);
            if (dataLenth > 0) {
                eResult = pktmng.processMsg(recv_buffer, dataLenth, _phase);
                if ((eResult == IntoBA) || (eResult == Block)) {
                    return eResult;
                } else if ((eResult == SendMsg) && (_role != LISTENER)) {
                    pktmng.getMsgInfo(stMsgInfo);
                    sendMsg(stMsgInfo);
                }
            }

            if (isTimeout()) {
                std::cout << "timeout " << std::endl;
                if (_phase == PHASE_BA0) {
                    pktmng.inToBA();
                    return IntoBA;
                } else {
                    pktmng.isFinishBA();
                    return Block;
                }
            }
        }
    }

    void UranusNode::startup() {
        _phase = PHASE_BA0;
        _role = PROPOSER;
        processresult eResult = processOk;
        MsgInfo stMsgInfo;

        start = boost::chrono::steady_clock::now();

        if (_role == PROPOSER) {
            pktmng.MsgInit(stMsgInfo);
            stMsgInfo.eType = MSG_TYPE_PROPOSE;
            insertMsg(stMsgInfo);
            sendMsg(stMsgInfo);

            stMsgInfo.eType = MSG_TYPE_ECHO;
            sendMsg(stMsgInfo);
        }
#if 0
        else if (_role == VOTER)
        {
            pktmng.MsgInit(stMsgInfo);
            stMsgInfo.eType = MSG_TYPE_ECHO;
            sendMsg(stMsgInfo);
        }
#endif
        eResult = Listen();
        std::cout << "BA0 ok." << endl;
        _role = VOTER;
        _phase = PHASE_BA1;

        if (eResult == IntoBA) {
            pktmng.getMsgForBA(stMsgInfo);
            sendMsg(stMsgInfo);
        }

        start = boost::chrono::steady_clock::now();

        eResult = Listen();
        cout << "BA1 ok." << endl;
        pktmng.block();
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
        char *pTmp = NULL;
        size_t size = 0;

        pTmp = msg;

        if (stMsgInfo.eType == MSG_TYPE_NOMSG)
            return false;

        //type
        *(unsigned short *) pTmp = stMsgInfo.eType;
        pTmp += sizeof(unsigned short);

        //phase
        *(uint16_t*) pTmp = stMsgInfo.phase;
        pTmp += sizeof(uint16_t);

        //vetor proof
        *(unsigned int *) pTmp = PROOF;
        pTmp += sizeof(unsigned int);

        *(unsigned int *) pTmp = stMsgInfo.proof.length();
        pTmp += sizeof(unsigned int);

        memcpy(pTmp, stMsgInfo.proof.c_str(), stMsgInfo.proof.length());
        pTmp += stMsgInfo.proof.length();

        //vetor sign
        *(unsigned int *) pTmp = SIGN;
        pTmp += sizeof(unsigned int);

        *(unsigned int *) pTmp = stMsgInfo.signature.length();
        pTmp += sizeof(unsigned int);

        memcpy(pTmp, stMsgInfo.signature.c_str(), stMsgInfo.signature.length());
        pTmp += stMsgInfo.signature.length();

        if (stMsgInfo.eType == MSG_TYPE_EMPTY) {
            //empty
            *(unsigned int *) pTmp = TXSEMPTY;
            pTmp += sizeof(unsigned int);

            //tail
            *(unsigned int *) pTmp = TAIL;
            pTmp += sizeof(unsigned int);

            size = pTmp - msg;

            if (size >= BUFSIZE) {
                return false;
            }
            connection.sendBrdCast(msg, size);
            return true;
        }

        //proposer proof
        *(unsigned int *) pTmp = PROPOSERPROOF;
        pTmp += sizeof(unsigned int);

        *(unsigned int *) pTmp = stMsgInfo.proposerProof.length();
        pTmp += sizeof(unsigned int);

        memcpy(pTmp, stMsgInfo.proposerProof.c_str(), stMsgInfo.proposerProof.length());
        pTmp += stMsgInfo.proposerProof.length();

        //proposer sign
        *(unsigned int *) pTmp = PROPOSERSIGN;
        pTmp += sizeof(unsigned int);

        *(unsigned int *) pTmp = stMsgInfo.proposerSignature.length();
        pTmp += sizeof(unsigned int);

        memcpy(pTmp, stMsgInfo.proposerSignature.c_str(), stMsgInfo.proposerSignature.length());
        pTmp += stMsgInfo.proposerSignature.length();

        //hash
        *(unsigned int *) pTmp = TXSHASH;
        pTmp += sizeof(unsigned int);

        *(unsigned int *) pTmp = stMsgInfo.txsHash.length();
        pTmp += sizeof(unsigned int);

        memcpy(pTmp, stMsgInfo.txsHash.c_str(), stMsgInfo.txsHash.length());
        pTmp += stMsgInfo.txsHash.length();

        if (stMsgInfo.eType == MSG_TYPE_PROPOSE) {
            //txs
            *(unsigned int *) pTmp = TXS;
            pTmp += sizeof(unsigned int);

            *(unsigned int *) pTmp = stMsgInfo.txs.length();
            pTmp += sizeof(unsigned int);

            memcpy(pTmp, stMsgInfo.txs.c_str(), stMsgInfo.txs.length());
            pTmp += stMsgInfo.txs.length();
        }

        //tail
        *(unsigned int *) pTmp = TAIL;
        pTmp += sizeof(unsigned int);
        size = pTmp - msg;

        if (size >= BUFSIZE) {
            return false;
        }
        connection.sendBrdCast(msg, size);
        return true;
    }
} 


