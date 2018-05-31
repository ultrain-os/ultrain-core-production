#pragma once

#include <stdint.h>

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "define.hpp"

namespace ultrainio {

    class UranusNode;

    struct ProposeMsg {
        consensus_phase phase;
        std::string txs;
        std::string txs_hash;
        std::string txs_signature;
        std::string proposer_pk; //public key
        std::string proposer_role_vrf;
    };

    struct EchoMsg {
        consensus_phase phase;
        std::string txs_hash;
        std::string txs_signature;
        std::string proposer_pk; //proposer public key
        std::string proposer_role_vrf;
        std::vector<std::string> pk_pool; //public key pool
    };

    struct TxsBlock {
        std::string txs;
        std::string txs_hash;
        std::string txs_signature;
        std::string proposer_pk; //proposer public key
        std::string proposer_role_vrf;
        std::vector<std::string> pk_pool; //public key pool
        void clear() {
            txs.clear();
            txs_hash.clear();
            txs_signature.clear();
            proposer_pk.clear();
            proposer_role_vrf.clear();
            pk_pool.clear();
        }
    };

    struct MsgInfo {
        uint16_t type;
        uint16_t phase;
        uint32_t block_id;
        std::string txs;
        std::string txs_signature;
        std::string proposer_pk; //proposer public key
        std::string proposer_role_vrf;
        std::string txs_hash;
        std::string txs_hash_signature;
        std::string pk; //public key
        std::string role_vrf;

        MsgInfo() : type(MSG_TYPE_NOMSG), phase(PHASE_INIT), block_id(0), txs(), txs_signature(), proposer_pk(),
                    proposer_role_vrf(),txs_hash(), txs_hash_signature(), pk(), role_vrf() {
        }
    };

    class PktManager : public std::enable_shared_from_this<PktManager> {
    public:
        static MsgInfo construct_msg_info(const TxsBlock& block);

        PktManager(UranusNode* node);

        void reset();

        // format msg
        void processMsg(const char* buf, size_t size, uint16_t local_phase, uint32_t local_block_id);

        bool is_min_propose(const ProposeMsg& propose_msg);

        bool is_min_2f_echo(const EchoMsg& echo_msg);

        TxsBlock produce_tentative_block();

        bool formatMsg(MsgInfo &msg_info, const char* buf, size_t size, uint16_t local_phase, uint32_t local_block_id);

        bool MsgInit(MsgInfo &stMsgInfo);

        bool isValidMsg(MsgInfo &stMsgInfo, std::string &signature);

        bool insertMsg(MsgInfo &stMsgInfo);

    private:
        UranusNode* _node;
        std::map<std::string, ProposeMsg> _proposer_msg_map;
        std::map<std::string, EchoMsg> _echo_msg_map;
    };
}
