#pragma once

#include <fc/reflect/reflect.hpp>
#include <ultrainio/chain/block.hpp>
#include <crypto/Signature.h>

namespace ultrainio {

    using Block = chain::signed_block;
    using BlockHeader = chain::block_header;
    using SignBlockHeader = chain::signed_block_header;

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
        uint32_t timestamp;
        Block block;
    };

    struct UnsignedEchoMsg {
        BlockHeader blockHeader;
        ConsensusPhase phase;
        uint32_t    baxCount;
        std::string pk; // hex string
        std::string proof;
    };

    struct EchoMsg : public UnsignedEchoMsg {
        uint32_t timestamp;
        std::string signature; // hex string
    };

    // aggregate echo msg
    struct UnsignedAggEchoMsg {
        BlockHeader blockHeader;
        std::vector<std::string> pkPool;
        std::vector<std::string> proofPool;
        ConsensusPhase phase;
        uint32_t baxCount;
        std::string pk; // hex string
        std::string proof;
    };

    struct AggEchoMsg : public UnsignedAggEchoMsg {
        std::string signature; // hex string
    };
}

FC_REFLECT( ultrainio::ProposeMsg, (timestamp)(block))
FC_REFLECT( ultrainio::UnsignedEchoMsg, (blockHeader)(phase)(baxCount)(pk)(proof))
FC_REFLECT_DERIVED( ultrainio::EchoMsg, (ultrainio::UnsignedEchoMsg), (timestamp)(signature))
FC_REFLECT( ultrainio::SyncRequestMessage, (startBlockNum)(endBlockNum) )
FC_REFLECT( ultrainio::ReqLastBlockNumMsg, (seqNum))
FC_REFLECT( ultrainio::RspLastBlockNumMsg, (seqNum)(blockNum)(blockHash)(prevBlockHash))
FC_REFLECT( ultrainio::UnsignedAggEchoMsg, (blockHeader)(pkPool)(proofPool)(phase)(baxCount)(pk)(proof))
FC_REFLECT_DERIVED( ultrainio::AggEchoMsg, (ultrainio::UnsignedAggEchoMsg), (signature))
