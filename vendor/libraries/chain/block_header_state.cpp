#include <ultrainio/chain/block_header_state.hpp>
#include <ultrainio/chain/exceptions.hpp>
#include <limits>

namespace ultrainio { namespace chain {

  /**
   *  Generate a template block header state for a given block time, it will not
   *  contain a transaction mroot, action mroot as those components
   *  are derived from chain state.
   */
  block_header_state block_header_state::generate_next( block_timestamp_type when )const {
    block_header_state result;

    if( when != block_timestamp_type() ) {
       ULTRAIN_ASSERT( when > header.timestamp, block_validate_exception, "next block must be in the future" );
    } else {
       (when = header.timestamp).slot++;
    }
    result.header.timestamp                                = when;
    result.header.previous                                 = id;
    // TODO(yufengshen) : Clean this one.
    account_name producer_name = "ultrainio";
    //    result.block_signing_key                               = prokey.block_signing_key;
    result.block_num                                       = block_num + 1;
    result.blockroot_merkle = blockroot_merkle;
    result.blockroot_merkle.append( id );

    auto block_mroot = result.blockroot_merkle.get_root();

    // TODO(yufengshen): Check this
    result.dpos_proposed_irreversible_blocknum   = block_num;
    result.bft_irreversible_blocknum             = block_num;
    result.dpos_irreversible_blocknum            = block_num;

    return result;
  } /// generate_next

  /**
   *  Transitions the current header state into the next header state given the supplied signed block header.
   *
   *  Given a signed block header, generate the expected template based upon the header time,
   *  then validate that the provided header matches the template.
   *
   */
  block_header_state block_header_state::next( const signed_block_header& h, bool trust )const {
    ULTRAIN_ASSERT( h.timestamp != block_timestamp_type(), block_validate_exception, "", ("h",h) );
    ULTRAIN_ASSERT( h.header_extensions.size() == 0, block_validate_exception, "no supported extensions" );

    ULTRAIN_ASSERT( h.timestamp > header.timestamp, block_validate_exception, "block must be later in time" );
    ULTRAIN_ASSERT( h.previous == id, unlinkable_block_exception, "block must link to current state" );
    auto result = generate_next( h.timestamp );

    // FC_ASSERT( result.header.block_mroot == h.block_mroot, "mismatch block merkle root" );

     /// below this point is state changes that cannot be validated with headers alone, but never-the-less,
     /// must result in header state changes

    // TODO(yufengshen) : always confirming 1 for now.
    result.set_confirmed(1);

    result.header.action_mroot       = h.action_mroot;
    result.header.transaction_mroot  = h.transaction_mroot;
    result.header.committee_mroot    = h.committee_mroot;
    result.header.proposer           = h.proposer;
    result.header.proposerProof      = h.proposerProof;
    result.id                        = result.header.id();
    /*
    ilog("----block_header_state::next ${time} ${pro} ${bn} ${prev} ${tx_mroot} ${action_mroot} ${hash}  ${ver} ${producer}",
	 ("time",result.header.timestamp)("pro", result.header.producer) ("bn", result.header.block_num())
	 ("prev", result.header.previous)("tx_mroot",result.header.transaction_mroot)("action_mroot",result.header.action_mroot)
	 ("ver", result.header.version)
	 ("producer", (bool)(result.header.new_producers))
	 ("hash",result.header.id()));

    fprintf(stdout, "ppk %s ppf %s\n", result.header.proposerPk.c_str(), result.header.proposerProof.c_str());
    fflush(stdout);
    */
    // TODO - yufengshen - we should check this
    //    if( !trust ) {
    //       FC_ASSERT( result.block_signing_key == result.signee(), "block not signed by expected key",
    //                  ("result.block_signing_key", result.block_signing_key)("signee", result.signee() ) );
    //    }

    return result;
  } /// next

  void block_header_state::set_confirmed( uint16_t num_prev_blocks ) {
     /*
     idump((num_prev_blocks)(confirm_count.size()));

     for( uint32_t i = 0; i < confirm_count.size(); ++i ) {
        std::cerr << "confirm_count["<<i<<"] = " << int(confirm_count[i]) << "\n";
     }
     */

     int32_t i = (int32_t)(confirm_count.size() - 1);
     uint32_t blocks_to_confirm = num_prev_blocks + 1; /// confirm the head block too
     while( i >= 0 && blocks_to_confirm ) {
        --confirm_count[i];
        //idump((confirm_count[i]));
        if( confirm_count[i] == 0 )
        {
           uint32_t block_num_for_i = block_num - (uint32_t)(confirm_count.size() - 1 - i);
           dpos_proposed_irreversible_blocknum = block_num_for_i;
           //idump((dpos2_lib)(block_num)(dpos_irreversible_blocknum));

           if (i == confirm_count.size() - 1) {
              confirm_count.resize(0);
           } else {
              memmove( &confirm_count[0], &confirm_count[i + 1], confirm_count.size() - i  - 1);
              confirm_count.resize( confirm_count.size() - i - 1 );
           }

           return;
        }
        --i;
        --blocks_to_confirm;
     }
  }

  digest_type   block_header_state::sig_digest()const {
      return digest_type::hash(std::make_pair(header.digest(), blockroot_merkle.get_root()));
  }

} } /// namespace ultrainio::chain
