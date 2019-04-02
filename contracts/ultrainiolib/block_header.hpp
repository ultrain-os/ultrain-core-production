/**
 *  @file
 *  @copyright defined in ultrain/LICENSE.txt
 */
#pragma once
#include <ultrainiolib/crypto.h>
#include <ultrainiolib/types.hpp>
#include <ultrainiolib/datastream.hpp>
#include <ultrainiolib/time.hpp>
#include <ultrainiolib/serialize.hpp>
#include <iostream>
#include <vector>
#include <string>

namespace ultrainio {

    using namespace std;

    uint32_t endian_reverse_u32( uint32_t x ) {
       return (((x >> 0x18) & 0xFF)        )
            | (((x >> 0x10) & 0xFF) << 0x08)
            | (((x >> 0x08) & 0xFF) << 0x10)
            | (((x        ) & 0xFF) << 0x18);
    }

    struct block_header {
      block_timestamp_type             timestamp;
      account_name                     proposer;
#ifdef CONSENSUS_VRF
      std::string                      proposerProof;
#endif
      uint32_t                         version = 0;
      block_id_type                    previous;
      checksum256                      transaction_mroot; /// mroot of cycles_summary
      checksum256                      action_mroot; /// mroot of all delivered action receipts
      checksum256                      committee_mroot;
      extensions_type                  header_extensions;

#ifdef CONSENSUS_VRF
      ULTRAINLIB_SERIALIZE( block_header, (timestamp)(proposer)(proposerProof)(version)(previous)
                                          (transaction_mroot)(action_mroot)(committee_mroot)(header_extensions) )
#else
      ULTRAINLIB_SERIALIZE( block_header, (timestamp)(proposer)(version)(previous)
                                          (transaction_mroot)(action_mroot)(committee_mroot)(header_extensions) )
#endif

      checksum256     digest() const {
          bytes block_stream = pack(*this);
          checksum256 hash;
          sha256(block_stream.data(), block_stream.size(), &hash);
          return hash;
      }

      block_id_type     id() const {
          // Do not include signed_block_header attributes in id, specifically exclude producer_signature.
          block_id_type result = digest(); //fc::sha256::hash(*static_cast<const block_header*>(this));
          uint64_t _hash64;
          memcpy(&_hash64, result.hash, sizeof(uint64_t) );
          _hash64 &= 0xffffffff00000000;
          _hash64 += endian_reverse_u32(block_num()); // store the block num in the ID, 160 bits is plenty for the hash
          memcpy(result.hash, &_hash64, sizeof(uint64_t) );
          return result;
      }

      uint32_t          block_num() const {
          return num_from_id(previous) + 1;
      }

      static uint32_t   num_from_id(const block_id_type& id) {
          uint64_t _hash64;
          memcpy(&_hash64, id.hash, sizeof(uint64_t) );
          return endian_reverse_u32(uint32_t(_hash64));
      }
    };

   struct signed_block_header : public block_header
   {
      std::string       signature;

      ULTRAINLIB_SERIALIZE_DERIVED(signed_block_header, block_header, (signature))
   };
}
