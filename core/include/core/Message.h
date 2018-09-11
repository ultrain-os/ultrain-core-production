#pragma once

#include <fc/reflect/reflect.hpp>
#include <ultrainio/chain/block.hpp>
#include <crypto/Signature.h>
#include <core/Redefined.h>

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
        Block block;
    };

    struct UnsignedEchoMsg {
        BlockIdType blockId;
        ConsensusPhase phase;
        uint32_t    proposerPriority;
        uint32_t    baxCount;
        uint32_t    timestamp;
        AccountName account;
        std::string proof;
    };

    struct EchoMsg : public UnsignedEchoMsg {
        std::string signature; // hex string
    };

    // aggregate echo msg
    struct UnsignedAggEchoMsg {
        BlockIdType blockId;
        uint32_t    proposerPriority;
        std::vector<AccountName> accountPool;
        std::vector<std::string> proofPool;
        std::vector<std::string> sigPool;
        std::vector<uint32_t> timePool;
        ConsensusPhase phase;
        uint32_t baxCount;
        AccountName account;
        // the proof of the node which send AggEchoMsg
        std::string myProposerProof;
    };

    struct AggEchoMsg : public UnsignedAggEchoMsg {
        std::string signature; // hex string
    };
}

FC_REFLECT( ultrainio::ProposeMsg, (block))
FC_REFLECT( ultrainio::UnsignedEchoMsg, (blockId)(phase)(proposerPriority)(baxCount)(timestamp)(account)(proof))
FC_REFLECT_DERIVED( ultrainio::EchoMsg, (ultrainio::UnsignedEchoMsg), (signature))
FC_REFLECT( ultrainio::SyncRequestMessage, (startBlockNum)(endBlockNum) )
FC_REFLECT( ultrainio::ReqLastBlockNumMsg, (seqNum))
FC_REFLECT( ultrainio::RspLastBlockNumMsg, (seqNum)(blockNum)(blockHash)(prevBlockHash))
FC_REFLECT( ultrainio::UnsignedAggEchoMsg, (blockId)(proposerPriority)(accountPool)(proofPool)(sigPool)(timePool)
                                           (phase)(baxCount)(account)(myProposerProof))
FC_REFLECT_DERIVED( ultrainio::AggEchoMsg, (ultrainio::UnsignedAggEchoMsg), (signature))
