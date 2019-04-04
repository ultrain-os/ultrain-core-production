#pragma once

#include <fc/reflect/reflect.hpp>
#include <fc/variant.hpp>
#include <ultrainio/chain/block.hpp>
#include <crypto/Signature.h>
#include <core/types.h>

namespace ultrainio {

    enum ConsensusPhase {
        kPhaseInit = 0,
        kPhaseBA0,
        kPhaseBA1,
        kPhaseBAX
    };

    struct ExtType {
        uint32_t key;
        std::string value;
    };

    typedef std::vector<ExtType> MsgExtension;

    struct ReqSyncMsg {
        uint32_t seqNum;
        uint32_t startBlockNum;
        uint32_t endBlockNum;
    };

    struct ReqBlockNumRangeMsg {
        uint32_t seqNum;
    };

    struct RspBlockNumRangeMsg {
        uint32_t seqNum;
        uint32_t firstNum;
        uint32_t lastNum;
        std::string blockHash;
        std::string prevBlockHash;
    };

    struct SyncBlockMsg {
        uint32_t seqNum;
        Block block;
        std::string proof;
    };

    struct SyncStopMsg {
        uint32_t seqNum;
    };

    struct UnsignedProposeMsg {
        Block block;
        MsgExtension ext;
    };

    struct ProposeMsg : public UnsignedProposeMsg {
        std::string signature;
    };

    struct CommonEchoMsg {
        BlockIdType blockId;
        ConsensusPhase phase;
        uint32_t    baxCount;
#ifdef CONSENSUS_VRF
        uint32_t    proposerPriority;
#else
        AccountName proposer;
#endif
        void toStringStream(std::stringstream& ss) const;

        bool fromStringStream(std::stringstream& ss);

        bool operator == (const CommonEchoMsg&) const;
    };

    struct UnsignedEchoMsg : public CommonEchoMsg {
        std::string blsSignature;
        AccountName account;
        uint32_t    timestamp;
#ifdef CONSENSUS_VRF
        std::string proof;
#endif
        MsgExtension ext;
    };

    struct EchoMsg : public UnsignedEchoMsg {
        std::string signature; // hex string
    };
}

FC_REFLECT( ultrainio::ExtType, (key)(value))
FC_REFLECT( ultrainio::UnsignedProposeMsg, (block)(ext))
FC_REFLECT_DERIVED( ultrainio::ProposeMsg, (ultrainio::UnsignedProposeMsg), (signature))

#ifdef CONSENSUS_VRF
FC_REFLECT( ultrainio::CommonEchoMsg, (blockId)(phase)(baxCount)(proposerPriority))
FC_REFLECT_DERIVED( ultrainio::UnsignedEchoMsg, (ultrainio::CommonEchoMsg), (blsSignature)(account)(timestamp)(proof)(ext))
#else
FC_REFLECT( ultrainio::CommonEchoMsg, (blockId)(phase)(baxCount)(proposer))
FC_REFLECT_DERIVED( ultrainio::UnsignedEchoMsg, (ultrainio::CommonEchoMsg), (blsSignature)(account)(timestamp)(ext))
#endif

FC_REFLECT_DERIVED( ultrainio::EchoMsg, (ultrainio::UnsignedEchoMsg), (signature))
FC_REFLECT( ultrainio::ReqSyncMsg, (seqNum)(startBlockNum)(endBlockNum) )
FC_REFLECT( ultrainio::ReqBlockNumRangeMsg, (seqNum))
FC_REFLECT( ultrainio::RspBlockNumRangeMsg, (seqNum)(firstNum)(lastNum)(blockHash)(prevBlockHash))
FC_REFLECT( ultrainio::SyncBlockMsg, (seqNum)(block)(proof))
FC_REFLECT( ultrainio::SyncStopMsg, (seqNum))
