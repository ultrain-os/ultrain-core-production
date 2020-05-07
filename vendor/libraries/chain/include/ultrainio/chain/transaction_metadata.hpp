/**
 *  @file
 *  @copyright defined in ultrain/LICENSE.txt
 */
#pragma once
#include <ultrainio/chain/transaction.hpp>
#include <ultrainio/chain/block.hpp>
#include <ultrainio/chain/trace.hpp>

namespace ultrainio { namespace chain {

/**
 *  This data structure should store context-free cached data about a transaction such as
 *  packed/unpacked/compressed and recovered keys
 */
class transaction_metadata {
   public:
      transaction_id_type                                        id;
      transaction_id_type                                        signed_id;
      signed_transaction                                         trx;
      packed_transaction                                         packed_trx;
      optional<pair<chain_id_type, flat_set<public_key_type>>>   signing_keys;
      bool                                                       accepted = false;

      transaction_metadata( const signed_transaction& t, packed_transaction::compression_type c = packed_transaction::none )
      :trx(t),packed_trx(t, c) {
         id = trx.id();
         //raw_packed = fc::raw::pack( static_cast<const transaction&>(trx) );
         signed_id = digest_type::hash(packed_trx);
      }

      transaction_metadata( const packed_transaction& ptrx )
      :trx( ptrx.get_signed_transaction() ), packed_trx(ptrx) {
         id = trx.id();
         //raw_packed = fc::raw::pack( static_cast<const transaction&>(trx) );
         signed_id = digest_type::hash(packed_trx);
      }

      const flat_set<public_key_type>& recover_keys(const chain_id_type& chain_id,
                                                    map<signature_type, public_key_type>* sig_to_key_map) {
          if(!signing_keys || signing_keys->first != chain_id ) // Unlikely for more than one chain_id to be used in one nodultrain instance
              signing_keys = std::make_pair( chain_id, trx.get_signature_keys( chain_id, sig_to_key_map ) );
          return signing_keys->second;
      }

      uint32_t total_actions()const { return trx.context_free_actions.size() + trx.actions.size(); }
};

using transaction_metadata_ptr = std::shared_ptr<transaction_metadata>;

} } // ultrainio::chain
