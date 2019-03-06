/**
 *  @file
 *  @copyright defined in ultrain/LICENSE.txt
 */
#include "ultrainio.system.hpp"

#include <ultrainiolib/crypto.h>
#include <ultrainiolib/datastream.hpp>
#include <ultrainiolib/serialize.hpp>
#include <ultrainiolib/transaction.hpp>
#include <ultrainio.token/ultrainio.token.hpp>

#include <algorithm>
#include <cmath>

namespace ultrainiosystem {
    using namespace ultrainio;

    void system_contract::regproducer(const account_name producer,
                                      const std::string& producer_key,
                                      const std::string& bls_key,
                                      account_name rewards_account,
                                      const std::string& url,
                                      uint64_t location ) {
        ultrainio_assert( url.size() < 512, "url too long" );
        // key is hex encoded
        ultrainio_assert( producer_key.size() == 64, "public key should be of size 64" );
        require_auth( producer );
        uint64_t curblocknum = (uint64_t)head_block_number() + 1;
        auto briefprod = _briefproducers.find(producer);
        if(briefprod == _briefproducers.end()) {
            //new producer, add to disabled table for now
            ultrainio_assert( is_account( rewards_account ), "rewards account not exists" );
            disabled_producers_table dp_tbl(_self, _self);
            dp_tbl.emplace( [&]( disabled_producer& dis_prod ) {
                dis_prod.owner                   = producer;
                dis_prod.producer_key            = producer_key;
                dis_prod.bls_key                 = bls_key;
                dis_prod.url                     = url;
                dis_prod.location                = location;
                dis_prod.last_operate_blocknum   = curblocknum;
                dis_prod.delegated_cons_blocknum = 0;
                dis_prod.claim_rewards_account   = rewards_account;
            });
            _briefproducers.emplace([&]( producer_brief& brief_prod ) {
                brief_prod.owner        = producer;
                brief_prod.location     = location;
                brief_prod.in_disable   = true;
            });
        } else {
            ultrainio_assert(location != default_chain_name, "default name is not allowed when updating producer");
            if (briefprod->location != location) {
                _briefproducers.modify(briefprod, [&](producer_brief& producer_brf) {
                    producer_brf.location = location;
                });
            }
            if (briefprod->in_disable) {
                disabled_producers_table dp_tbl(_self, _self);
                auto it_disable = dp_tbl.find(producer);
                ultrainio_assert(it_disable != dp_tbl.end(), "error: producer is not in disabled table");
                if(it_disable->total_cons_staked >= _gstate.min_activated_stake) {
                    uint64_t assigned_location = it_disable->location;
                    if(assigned_location == default_chain_name) {
                        assigned_location = getdefaultchain();
                    }
                    producer_info new_en_prod;
                    new_en_prod.owner                   = it_disable->owner;
                    new_en_prod.producer_key            = it_disable->producer_key;
                    new_en_prod.bls_key                 = it_disable->bls_key;
                    new_en_prod.total_cons_staked       = it_disable->total_cons_staked;
                    new_en_prod.url                     = it_disable->url;
                    new_en_prod.total_produce_block     = it_disable->total_produce_block;
                    new_en_prod.location                = assigned_location;
                    new_en_prod.last_operate_blocknum   = curblocknum;
                    new_en_prod.delegated_cons_blocknum = it_disable->delegated_cons_blocknum;
                    new_en_prod.claim_rewards_account   = it_disable->claim_rewards_account;
                    new_en_prod.unpaid_balance          = 0;
                    new_en_prod.vote_number             = 0;
                    new_en_prod.last_vote_blocknum      = 0;
                    add_to_chain(assigned_location, new_en_prod);
                    dp_tbl.erase(it_disable);
                    _briefproducers.modify(briefprod, [&](producer_brief& producer_brf) {
                        producer_brf.in_disable = false;
                        producer_brf.location = assigned_location;
                    });
                }
                else {
                    //still disable
                    dp_tbl.modify(it_disable, [&]( disabled_producer& dis_prod ) {
                        dis_prod.producer_key            = producer_key;
                        dis_prod.bls_key                 = bls_key;
                        dis_prod.url                     = url;
                        dis_prod.location                = location;
                        dis_prod.last_operate_blocknum   = curblocknum;
                    });
                }
            }
            else {
                if(location != master_chain_name) {
                    ultrainio_assert(_subchains.find(location) != _subchains.end(),
                                     "wrong location, subchain is not existed");
                }
                //if location changes
                producers_table _producers(_self, briefprod->location);
                auto prod = _producers.find(producer);
                ultrainio_assert(prod != _producers.end(), "producer not found");
                if(prod->location != location) {
                    ultrainio_assert(!prod->is_on_master_chain(), "cannot move producers from master chain");
                    add_to_chain(location, *prod);
                    remove_from_chain(prod->location, prod->owner);
                } else {
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

        if (has_auth(_self)) {
            require_auth(_self);
        } else {
            ultrainio_assert( _gstate.is_master_chain(), "only master chain allow regproducer" );
            asset cur_tokens =
                ultrainio::token(N(utrio.token)).get_balance( producer,symbol_type(CORE_SYMBOL).name());
            ultrainio_assert( cur_tokens.amount >= 50000,
                              "The current action fee is 5 UGAS, please ensure that the account is fully funded" );
            INLINE_ACTION_SENDER(ultrainio::token, safe_transfer)( N(utrio.token), {producer,N(active)},
                { producer, N(utrio.fee), asset(50000), std::string("regproducer fee") } );
        }
    }

   void system_contract::unregprod( const account_name producer ) {
      require_auth( producer );
      ultrainio_assert( get_enabled_producers_number() > _gstate.min_committee_member_number,
                        "The number of committee member is too small, unregprod suspended for now");
      if (has_auth(_self)) {
         require_auth(_self);
      } else{
         ultrainio_assert( _gstate.is_master_chain(), "only master chain allow unregprod" );
      }
      auto briefprod = _briefproducers.find(producer);
      ultrainio_assert(briefprod != _briefproducers.end(), "this account is not a producer");
      ultrainio_assert(!briefprod->in_disable, "this producer is not enabled");

      producers_table _producers(_self, briefprod->location);
      const auto& prod = _producers.find( producer );
      ultrainio_assert(prod != _producers.end(), "producer is not found in its location");
      uint64_t curblocknum = (uint64_t)head_block_number() + 1;
      ultrainio_assert( (curblocknum - prod->last_operate_blocknum) > 2 ,
                        "wait at least 2 blocks before this unregprod operation" );

      //pay unpaid_balance
      if(prod->unpaid_balance > 0 && _gstate.is_master_chain()) {
         claim_reward_to_account(prod->claim_rewards_account, asset((int64_t)prod->unpaid_balance));
      }
      disabled_producers_table dp_tbl(_self, _self);
      dp_tbl.emplace( [&]( disabled_producer& dis_prod ) {
          dis_prod = *prod;
          dis_prod.last_operate_blocknum = curblocknum;
      });
      _briefproducers.modify(briefprod, [&](producer_brief& producer_brf) {
          producer_brf.in_disable = true;
      });
      remove_from_chain(briefprod->location, producer);
   }

    inline uint32_t system_contract::get_enabled_producers_number(){
      uint32_t  enabled_prod_num = 0;
      producers_table _producers(_self, master_chain_name);
      for(auto itr = _producers.begin(); itr != _producers.end(); ++itr){
         ++enabled_prod_num;
      }
      return enabled_prod_num;
    }

    std::vector<uint64_t> system_contract::get_all_chainname() {
        std::vector<uint64_t> scopes;
        scopes.emplace_back(master_chain_name);
        for(auto ite_chain = _subchains.begin(); ite_chain != _subchains.end(); ++ite_chain) {
            scopes.emplace_back(ite_chain->chain_name);
        }
        return scopes;
    }
} /// namespace ultrainiosystem
