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

    struct ReqSyncMsg {
        uint32_t seqNum;
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

    // Keep account & signature in the struct and we will use them soon.
    struct SyncBlockMsg {
        uint32_t seqNum;
        AccountName account;
        std::string signature;
        Block block;
    };

    struct SyncStopMsg {
        uint32_t seqNum;
        AccountName account;
        std::string signature;
    };

    struct ProposeMsg {
        Block block;
    };

    struct UnsignedEchoMsg {
        BlockIdType blockId;
        ConsensusPhase phase;
        uint32_t    baxCount;
        uint32_t    timestamp;
        AccountName account;
#ifdef CONSENSUS_VRF
        uint32_t    proposerPriority;
        std::string proof;
#else
        AccountName proposer;
#endif
    };

    struct EchoMsg : public UnsignedEchoMsg {
        std::string signature; // hex string
    };

    // aggregate echo msg
    struct UnsignedAggEchoMsg {
        BlockIdType blockId;
        std::vector<AccountName> accountPool;
        std::vector<std::string> sigPool;
        std::vector<uint32_t> timePool;
        ConsensusPhase phase;
        uint32_t baxCount;
        AccountName account;
#ifdef CONSENSUS_VRF
        uint32_t    proposerPriority;
        std::vector<std::string> proofPool;
        // the proof of the node which send AggEchoMsg
        std::string myProposerProof;
#else
        AccountName proposer;
#endif
    };

    struct AggEchoMsg : public UnsignedAggEchoMsg {
        std::string signature; // hex string
    };
}

FC_REFLECT( ultrainio::ProposeMsg, (block))
#ifdef CONSENSUS_VRF
FC_REFLECT( ultrainio::UnsignedEchoMsg, (blockId)(phase)(baxCount)(timestamp)(account)(proposerPriority)(proof))
#else
FC_REFLECT( ultrainio::UnsignedEchoMsg, (blockId)(phase)(baxCount)(timestamp)(account)(proposer))
#endif
FC_REFLECT_DERIVED( ultrainio::EchoMsg, (ultrainio::UnsignedEchoMsg), (signature))
FC_REFLECT( ultrainio::ReqSyncMsg, (seqNum)(startBlockNum)(endBlockNum) )
FC_REFLECT( ultrainio::ReqLastBlockNumMsg, (seqNum))
FC_REFLECT( ultrainio::RspLastBlockNumMsg, (seqNum)(blockNum)(blockHash)(prevBlockHash))
FC_REFLECT( ultrainio::SyncBlockMsg, (seqNum)(block))
FC_REFLECT( ultrainio::SyncStopMsg, (seqNum))
#ifdef CONSENSUS_VRF
FC_REFLECT( ultrainio::UnsignedAggEchoMsg, (blockId)(accountPool)(sigPool)(timePool)(phase)(baxCount)
                                           (account)(proposerPriority)(proofPool)(myProposerProof))
#else
FC_REFLECT( ultrainio::UnsignedAggEchoMsg, (blockId)(accountPool)(sigPool)(timePool)(phase)(baxCount)(account)(proposer))
#endif
FC_REFLECT_DERIVED( ultrainio::AggEchoMsg, (ultrainio::UnsignedAggEchoMsg), (signature))
