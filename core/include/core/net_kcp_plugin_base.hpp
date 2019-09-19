#pragma once


#include <fc/network/message_buffer.hpp>
#include <fc/network/ip.hpp>
#include <fc/io/json.hpp>
#include <fc/io/raw.hpp>
#include <fc/log/appender.hpp>
#include <fc/container/flat.hpp>
#include <fc/reflect/variant.hpp>
#include <fc/crypto/rand.hpp>
#include <fc/exception/exception.hpp>

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/host_name.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/intrusive/set.hpp>

#include <random>
#include <rpos/Genesis.h>
#include <rpos/MsgMgr.h>
#include <rpos/Utils.h>

#include <lightclient/LightClientCallback.h>
#include <lightclient/LightClient.h>
#include <lightclient/LightClientMgr.h>
#include <lightclient/StartPoint.h>

#if defined( __linux__ )
#include <malloc.h>
#endif
#include <p2p/NodeTable.h>
#include <rpos/StakeVoteBase.h>
#include <crypto/Bls.h>
#include <crypto/Signer.h>
#include <crypto/Validator.h>
#include <core/utils.h>

using namespace ultrainio::chain::plugin_interface::compat;

namespace ultrainio {

    using std::vector;

    using boost::asio::ip::tcp;
    using boost::asio::ip::address_v4;
    using boost::asio::ip::host_name;
    using boost::intrusive::rbtree;
    using boost::multi_index_container;

    using fc::time_point;
    using fc::time_point_sec;
    using ultrainio::chain::transaction_id_type;
    namespace bip = boost::interprocess;

    struct node_transaction_state {
        transaction_id_type id;
        time_point_sec  expires;  /// time after which this may be purged.
                                  /// Expires increased while the txn is
                                  /// "in flight" to anoher peer
        packed_transaction packed_txn;
        vector<char>       serialized_txn; /// the received raw bundle
        uint32_t           block_num = 0; /// block transaction was included in
        uint32_t           true_block = 0; /// used to reset block_uum when request is 0
        uint16_t           requests = 0; /// the number of "in flight" requests for this txn
    };

    struct update_in_flight {
        int32_t incr;
        update_in_flight (int32_t delta) : incr (delta) {}
        void operator() (node_transaction_state& nts) {
            int32_t exp = nts.expires.sec_since_epoch();
            nts.expires = fc::time_point_sec (exp + incr * 60);
            if( nts.requests == 0 ) {
                nts.true_block = nts.block_num;
                nts.block_num = 0;
            }
            nts.requests += incr;
            if( nts.requests == 0 ) {
                nts.block_num = nts.true_block;
            }
        }
    };

    struct by_expiry;
    struct by_block_num;

    typedef multi_index_container<
        node_transaction_state,
        indexed_by<
            ordered_unique<
               tag< by_id >,
               member < node_transaction_state,
                    transaction_id_type,
                    &node_transaction_state::id > >,
            ordered_non_unique<
                tag< by_expiry >,
                member< node_transaction_state,
                    fc::time_point_sec,
                    &node_transaction_state::expires > >,
            ordered_non_unique<
                tag<by_block_num>,
                member< node_transaction_state,
                    uint32_t,
                    &node_transaction_state::block_num > >
        >
    > node_transaction_index;

    enum connection_direction{
        direction_in,
        direction_out,
        direction_passive_out,
    };

    struct connection_status {
        string            peer;
        bool              connecting = false;
        handshake_message last_handshake;
    };

    /**
     *  Index by id
     *  Index by is_known, block_num, validated_time, this is the order we will broadcast
     *  to peer.
     *  Index by is_noticed, validated_time
     *
     */
    struct transaction_state {
        transaction_id_type id;
        bool                is_known_by_peer = false; ///< true if we sent or received this trx to this peer or received notice from peer
        bool                is_noticed_to_peer = false; ///< have we sent peer notice we know it (true if we receive from this peer)
        uint32_t            block_num = 0; ///< the block number the transaction was included in
        time_point_sec      expires;
        time_point          requested_time; /// in case we fetch large trx
    };

    struct update_txn_expiry {
        time_point_sec new_expiry;
        update_txn_expiry(time_point_sec e) : new_expiry(e) {}
        void operator() (transaction_state& ts) {
            ts.expires = new_expiry;
        }
    };

    typedef multi_index_container<
      transaction_state,
      indexed_by<
         ordered_unique< tag<by_id>, member<transaction_state, transaction_id_type, &transaction_state::id > >,
         ordered_non_unique< tag< by_expiry >, member< transaction_state,fc::time_point_sec,&transaction_state::expires >>,
         ordered_non_unique<
            tag<by_block_num>,
            member< transaction_state,
                    uint32_t,
                    &transaction_state::block_num > >
         >

      > transaction_state_index;


    struct peer_block_state {
        block_id_type id;
        uint32_t      block_num;
        bool          is_known;
        bool          is_noticed;
        time_point    requested_time;
    };

    struct update_request_time {
        void operator() (struct transaction_state &ts) {
            ts.requested_time = time_point::now();
        }
        void operator () (struct peer_block_state &bs) {
            bs.requested_time = time_point::now();
        }
    };

    typedef multi_index_container<
        peer_block_state,
        indexed_by<
            ordered_unique< tag<by_id>, member<peer_block_state, block_id_type, &peer_block_state::id > >,
            ordered_unique< tag<by_block_num>, member<peer_block_state, uint32_t, &peer_block_state::block_num > >
        >
    > peer_block_state_index;

    struct update_known_by_peer {
        void operator() (peer_block_state& bs) {
            bs.is_known = true;
        }
        void operator() (transaction_state& ts) {
            ts.is_known_by_peer = true;
        }
    };

    struct update_block_num {
        uint32_t new_bnum;
        update_block_num(uint32_t bnum) : new_bnum(bnum) {}
        void operator() (node_transaction_state& nts) {
            if (nts.requests ) {
                nts.true_block = new_bnum;
            }
            else {
                nts.block_num = new_bnum;
            }
        }
        void operator() (transaction_state& ts) {
            ts.block_num = new_bnum;
        }
        void operator() (peer_block_state& pbs) {
            pbs.block_num = new_bnum;
        }
    };

}  // namespace ultrainio

FC_REFLECT( ultrainio::connection_status, (peer)(connecting)(last_handshake) )
