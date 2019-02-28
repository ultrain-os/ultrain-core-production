/**
 *  @file
 *  @copyright defined in ultrain/LICENSE.txt
 */
#include "ultrainio.system.hpp"

#include <ultrainiolib/ultrainio.hpp>
#include <ultrainiolib/crypto.h>
#include <ultrainiolib/print.hpp>
#include <ultrainiolib/datastream.hpp>
#include <ultrainiolib/serialize.hpp>
#include <ultrainiolib/multi_index.hpp>
#include <ultrainiolib/privileged.hpp>
#include <ultrainiolib/singleton.hpp>
#include <ultrainiolib/transaction.hpp>
#include <ultrainio.token/ultrainio.token.hpp>

#include <algorithm>
#include <cmath>

namespace ultrainiosystem {
   using ultrainio::indexed_by;
   using ultrainio::const_mem_fun;
   using ultrainio::bytes;
   using ultrainio::print;
   using ultrainio::singleton;
   using ultrainio::transaction;

   /**
    *  This method will create a producer_config and producer_info object for 'producer'
    *
    *  @pre producer is not already registered
    *  @pre producer to register is an account
    *  @pre authority of producer to register
    *
    */
    void system_contract::regproducer( const account_name producer, const std::string& producer_key, const std::string& bls_key, account_name rewards_account, const std::string& url, uint64_t location ) {
      ultrainio_assert( url.size() < 512, "url too long" );
      // key is hex encoded
      ultrainio_assert( producer_key.size() == 64, "public key should be of size 64" );

      require_auth( producer );
      if (location == master_chain_name || location == default_chain_name) {
          if(location == default_chain_name) {
              auto ite_chain = _subchains.begin();
              auto ite_min = _subchains.end();
              uint32_t min_committee_size = std::numeric_limits<uint32_t>::max();
              for(; ite_chain != _subchains.end(); ++ite_chain) {
                  uint32_t my_committee_num = ite_chain->committee_members.size();
                  if(my_committee_num < min_committee_size ) {
                      min_committee_size = my_committee_num;
                      ite_min = ite_chain;
                  }
              }
              ultrainio_assert(ite_min != _subchains.end(), "there's not any sidechain existed" );
              location = ite_min->chain_name;
          }
          //require_auth( producer );
      }
      else {
          auto ite_chain = _subchains.find(uint64_t(location) );
          ultrainio_assert(ite_chain != _subchains.end(), "wrong location, subchain is not existed");
          //require_auth( N(ultrainio) ); //pending que or specified subchain
      }

      auto prod = _producers.find( producer );
      if( prod != _producers.end() ) {
         //if location changes
         if(prod->is_enabled && prod->location != location) {
             ultrainio_assert(!prod->is_in_pending_queue(), "cannot move producers in pending queue");
             ultrainio_assert(!prod->is_on_master_chain(), "cannot move producers from master chain");
             if(prod->is_on_subchain()) {
                 remove_from_subchain(prod->location, prod->owner);
             }
             if(location != master_chain_name && location != pending_queue) {
                 add_to_subchain(location, prod->owner, prod->producer_key, prod->bls_key);
             }
             else if(location == pending_queue) {
                 add_to_pending_queue(prod->owner, prod->producer_key, prod->bls_key);
             }
         }
         _producers.modify( prod, [&]( producer_info& info ){
               info.producer_key = producer_key;
               info.bls_key  = bls_key;
               info.url          = url;
               info.location     = location;
               if(info.total_cons_staked >= _gstate.min_activated_stake)
                    info.is_enabled = true;
               else
                    info.is_enabled = false;  
         });
      } else {
         ultrainio_assert( is_account( rewards_account ), "rewards account not exists" );
         _producers.emplace( [&]( producer_info& info ){
               info.owner         = producer;
               info.total_cons_staked   = 0LL;
               info.producer_key  = producer_key;
               info.bls_key  = bls_key;
               info.is_enabled    = false;
               info.url           = url;
               info.location      = location;
               info.claim_rewards_account = rewards_account;
         });
      }
      if(has_auth(_self)){
         require_auth(_self);
      } else{
         ultrainio_assert( _gstate.is_master_chain(), "only master chain allow regproducer" );
         asset cur_tokens = ultrainio::token(N(utrio.token)).get_balance( producer,symbol_type(CORE_SYMBOL).name());
         ultrainio_assert( cur_tokens.amount >= 50000, "The current action fee is 5 UGAS, please ensure that the account is fully funded" );
         INLINE_ACTION_SENDER(ultrainio::token, safe_transfer)( N(utrio.token), {producer,N(active)},
            { producer, N(utrio.fee), asset(50000), std::string("regproducer") } );
      }
   }

   void system_contract::unregprod( const account_name producer ) {
      require_auth( producer );
      if(has_auth(_self)){
         require_auth(_self);
      } else{
         ultrainio_assert( _gstate.is_master_chain(), "only master chain allow unregprod" );
      }
      const auto& prod = _producers.get( producer, "producer not found" );
      uint64_t curblocknum = (uint64_t)head_block_number() + 1;
      print("unregprod curblocknum:",curblocknum," last_operate_blocknum:",prod.last_operate_blocknum);

      ultrainio_assert( (curblocknum - prod.last_operate_blocknum) > 2 , "interval operate at least more than two number block high" );
      _producers.modify( prod, [&]( producer_info& info ){
            info.set_disabled();
            info.last_operate_blocknum = curblocknum;
      });
/*
      if(prod.is_on_pending_chian()) {
          remove_from_pendingchain(producer);
      }
      else if(prod.is_on_subchain()) {
          //todo
      } */
   }


} /// namespace ultrainiosystem
