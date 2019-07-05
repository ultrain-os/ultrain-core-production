#pragma once
#include <ultrainio/chain/block_timestamp.hpp>

namespace ultrainio { namespace chain {

   struct block_header
   {
      block_timestamp_type             timestamp;
      account_name                     proposer;
      uint32_t                         version = 0;
      block_id_type                    previous;
      checksum256_type                 transaction_mroot; /// mroot of cycles_summary
      checksum256_type                 action_mroot; /// mroot of all delivered action receipts
      checksum256_type                 committee_mroot;
      extensions_type                  header_extensions;

      digest_type       digest()const;
      block_id_type     id() const;
      uint32_t          block_num() const { return num_from_id(previous) + 1; }
      static uint32_t   num_from_id(const block_id_type& id);
   };


   struct signed_block_header : public block_header
   {
      std::string       signature;
   };

} } /// namespace ultrainio::chain

FC_REFLECT(ultrainio::chain::block_header,
           (timestamp)(proposer)(version)
           (previous)(transaction_mroot)(action_mroot)(committee_mroot)
           (header_extensions))

FC_REFLECT_DERIVED(ultrainio::chain::signed_block_header, (ultrainio::chain::block_header), (signature))
