/**
 *  @file
 *  @copyright defined in ultrain/LICENSE.txt
 */
#pragma once

#include <fc/container/flat_fwd.hpp>
#include <fc/io/varint.hpp>
#include <fc/io/enum_type.hpp>
#include <fc/crypto/sha224.hpp>
#include <fc/optional.hpp>
#include <fc/safe.hpp>
#include <fc/container/flat.hpp>
#include <fc/string.hpp>
#include <fc/io/raw.hpp>
#include <fc/static_variant.hpp>
#include <fc/smart_ref_fwd.hpp>
#include <fc/crypto/ripemd160.hpp>
#include <fc/fixed_string.hpp>

#include <memory>
#include <vector>
#include <deque>
#include <cstdint>

#include <chrono>

#include <ultrainio/chain/worldstate_file_manager.hpp>
#include <ultrainio/chain/block_log.hpp>
#include <ultrainio/chain/block_header_state.hpp>

namespace ultrainio {

    using namespace std;
    using namespace fc;

    static_assert(sizeof(std::chrono::system_clock::duration::rep) >= 8, "system_clock is expected to be at least 64 bits");
   typedef std::chrono::system_clock::duration::rep tstamp;

   namespace wss {

       struct handshake_message {
           uint16_t network_version = 0; ///< incremental value above a computed base
           fc::sha256 node_id; ///< used to identify peers and prevent self-connect
           tstamp time;
           fc::sha256 token; ///< digest of time to prove we own the private key of the key above
           string p2p_address;
           string os;
           string agent;
           int16_t generation;
       };

       enum go_away_reason {
           no_reason, ///< no reason to go away
           self, ///< the connection is to itself
           duplicate, ///< the connection is redundant
           wrong_chain, ///< the peer's chain id doesn't match
           wrong_version, ///< the peer's network version doesn't match
           forked, ///< the peer's irreversible blocks are different
           unlinkable, ///< the peer sent a block we couldn't use
           bad_transaction, ///< the peer sent a transaction that failed verification
           validation, ///< the peer sent a block that failed validation
           benign_other, ///< reasons such as a timeout. not fatal but warrant resetting
           fatal_other, ///< a catch-all for errors we don't have discriminated
           authentication ///< peer failed authenicatio
       };

       constexpr auto reason_str(go_away_reason rsn) {
           switch (rsn) {
               case no_reason :
                   return "no reason";
               case self :
                   return "self connect";
               case duplicate :
                   return "duplicate";
               case fatal_other :
                   return "some other failure";
               case benign_other :
                   return "some other non-fatal condition";
               default :
                   return "some crazy reason";
           }
       }

       struct go_away_message {
           go_away_message(go_away_reason r = no_reason) : reason(r), node_id() {}

           go_away_reason reason;
           fc::sha256 node_id; ///< for duplicate notification
       };

       typedef std::chrono::system_clock::duration::rep tstamp;
       typedef int32_t tdist;

       static_assert(sizeof(std::chrono::system_clock::duration::rep) >= 8,
                     "system_clock is expected to be at least 64 bits");

       struct time_message {
           tstamp org;       //!< origin timestamp
           tstamp rec;       //!< receive timestamp
           tstamp xmt;       //!< transmit timestamp
           mutable tstamp dst;       //!< destination timestamp
       };

       enum id_list_modes {
           none,
           catch_up,
           last_irr_catch_up,
           normal
       };

       constexpr auto modes_str(id_list_modes m) {
           switch (m) {
               case none :
                   return "none";
               case catch_up :
                   return "catch up";
               case last_irr_catch_up :
                   return "last irreversible";
               case normal :
                   return "normal";
               default:
                   return "undefined mode";
           }
       }

       template<typename T>
       struct select_ids {
           select_ids() : mode(none), pending(0), ids() {}

           id_list_modes mode;
           uint32_t pending;
           vector<T> ids;

