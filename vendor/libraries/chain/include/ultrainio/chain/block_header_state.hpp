#pragma once
#include <ultrainio/chain/block_header.hpp>
#include <ultrainio/chain/incremental_merkle.hpp>

namespace ultrainio { namespace chain {

/**
 *  @struct block_header_state
 *  @brief defines the minimum state necessary to validate transaction headers
 */
struct block_header_state {
    block_id_type                     id;
    uint32_t                          block_num = 0;
    signed_block_header               header;
    uint32_t                          dpos_proposed_irreversible_blocknum = 0;
    uint32_t                          dpos_irreversible_blocknum = 0;
    uint32_t                          bft_irreversible_blocknum = 0;
    incremental_merkle                blockroot_merkle;
    public_key_type                   block_signing_key;
    vector<uint8_t>                   confirm_count;

    block_header_state   next( const signed_block_header& h, bool trust = false )const;
    block_header_state   generate_next( block_timestamp_type when )const;

    void set_confirmed( uint16_t num_prev_blocks );

    const block_id_type& prev()const { return header.previous; }
    digest_type          sig_digest()const;
};



} } /// namespace ultrainio::chain

FC_REFLECT( ultrainio::chain::block_header_state,
            (id)(block_num)(header)(dpos_proposed_irreversible_blocknum)(dpos_irreversible_blocknum)(bft_irreversible_blocknum)
            (blockroot_merkle)(block_signing_key)(confirm_count) )
