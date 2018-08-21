#pragma once

#include <fc/reflect/reflect.hpp>
#include <ultrainio/chain/block.hpp>

namespace ultrainio {

    using Block = chain::signed_block;
    using BlockHeader = chain::signed_block_header;

    enum ConsensusPhase {
        kPhaseInit = 0,
        kPhaseBA0,
        kPhaseBA1,
        kPhaseBAX
    };

    struct SyncRequestMessage {
        uint32_t startBlockNum;
        uint32_t endBlockNum;
    };

    struct ReqLastBlockNumMsg {
        uint32_t seqNum;    
    };

    struct RspLastBlockNumMsg {
        uint32_t seqNum;
        uint32_t blockNum;
        std::string blockHash;
        std::string prevBlockHash;
    };

    struct ProposeMsg {
        Block block;
    };

    struct EchoMsg {
        BlockHeader blockHeader;
        ConsensusPhase phase;
        uint32_t    baxCount;
        std::string pk;
        std::string proof;
        std::string signature;
    };

    // aggregate echo msg
    struct AggEchoMsg {
        BlockHeader blockHeader;
        std::vector<std::string> pkPool;
        std::vector<std::string> proofPool;
        ConsensusPhase phase;
        uint32_t baxCount;
        std::string pk;
        std::string proof;
        std::string signature;
    };
}

FC_REFLECT( ultrainio::ProposeMsg, (block) )
FC_REFLECT( ultrainio::EchoMsg, (blockHeader)(phase)(baxCount)(pk)(proof)(signature) )
FC_REFLECT( ultrainio::SyncRequestMessage, (startBlockNum)(endBlockNum) )
FC_REFLECT( ultrainio::ReqLastBlockNumMsg, (seqNum))
FC_REFLECT( ultrainio::RspLastBlockNumMsg, (seqNum)(blockNum)(blockHash)(prevBlockHash))
FC_REFLECT( ultrainio::AggEchoMsg, (blockHeader)(pkPool)(proofPool)(phase)(baxCount)(pk)(proof)(signature))
