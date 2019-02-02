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
       //(when = header.timestamp).slot++;
         when = header.timestamp;
	 when.abstime += config::block_interval_ms/1000;
    }
    result.header.timestamp                                = when;
    result.header.previous                                 = id;
    // TODO(yufengshen) : Clean this one.
    account_name producer_name = "ultrainio";
    result.block_num                                       = block_num + 1;
    result.irreversible_blocknum            = block_num;

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
    //ULTRAIN_ASSERT( h.header_extensions.size() == 0, block_validate_exception, "no supported extensions" );

    ULTRAIN_ASSERT( h.timestamp > header.timestamp, block_validate_exception, "block must be later in time" );
    ULTRAIN_ASSERT( h.previous == id, unlinkable_block_exception, "block must link to current state" );
    auto result = generate_next( h.timestamp );

     /// below this point is state changes that cannot be validated with headers alone, but never-the-less,
     /// must result in header state changes

    result.header.action_mroot       = h.action_mroot;
    result.header.transaction_mroot  = h.transaction_mroot;
    result.header.committee_mroot    = h.committee_mroot;
    result.header.proposer           = h.proposer;
    result.header.header_extensions  = h.header_extensions;
#ifdef CONSENSUS_VRF
    result.header.proposerProof      = h.proposerProof;
#endif
    result.id                        = result.header.id();
    /*
    ilog("----block_header_state::next ${time} ${pro} ${bn} ${prev} ${tx_mroot} ${action_mroot} ${hash}  ${ver} ${producer}",
	 ("time",result.header.timestamp)("pro", result.header.producer) ("bn", result.header.block_num())
	 ("prev", result.header.previous)("tx_mroot",result.header.transaction_mroot)("action_mroot",result.header.action_mroot)
	 ("ver", result.header.version)
	 ("producer", (bool)(result.header.new_producers))
	 ("hash",result.header.id()));

#ifdef CONSENSUS_VRF
    fprintf(stdout, "ppk %s ppf %s\n", result.header.proposerPk.c_str(), result.header.proposerProof.c_str());
#else
    fprintf(stdout, "ppk %s\n", result.header.proposerPk.c_str());
#endif

    fflush(stdout);
    */

    return result;
  } /// next

  digest_type   block_header_state::sig_digest()const {
      return header.digest();
  }

} } /// namespace ultrainio::chain


