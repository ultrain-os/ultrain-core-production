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

    void system_contract::regproducer( const account_name producer, const std::string& producer_key, const std::string& bls_key, account_name rewards_account, const std::string& url, uint64_t location ) {
        ultrainio_assert( url.size() < 512, "url too long" );
        // key is hex encoded
        ultrainio_assert( producer_key.size() == 64, "public key should be of size 64" );
        require_auth( producer );
        auto briefprod = _briefproducers.find(producer);
        if(briefprod == _briefproducers.end()) {
            //new producer, add to disabled table
            ultrainio_assert( is_account( rewards_account ), "rewards account not exists" );
            disabled_producers_table dp_tbl(_self, _self);
            dp_tbl.emplace( [&]( disabled_producer& dis_prod ) {
                dis_prod.owner                   = producer;
                dis_prod.producer_key            = producer_key;
                dis_prod.bls_key                 = bls_key;
                dis_prod.total_cons_staked       = 0;
                dis_prod.url                     = url;
                dis_prod.total_produce_block     = 0;
                dis_prod.location                = location;
                dis_prod.last_operate_blocknum   = 0;
                dis_prod.delegated_cons_blocknum = 0;
                dis_prod.claim_rewards_account   = rewards_account;
            });
            _briefproducers.emplace([&]( producer_brief& brief_prod ) {
                brief_prod.owner        = producer;
                brief_prod.location     = location;
                brief_prod.in_disable   = true;
            });
        }
        else {
            ultrainio_assert(location != default_chain_name, "default name is not allowed when updating producer");
            if (briefprod->location != location) {
                _briefproducers.modify(briefprod, [&](producer_brief& producer_brf) {
                    producer_brf.location = location;
                });
            }
            if (briefprod->in_disable) {
                disabled_producers_table dp_tbl(_self, _self);
                auto it_disable = dp_tbl.find(producer);
                dp_tbl.modify(it_disable, [&]( disabled_producer& dis_prod ) {
                    dis_prod.producer_key            = producer_key;
                    dis_prod.bls_key                 = bls_key;
                    dis_prod.url                     = url;
                    dis_prod.location                = location;
                });
            }
            else {
                if(location != master_chain_name) {
                    auto ite_chain = _subchains.find(location);
                    ultrainio_assert(ite_chain != _subchains.end(), "wrong location, subchain is not existed");
                }
                //if location changes
                producers_table _producers(_self, briefprod->location);
                auto prod = _producers.find(producer);
                ultrainio_assert(prod != _producers.end(), "producer is not found");
                if(prod->location != location) {
                    ultrainio_assert(!prod->is_on_master_chain(), "cannot move producers from master chain");
                    add_to_chain(location, *prod);
                    remove_from_chain(prod->location, prod->owner);
                }
                else {
                    _producers.modify( prod, [&]( producer_info& info ) {
                        info.producer_key = producer_key;
                        info.bls_key      = bls_key;
                        info.url          = url;
                        info.location     = location;
                        //TODO, is the bloew check needed?
                    });
                }
            }
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
      ultrainio_assert( get_enable_producers_number() > _gstate.min_committee_member_number, " The number of committees is about to be smaller than the minimum number and therefore cannot be unregprod" );
      if(has_auth(_self)){
         require_auth(_self);
      } else{
         ultrainio_assert( _gstate.is_master_chain(), "only master chain allow unregprod" );
      }
      auto briefprod = _briefproducers.find(producer);
      ultrainio_assert(briefprod != _briefproducers.end(), "this account is not a producer");
      ultrainio_assert(!briefprod->in_disable, "this producer has been unregistered before");

      producers_table _producers(_self, briefprod->location);
      const auto& prod = _producers.find( producer );
      ultrainio_assert(prod != _producers.end(), "producer is not found in its location");
      uint64_t curblocknum = (uint64_t)head_block_number() + 1;
      ultrainio_assert( (curblocknum - prod->last_operate_blocknum) > 2 , "interval operate at least more than two number block high" );

      disabled_producers_table dp_tbl(_self, _self);
      dp_tbl.emplace( [&]( disabled_producer& dis_prod ) {
         dis_prod.owner                   = prod->owner;
         dis_prod.producer_key            = prod->producer_key;
         dis_prod.bls_key                 = prod->bls_key;
         dis_prod.total_cons_staked       = prod->total_cons_staked;
         dis_prod.url                     = prod->url;
         dis_prod.total_produce_block     = prod->total_produce_block;
         dis_prod.location                = prod->location;
         dis_prod.last_operate_blocknum   = curblocknum;
         dis_prod.delegated_cons_blocknum = prod->delegated_cons_blocknum;
         dis_prod.claim_rewards_account   = prod->claim_rewards_account;
      });
      remove_from_chain(briefprod->location, producer);
      //pay unpaid_balance
      if(prod->unpaid_balance > 0) {
         claimrewardtoaccount(prod->claim_rewards_account, asset((int64_t)prod->unpaid_balance));
      }
      _producers.erase(prod);
/*
      if(prod.is_on_pending_chian()) {
          remove_from_pendingchain(producer);
      }
      else if(prod.is_on_subchain()) {
          //todo
      } */
   }
   inline uint32_t system_contract::get_enable_producers_number(){
      uint32_t  enableprodnum = 0;
      producers_table _producers(_self, master_chain_name);
      for(auto itr = _producers.begin(); itr != _producers.end(); ++itr){
         ++enableprodnum;
      }
      return enableprodnum;
   }

    std::vector<uint64_t> system_contract::getallchainname() {
        std::vector<uint64_t> scopes;
        scopes.emplace_back(master_chain_name);
        for(auto ite_chain = _subchains.begin(); ite_chain != _subchains.end(); ++ite_chain) {
            scopes.emplace_back(ite_chain->chain_name);
        }
        return scopes;
    }
} /// namespace ultrainiosystem
