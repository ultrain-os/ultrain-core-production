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
    void system_contract::regproducer( const account_name producer, const std::string& producer_key, const std::string& url, uint64_t location, account_name rewards_account ) {
      ultrainio_assert( url.size() < 512, "url too long" );
      // key is hex encoded
      ultrainio_assert( producer_key.size() == 64, "public key should be of size 64" );
      require_auth( producer );

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
                 add_to_subchain(location, prod->owner, prod->producer_key);
             }
             else if(location == pending_queue) {
                 add_to_pending_queue(prod->owner, prod->producer_key);
             }
             else {
                 if(!prod->hasactived) {
                     update_activated_stake(prod->total_cons_staked);
                     _producers.modify(prod, 0 , [&](auto & v) {
                         v.hasactived = true;
                     });
                 }
             }
         }
         _producers.modify( prod, producer, [&]( producer_info& info ){
               info.producer_key = producer_key;
               info.is_active    = true;
               info.url          = url;
               info.location     = location;
         });
      } else {
         ultrainio_assert( is_account( rewards_account ), "rewards account not exists" );
         _producers.emplace( producer, [&]( producer_info& info ){
               info.owner         = producer;
               info.total_cons_staked   = 0LL;
               info.producer_key  = producer_key;
               info.is_active     = true;
               info.is_enabled    = false;
               info.url           = url;
               info.location      = location;
               info.claim_rewards_account = rewards_account;
         });
      }
      if(has_auth(_self)){
         require_auth(_self);
      } else{
         INLINE_ACTION_SENDER(ultrainio::token, transfer)( N(utrio.token), {producer,N(active)},
            { producer, N(utrio.fee), asset(50000), std::string("regproducer") } );
      }
   }

   void system_contract::unregprod( const account_name producer ) {
      require_auth( producer );

      const auto& prod = _producers.get( producer, "producer not found" );
      uint64_t curblocknum = (uint64_t)tapos_block_num();
      print("unregprod tapos_block_num:",curblocknum," prod->last_operate_blocknum:",prod.last_operate_blocknum);

      ultrainio_assert( (curblocknum - prod.last_operate_blocknum) > 2 , "interval operate at least more than two number block high" );
      _producers.modify( prod, 0, [&]( producer_info& info ){
            info.deactivate();
            info.is_enabled = false;
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

   inline void system_contract::update_activated_stake(int64_t stake) {
       _gstate.total_activated_stake += stake;
         if( _gstate.total_activated_stake >= _gstate.min_activated_stake && _gstate.thresh_activated_stake_time == 0 ) {
            _gstate.thresh_activated_stake_time = current_time();
         }

   }

} /// namespace ultrainiosystem
