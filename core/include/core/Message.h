#pragma once

#include <fc/reflect/reflect.hpp>
#include <fc/variant.hpp>
#include <ultrainio/chain/block.hpp>
#include <crypto/Signature.h>

#include "core/ExtType.h"
#include "core/types.h"

namespace ultrainio {

    enum ConsensusPhase {
        kPhaseInit = 0,
        kPhaseBA0,
        kPhaseBA1,
        kPhaseBAX
    };

    typedef std::vector<ExtType> MsgExtension;

    struct ReqSyncMsg {
        uint32_t seqNum;
        uint32_t startBlockNum;
        uint32_t endBlockNum;
        MsgExtension ext;
    };

    struct ReqBlockNumRangeMsg {
        uint32_t seqNum;
        MsgExtension ext;
    };

    struct RspBlockNumRangeMsg {
        uint32_t seqNum;
        uint32_t firstNum;
        uint32_t lastNum;
        std::string blockHash;
        std::string prevBlockHash;
        MsgExtension ext;
    };

    struct SyncBlockMsg {
        uint32_t seqNum;
        Block block;
        std::string proof;
        MsgExtension ext;
    };

    struct SyncStopMsg {
        uint32_t seqNum;
        MsgExtension ext;
    };

    // propose message
    struct UnsignedProposeMsg {
        Block block;
        MsgExtension ext;
    };

    struct ProposeMsg : public UnsignedProposeMsg {
        std::string signature;
    };

    // echo message
    struct CommonEchoMsg {
        BlockIdType blockId;
        ConsensusPhase phase;
        uint32_t    baxCount;
        AccountName proposer;
        void toStringStream(std::stringstream& ss) const;

        bool fromStringStream(std::stringstream& ss);

        bool operator == (const CommonEchoMsg&) const;

        uint32_t blockNum() const;
    };

    struct UnsignedEchoMsg : public CommonEchoMsg {
        std::string blsSignature;
        AccountName account;
        uint32_t    timestamp;
        MsgExtension ext;
        bool operator == (const UnsignedEchoMsg&) const;
    };

    struct EchoMsg : public UnsignedEchoMsg {
        std::string signature; // hex string
        bool operator == (const EchoMsg&) const;
    };
}

FC_REFLECT( ultrainio::UnsignedProposeMsg, (block)(ext))
FC_REFLECT_DERIVED( ultrainio::ProposeMsg, (ultrainio::UnsignedProposeMsg), (signature))

FC_REFLECT( ultrainio::CommonEchoMsg, (blockId)(phase)(baxCount)(proposer))
FC_REFLECT_DERIVED( ultrainio::UnsignedEchoMsg, (ultrainio::CommonEchoMsg), (blsSignature)(account)(timestamp)(ext))

FC_REFLECT_DERIVED( ultrainio::EchoMsg, (ultrainio::UnsignedEchoMsg), (signature))
FC_REFLECT( ultrainio::ReqSyncMsg, (seqNum)(startBlockNum)(endBlockNum)(ext))
FC_REFLECT( ultrainio::ReqBlockNumRangeMsg, (seqNum)(ext))
FC_REFLECT( ultrainio::RspBlockNumRangeMsg, (seqNum)(firstNum)(lastNum)(blockHash)(prevBlockHash)(ext))
FC_REFLECT( ultrainio::SyncBlockMsg, (seqNum)(block)(proof)(ext))
FC_REFLECT( ultrainio::SyncStopMsg, (seqNum)(ext))
