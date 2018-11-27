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
    uint32_t                          irreversible_blocknum = 0;

    block_header_state   next( const signed_block_header& h, bool trust = false )const;
    block_header_state   generate_next( block_timestamp_type when )const;

    const block_id_type& prev()const { return header.previous; }
    digest_type          sig_digest()const;
};

} } /// namespace ultrainio::chain

FC_REFLECT( ultrainio::chain::block_header_state,
            (id)(block_num)(header)(irreversible_blocknum)
          )