           bool empty() const { return (mode == none || ids.empty()); }
       };

       const unsigned long MAX_PACKET_DATA_LENGTH		=	1024 * 1024;

       struct ReqLastWsInfoMsg {
            fc::sha256 chain_id;
            uint32_t block_height;
       };

       struct RspLastWsInfoMsg
       {
        //    uint32_t     fileSeqNum;
        //    long double  fileSize;
        //    std::string  fileName;
        //    std::string  fileHashString;
           ultrainio::chain::ws_info info;
       };

        struct ReqWsFileMsg {
            uint32_t lenPerSlice;
            uint32_t index;
            ultrainio::chain::ws_info info;
        };

        struct FileTransferPacket
        {
            unsigned long    chunkLen;
            std::string      chunkHashString;
            uint32_t sliceId;
            bool endOfFile = false;
            std::vector<char> chunk;
        };

        struct ReqBlocksInfoMsg {
            fc::sha256 chain_id;
            uint32_t block_height;
        };

        struct RspBlocksInfoMsg
        {
            fc::sha256 chain_id;
            uint32_t block_height;
            chain::genesis_state gs;
        };

        struct ReqBlocksFileMsg {
            uint32_t block_num;
        };

        struct BlocksTransferPacket
        {
            unsigned long    chunkLen;
            uint32_t block_num;
            // std::vector<char> chunk;
            chain::signed_block block;
        };

       struct TimeInfo{
           tstamp reqTime;
           tstamp rspTime;
       };

       struct ReqTestTimeMsg {
           TimeInfo timeInfo;
           uint32_t size;
           std::vector<char>    chunk;
       };

       struct RspTestTimeMsg {
           TimeInfo timeInfo;
           std::vector<char>    chunk;
       };

       using net_message = static_variant<handshake_message,
               go_away_message,
               time_message,
               ReqLastWsInfoMsg,
               RspLastWsInfoMsg,
               ReqWsFileMsg,
               FileTransferPacket,
               ReqTestTimeMsg,
               RspTestTimeMsg,
               ReqBlocksInfoMsg,
               RspBlocksInfoMsg,
               ReqBlocksFileMsg,
               BlocksTransferPacket>;
   }
} // namespace ultrainio

FC_REFLECT( ultrainio::wss::select_ids<fc::sha256>, (mode)(pending)(ids) )
FC_REFLECT( ultrainio::wss::handshake_message,
            (network_version)(node_id)(time)(p2p_address)
            (os)(agent)(generation) )
FC_REFLECT( ultrainio::wss::go_away_message, (reason)(node_id) )
FC_REFLECT( ultrainio::wss::time_message, (org)(rec)(xmt)(dst) )
FC_REFLECT( ultrainio::wss::ReqLastWsInfoMsg, (chain_id)(block_height) )
FC_REFLECT( ultrainio::wss::RspLastWsInfoMsg, (info) )
FC_REFLECT( ultrainio::wss::ReqWsFileMsg, (lenPerSlice)(index)(info) )
FC_REFLECT( ultrainio::wss::FileTransferPacket, (chunkLen)(chunkHashString)(sliceId)(endOfFile)(chunk) )
FC_REFLECT( ultrainio::wss::TimeInfo, (reqTime)(rspTime) )
FC_REFLECT( ultrainio::wss::ReqTestTimeMsg, (timeInfo)(size)(chunk) )
FC_REFLECT( ultrainio::wss::RspTestTimeMsg, (timeInfo)(chunk) )
FC_REFLECT( ultrainio::wss::ReqBlocksInfoMsg, (chain_id)(block_height) )
FC_REFLECT( ultrainio::wss::RspBlocksInfoMsg, (chain_id)(block_height)(gs) )
FC_REFLECT( ultrainio::wss::ReqBlocksFileMsg, (block_num) )
FC_REFLECT( ultrainio::wss::BlocksTransferPacket, (chunkLen)(block_num)(block) )
