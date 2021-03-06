/**
 *  @file
 *  @copyright defined in ultrain/LICENSE.txt
 */
#pragma once
#include <ultrainio/chain/types.hpp>
#include <ultrainio/chain/authority.hpp>
#include <ultrainio/chain/block_timestamp.hpp>
#include <ultrainio/chain/abi_def.hpp>

#include "multi_index_includes.hpp"

namespace ultrainio { namespace chain {

   class account_object : public chainbase::object<account_object_type, account_object> {
      OBJECT_CTOR(account_object,(chain_names)(code)(abi))

      id_type              id;
      account_name         name;
      uint8_t              vm_type      = 0;
      uint8_t              vm_version   = 0;
      bool                 privileged   = false;
      bool                 updateable   = false;

      time_point           last_code_update;
      digest_type          code_version;
      block_timestamp_type creation_date;
      shared_vector<ultrainio::chain::name>  chain_names;

      shared_string  code;
      shared_string  abi;

      void set_abi( const ultrainio::chain::abi_def& a ) {
         abi.resize( fc::raw::pack_size( a ) );
         fc::datastream<char*> ds( abi.data(), abi.size() );
         fc::raw::pack( ds, a );
      }

      ultrainio::chain::abi_def get_abi()const {
         ultrainio::chain::abi_def a;
         ULTRAIN_ASSERT( abi.size() != 0, abi_not_found_exception, "No ABI set on account ${n}", ("n",name) );

         fc::datastream<const char*> ds( abi.data(), abi.size() );
         fc::raw::unpack( ds, a );
         return a;
      }
   };
   using account_id_type = account_object::id_type;

   struct by_name;
   using account_index = chainbase::shared_multi_index_container<
      account_object,
      indexed_by<
         ordered_unique<tag<by_id>, member<account_object, account_object::id_type, &account_object::id>>,
         ordered_unique<tag<by_name>, member<account_object, account_name, &account_object::name>>
      >
   >;

   class account_sequence_object : public chainbase::object<account_sequence_object_type, account_sequence_object>
   {
      OBJECT_CTOR(account_sequence_object);

      id_type      id;
      account_name name;
      uint64_t     recv_sequence = 0;
      uint64_t     auth_sequence = 0;
      uint64_t     code_sequence = 0;
      uint64_t     abi_sequence  = 0;
   };

   struct by_name;
   using account_sequence_index = chainbase::shared_multi_index_container<
      account_sequence_object,
      indexed_by<
         ordered_unique<tag<by_id>, member<account_sequence_object, account_sequence_object::id_type, &account_sequence_object::id>>,
         ordered_unique<tag<by_name>, member<account_sequence_object, account_name, &account_sequence_object::name>>
      >
   >;

   class auth_sequence_object : public chainbase::object<auth_sequence_object_type, auth_sequence_object>
   {
      OBJECT_CTOR(auth_sequence_object);

      id_type      id;
      account_name name;
      uint8_t      auth_sequence = 0;
   };

   struct by_name;
   using auth_sequence_index = chainbase::shared_multi_index_container<
      auth_sequence_object,
      indexed_by<
         ordered_unique<tag<by_id>, member<auth_sequence_object, auth_sequence_object::id_type, &auth_sequence_object::id>>,
         ordered_unique<tag<by_name>, member<auth_sequence_object, account_name, &auth_sequence_object::name>>
      >
   >;

} } // ultrainio::chain

CHAINBASE_SET_INDEX_TYPE(ultrainio::chain::account_object, ultrainio::chain::account_index)
CHAINBASE_SET_INDEX_TYPE(ultrainio::chain::account_sequence_object, ultrainio::chain::account_sequence_index)
CHAINBASE_SET_INDEX_TYPE(ultrainio::chain::auth_sequence_object, ultrainio::chain::auth_sequence_index)

FC_REFLECT(ultrainio::chain::account_object,(name)(vm_type)(vm_version)(privileged)(updateable)(last_code_update)(code_version)(creation_date)(chain_names)(code)(abi))
FC_REFLECT(ultrainio::chain::account_sequence_object, (name)(recv_sequence)(auth_sequence)(code_sequence)(abi_sequence))
FC_REFLECT(ultrainio::chain::auth_sequence_object, (name)(auth_sequence))
