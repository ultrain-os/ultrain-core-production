/**
 *  @file
 *  @copyright defined in ultrain/LICENSE.txt
 */

#include <ultrainio/chain/chain_id_type.hpp>
#include <ultrainio/chain/exceptions.hpp>

namespace ultrainio { namespace chain {

   void chain_id_type::reflector_verify()const {
      ULTRAIN_ASSERT( *reinterpret_cast<const fc::sha256*>(this) != fc::sha256(), chain_id_type_exception, "chain_id_type cannot be zero" );
   }

} }  // namespace ultrainio::chain

namespace fc {

   void to_variant(const ultrainio::chain::chain_id_type& cid, fc::variant& v) {
      to_variant( static_cast<const fc::sha256&>(cid), v);
   }

   void from_variant(const fc::variant& v, ultrainio::chain::chain_id_type& cid) {
      from_variant( v, static_cast<fc::sha256&>(cid) );
   }

} // fc
