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
    void system_contract::regproducer( const account_name producer, const std::string& producer_key, const std::string& url, uint64_t location ) {
      ultrainio_assert( url.size() < 512, "url too long" );
      // key is hex encoded
      ultrainio_assert( producer_key.size() == 64, "public key should be of size 64" );
      require_auth( producer );

      auto prod = _producers.find( producer );
      uint64_t curblocknum = tapos_block_num();
      if ( prod != _producers.end() ) {
         ultrainio_assert( (curblocknum - prod->last_operate_blocknum) > 2 , "interval operate at least more than certain number block high" );
         _producers.modify( prod, producer, [&]( producer_info& info ){
               info.producer_key = producer_key;
               info.is_active    = true;
               info.url          = url;
               info.location     = location;
               info.last_operate_blocknum = curblocknum;
            });
      } else {
         _producers.emplace( producer, [&]( producer_info& info ){
               info.owner         = producer;
               info.total_cons_staked   = 0LL;
               info.producer_key  = producer_key;
               info.is_active     = true;
               info.is_enabled    = false;
               info.url           = url;
               info.location      = location;
               info.last_operate_blocknum = curblocknum;
         });
      }
   }

   void system_contract::unregprod( const account_name producer ) {
      require_auth( producer );

      const auto& prod = _producers.get( producer, "producer not found" );
      uint64_t curblocknum = tapos_block_num();
      print("unregprod tapos_block_num:",curblocknum," prod->last_operate_blocknum:",prod.last_operate_blocknum);

      ultrainio_assert( (curblocknum - prod.last_operate_blocknum) > 2 , "interval operate at least more than certain number block high" );
      _producers.modify( prod, 0, [&]( producer_info& info ){
            info.deactivate();
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
