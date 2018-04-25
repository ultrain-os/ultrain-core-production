/**
 *  @file
 *  @copyright defined in ultrain/LICENSE.txt
 */

#include <ultrainio/chain/get_config.hpp>
#include <ultrainio/chain/config.hpp>
#include <ultrainio/chain/types.hpp>

namespace ultrainio { namespace chain {

fc::variant_object get_config()
{
   fc::mutable_variant_object result;

//   result["block_interval_ms"] = config::block_interval_ms;
//   result["producer_count"] = config::producer_count;
   /// TODO: add extra config parms
   return result;
}

} } // ultrainio::chain
