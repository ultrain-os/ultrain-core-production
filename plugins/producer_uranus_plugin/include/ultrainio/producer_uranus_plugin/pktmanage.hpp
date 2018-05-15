#pragma once

#include <stdint.h>

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "define.hpp"

namespace ultrain {

    struct ProposeMsg {
        uint16_t phase;
        unsigned short usSendFlag;
        std::string txs;
        std::string txsHash;
        std::string proof;
        std::string signature; //public key
    };

    struct EchoMsg {
        uint16_t phase;
        unsigned short usSendFlag;
        std::string txsHash;
        std::string proposerProof;
        std::string proposerSignature;//proposer public key
        std::vector<std::string> signpool;//public key pool
    };

    struct TxsBlock {
        std::string txs;
        std::string txsHash;
        std::string proposerProof;
        std::string proposerSignature; //proposer public key
        std::vector<std::string> signpool; //public key pool
        void clear() {
            txs.clear();
            txsHash.clear();
            proposerProof.clear();
            proposerSignature.clear();
            signpool.clear();
        }
    };

    struct MsgInfo {
        unsigned short eType;
        uint16_t phase;
        std::string txsHash;
        std::string proposerProof;
        std::string proposerSignature; //proposer public key
        std::string proof;
        std::string signature; //public key
        std::string txs;

        MsgInfo() : eType(MSG_TYPE_NOMSG), phase(PHASE_INIT), txsHash(), proposerProof(), proposerSignature(), proof(),
                    signature(), txs() {
        }
    };

    class PktManager : public std::enable_shared_from_this<PktManager> {
    public:

        PktManager();

        void reset();

        //format msg
        processresult processMsg(const char* buf, size_t size, unsigned short local_round);

        bool getMsgInfo(MsgInfo &stMsgInfo);

        bool inToBA();

        bool isFinishBA();

        void block();

        bool formatMsg(MsgInfo &msg_info, const char* buf, size_t size, unsigned short local_round);

        bool MsgInit(MsgInfo &stMsgInfo);

        bool getMsgForBA(MsgInfo &stMsgInfo);

        bool isValidMsg(MsgInfo &stMsgInfo, std::string &signature);

        bool insertMsg(MsgInfo &stMsgInfo);

    private:
        std::map<std::string, ProposeMsg> _proposer_msg_map;
        std::map<std::string, EchoMsg> _echo_msg_map;
        TxsBlock preblock;
        TxsBlock preBA;
    };
}
