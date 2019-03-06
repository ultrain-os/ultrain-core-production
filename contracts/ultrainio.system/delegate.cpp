/**
 *  @file
 *  @copyright defined in ultrain/LICENSE.txt
 */
#include "ultrainio.system.hpp"

#include <ultrainiolib/ultrainio.hpp>
#include <ultrainiolib/datastream.hpp>
#include <ultrainiolib/serialize.hpp>
#include <ultrainiolib/privileged.h>
#include <ultrainiolib/transaction.hpp>

#include <ultrainio.token/ultrainio.token.hpp>

#include <cmath>
#include <map>

namespace ultrainiosystem {
   using namespace ultrainio;
   using namespace std;

   static constexpr time refund_delay = 3*60;//3*24*3600;

   /**
    *  Every user 'from' has a scope/table that uses every receipient 'to' as the primary key.
    */
   struct delegated_consensus {
      account_name  from;
      account_name  to;
      asset         cons_weight;
      uint64_t  primary_key() const { return to; }

      ULTRAINLIB_SERIALIZE( delegated_consensus, (from)(to)(cons_weight) )
   };

    struct refund_cons {
      account_name  owner;
      time          request_time;
      asset         cons_amount;

      uint64_t  primary_key() const { return owner; }

      ULTRAINLIB_SERIALIZE( refund_cons, (owner)(request_time)(cons_amount) )
   };

   /**
    *  user staked consensus
    */
   typedef ultrainio::multi_index< N(delcons), delegated_consensus> del_consensus_table;
   typedef ultrainio::multi_index< N(refundscons), refund_cons>     refunds_cons_table;

   void system_contract::change_cons(account_name from, account_name receiver, asset stake_cons_delta) {
       require_auth(from);

       ultrainio_assert(_gstate.is_master_chain() ||  // on master chain
                        from == _self || // ultrainio account
                        name{from}.to_string().find( "utrio." ) == 0, // system account
                        "only master chain or privilaged account allow (un)delegate consensus" );

       ultrainio_assert(stake_cons_delta.amount >= 1000000 ||
                        stake_cons_delta.amount < 0 , "should stake at least 100 amount" );

      // update consensus stake delegated from "from" to "receiver"
      del_consensus_table     del_tbl(_self, from);
      auto itr = del_tbl.find(receiver);

      if (stake_cons_delta.amount < 0) {
          ultrainio_assert(itr != del_tbl.end(), "no delegate record found for undelgating action" );
      }

      if (itr == del_tbl.end()) {
         itr = del_tbl.emplace( [&]( auto& dbo ){
               dbo.from          = from;
               dbo.to            = receiver;
               dbo.cons_weight   = stake_cons_delta;
            });
      } else {
         del_tbl.modify( itr, [&]( auto& dbo  ) {
               if (stake_cons_delta.amount < 0) {
                  stake_cons_delta = -dbo.cons_weight;
                  dbo.cons_weight = asset(0);
               } else {
                  dbo.cons_weight += stake_cons_delta;
               }
            });
      }

      ultrainio_assert( asset(0) <= itr->cons_weight, "insufficient staked consensous" );
      if ( itr->cons_weight == asset(0)) {
         del_tbl.erase( itr );
      }

      account_name source_stake_from = from;
      if ( N(utrio.stake) != source_stake_from ) { //for ultrainio both transfer and refund make no sense
          if (stake_cons_delta.amount < 0) {
              process_undelegate_request(from, stake_cons_delta);
          } else {
            INLINE_ACTION_SENDER(ultrainio::token, safe_transfer)( N(utrio.token), {source_stake_from, N(active)},
               { source_stake_from, N(utrio.stake), asset(stake_cons_delta), std::string("stake consensus") } );
         }
      }

      // if on master chain, update voting power; else add to subchain.
      {
         asset total_update = stake_cons_delta;
         auto briefprod = _briefproducers.find(receiver);
         ultrainio_assert(briefprod != _briefproducers.end(), "this account is not a producer, please regproducer first");
         if(briefprod->in_disable) {
            disabled_producers_table dp_tbl(_self, _self);
            auto dis_prod = dp_tbl.find(receiver);
            ultrainio_assert(dis_prod != dp_tbl.end(), "receiver is not found in its location");
            uint64_t curblocknum = (uint64_t)head_block_number() + 1;
             if(stake_cons_delta.amount < 0){
                 print("undelegatecons from:",name{from}," receiver:",name{receiver}," curblocknum:",curblocknum," delegated_cons_blocknum:",dis_prod->delegated_cons_blocknum);//
                 if((name{from}.to_string().find( "utrio." ) != 0 ) && (from != _self)){
                     const uint32_t seconds_per_block     = block_interval_seconds();
                     uint32_t blocks_per_month            = seconds_per_day / seconds_per_block / 2;//seconds_per_year / seconds_per_block / 12;
                     ultrainio_assert( (curblocknum - dis_prod->delegated_cons_blocknum) > blocks_per_month , "should stake at least more than one month" );
                 }
             }
             if(dis_prod->is_on_master_chain()) {
                 _gstate.total_activated_stake += total_update.amount;
             }
             auto enabled = ((dis_prod->total_cons_staked + total_update.amount) >= _gstate.min_activated_stake);
             if(enabled) {
                producer_info new_en_prod;
                new_en_prod.owner                   = dis_prod->owner;
                new_en_prod.producer_key            = dis_prod->producer_key;
                new_en_prod.bls_key                 = dis_prod->bls_key;
                new_en_prod.total_cons_staked       = dis_prod->total_cons_staked + total_update.amount;
                new_en_prod.url                     = dis_prod->url;
                new_en_prod.total_produce_block     = dis_prod->total_produce_block;
                new_en_prod.location                = dis_prod->location;
                new_en_prod.last_operate_blocknum   = dis_prod->last_operate_blocknum;
                new_en_prod.delegated_cons_blocknum = curblocknum;
                new_en_prod.claim_rewards_account   = dis_prod->claim_rewards_account;
                new_en_prod.unpaid_balance          = 0;
                new_en_prod.vote_number             = 0;
                new_en_prod.last_vote_blocknum      = 0;
                add_to_chain(dis_prod->location, new_en_prod);
                dp_tbl.erase(dis_prod);
                _briefproducers.modify(briefprod, [&](producer_brief& producer_brf) {
                    producer_brf.in_disable = false;
                });
             }
             else {
                dp_tbl.modify(dis_prod, [&]( disabled_producer& disproducer ) {
                    disproducer.total_cons_staked += total_update.amount;
                    disproducer.delegated_cons_blocknum = curblocknum;
                });
             }
         }
         else {
             producers_table _producers(_self, briefprod->location);
             const auto& it = _producers.find( receiver );
             ultrainio_assert(it != _producers.end(), "receiver is not found in its location");

             uint64_t curblocknum = (uint64_t)head_block_number() + 1;
             if(stake_cons_delta.amount < 0){
                 print("undelegatecons from:",name{from}," receiver:",name{receiver}," curblocknum:",curblocknum," delegated_cons_blocknum:",it->delegated_cons_blocknum);//
                 if((name{from}.to_string().find( "utrio." ) != 0 ) && (from != _self)){
                     const uint32_t seconds_per_block     = block_interval_seconds();
                     uint32_t blocks_per_month            = seconds_per_day / seconds_per_block / 2;//seconds_per_year / seconds_per_block / 12;
                     ultrainio_assert( (curblocknum - it->delegated_cons_blocknum) > blocks_per_month , "should stake at least more than one month" );
                 }
             }
             if(it->is_on_master_chain()) {
                 _gstate.total_activated_stake += total_update.amount;
             }
             auto enabled = ((it->total_cons_staked + total_update.amount) >= _gstate.min_activated_stake);
             if(enabled) {
                 _producers.modify(it, [&](auto & v) {
                     v.total_cons_staked += total_update.amount;
                     v.delegated_cons_blocknum = curblocknum;
                 });
             }
             else {
                 if(it->unpaid_balance > 0){
                     claim_reward_to_account(it->claim_rewards_account, asset((int64_t)it->unpaid_balance));
                 }
                 disabled_producers_table dp_tbl(_self, _self);
                 dp_tbl.emplace( [&]( disabled_producer& dis_prod ) {
                     dis_prod.owner                   = it->owner;
                     dis_prod.producer_key            = it->producer_key;
                     dis_prod.bls_key                 = it->bls_key;
                     dis_prod.total_cons_staked       = it->total_cons_staked + total_update.amount;
                     dis_prod.url                     = it->url;
                     dis_prod.total_produce_block     = it->total_produce_block;
                     dis_prod.location                = 0;
                     dis_prod.last_operate_blocknum   = it->last_operate_blocknum;
                     dis_prod.delegated_cons_blocknum = curblocknum;
                     dis_prod.claim_rewards_account   = it->claim_rewards_account;
                 });
                 remove_from_chain(briefprod->location, receiver);
                 _briefproducers.modify(briefprod, [&](producer_brief& producer_brf) {
                     producer_brf.in_disable = true;
                 });
                 _producers.erase(it);
             }
         }
      }
   }


    void system_contract::process_undelegate_request(account_name from,
                                                     asset unstake_quantity) {
        ultrainio_assert( unstake_quantity < asset(0), "unstaked consensous asset must be negative" );
        refunds_cons_table refunds_tbl( _self, from );
        bool need_deferred_trx = false;
        auto req = refunds_tbl.find( from );
        // create refund_cons or update from existing refund_cons
        if ( req != refunds_tbl.end() ) { //need to update refund_cons
             refunds_tbl.modify( req, [&]( refund_cons& r ) {
                r.request_time = now();
                r.cons_amount -= unstake_quantity;
             });

             ultrainio_assert( asset(0) <= req->cons_amount, "negative refund_cons amount" ); //should never happen

             if ( req->cons_amount == asset(0)) {
                 refunds_tbl.erase( req );
                 need_deferred_trx = false;
             } else {
                 need_deferred_trx = true;
             }
         } else { //need to create refund_cons
             refunds_tbl.emplace( [&]( refund_cons& r ) {
                r.owner = from;
                r.cons_amount = -unstake_quantity;
                r.request_time = now();
             });
             need_deferred_trx = true;
         }

        uint128_t trxid = from + N(refund_cons);
        cancel_deferred(trxid);
        if ( need_deferred_trx ) {
            ultrainio::transaction out;
            out.actions.emplace_back( permission_level{ from, N(active) }, _self, NEX(refundcons), from );
            out.delay_sec = refund_delay;
            out.send( trxid, _self, true );
        }
    }

  void system_contract::syncresource(account_name receiver, uint64_t combosize, uint32_t block_height) {
       resources_lease_table _reslease_tbl( _self, master_chain_name );//In side chain, always treat itself as master
       auto resiter = _reslease_tbl.find(receiver);
       if(resiter != _reslease_tbl.end()) {
           ultrainio_assert(combosize >= resiter->lease_num, "error combo size in sync resource");
           int64_t remain_blockheight = (int64_t)block_height - ((int64_t)resiter->end_block_height - (int64_t)resiter->start_block_height);
           ultrainio_assert(remain_blockheight + 30 > 0, "error end time in sync resource");
           if(remain_blockheight > seconds_per_day/2/block_interval_seconds()){
              auto deltadays = ceil(block_interval_seconds()*remain_blockheight/double(seconds_per_day));
               INLINE_ACTION_SENDER(ultrainiosystem::system_contract, resourcelease)( N(ultrainio), {N(ultrainio), N(active)},
                                { N(ultrainio), receiver, 0, deltadays, master_chain_name} );
           }
           int64_t deltasize = (int64_t)combosize - (int64_t)resiter->lease_num;
           if(deltasize > 0) {
               INLINE_ACTION_SENDER(ultrainiosystem::system_contract, resourcelease)( N(ultrainio), {N(ultrainio), N(active)},
                                 { N(ultrainio), receiver, deltasize, 0, master_chain_name} );
           }
       } else {
           if(block_height > 0 && combosize > 0){
               auto days = ceil(block_interval_seconds()*block_height/(double)seconds_per_day);
               INLINE_ACTION_SENDER(ultrainiosystem::system_contract, resourcelease)( N(ultrainio), {N(ultrainio), N(active)},
                     { N(ultrainio), receiver, combosize, days, master_chain_name} );
            }
       }
   }

   void system_contract::resourcelease( account_name from, account_name receiver,
                          uint64_t combosize, uint64_t days, uint64_t location){
      require_auth( from );
      ultrainio_assert( _gstate.is_master_chain() || from == _self, "only master chain allow sync resourcelease" );
      ultrainio_assert(location != default_chain_name, "wrong location");
      auto chain_itr = _subchains.end();
      if(location != master_chain_name) {
         chain_itr = _subchains.find(location);
         ultrainio_assert(chain_itr != _subchains.end(), "this subchian location is not existed");
         ultrainio_assert(is_empowered(receiver, location), "the receiver is not yet empowered to this chain before");
      }else{
          ultrainio_assert( from == _self, "master chain not allow buy resourcelease" );
      }
      resources_lease_table _reslease_tbl( _self,location );

      auto max_availableused_size = _gstate.max_resources_number - _gstate.total_resources_used_number;
      if(chain_itr != _subchains.end()) {
          max_availableused_size = chain_itr->global_resource.max_resources_number - chain_itr->global_resource.total_resources_used_number;
      }
      std::string availableuserstr = "resources lease package available amount:"+ std::to_string(max_availableused_size);
      ultrainio_assert( combosize <= uint64_t(max_availableused_size), availableuserstr.c_str() );
      uint64_t bytes = 0;
      if(chain_itr == _subchains.end()) {
          bytes = (_gstate.max_ram_size-2ll*1024*1024*1024)/_gstate.max_resources_number;
      }
      else {
          bytes = (chain_itr->global_resource.max_ram_size-2ll*1024*1024*1024)/chain_itr->global_resource.max_resources_number;
      }
      print("resourcelease receiver:", name{receiver}, " combosize:",combosize," days:",days," location:",location);
      ultrainio_assert( days <= (365*30+7), "resource lease buy days must reserve a positive and less than 30 years" );

      // update totals of "receiver"
      {
         uint64_t cuttingfee = 0;
         auto reslease_itr = _reslease_tbl.find( receiver );
         if( reslease_itr ==  _reslease_tbl.end() ) {
            ultrainio_assert( (combosize > 0) && (days > 0), "resource lease buy days and numbler must > 0" );
            cuttingfee = days*combosize;
            if(chain_itr == _subchains.end()) {
                _gstate.total_resources_used_number += combosize;
                _gstate.total_ram_bytes_used += (uint64_t)combosize*bytes;
            }
            else {
                _subchains.modify(chain_itr, [&]( auto& _subchain ) {
                    _subchain.global_resource.total_resources_used_number += combosize;
                    _subchain.global_resource.total_ram_bytes_used += (uint64_t)combosize*bytes;
                });
            }
            reslease_itr = _reslease_tbl.emplace( [&]( auto& tot ) {
                  tot.owner = receiver;
                  tot.lease_num = combosize;
                  tot.start_block_height = (uint32_t)head_block_number();
                  tot.end_block_height = tot.start_block_height + (uint32_t)days*seconds_per_day / block_interval_seconds();
                  tot.modify_block_height = (uint32_t)head_block_number();
               });
         } else {
            ultrainio_assert(((combosize > 0) && (days == 0))||((combosize == 0) && (days > 0)), "resource lease days and numbler can't increase them at the same time" );
            if(combosize > 0)
            {
               ultrainio_assert(reslease_itr->end_block_height > (uint32_t)head_block_number(), "resource lease endtime already expired" );
               double remain_time = block_interval_seconds()*(reslease_itr->end_block_height - (uint32_t)head_block_number())/(double)seconds_per_day;
               cuttingfee = uint64_t(ceil(remain_time))*combosize;
               if(chain_itr == _subchains.end()) {
                   _gstate.total_resources_used_number += combosize;
                   _gstate.total_ram_bytes_used += (uint64_t)combosize*bytes;
               }
               else {
                   _subchains.modify(chain_itr, [&]( auto& _subchain ) {
                       _subchain.global_resource.total_resources_used_number += combosize;
                       _subchain.global_resource.total_ram_bytes_used += (uint64_t)combosize*bytes;
                   });
               }
            } else if(days > 0)
            {
               ultrainio_assert(reslease_itr->lease_num > 0, "resource lease number is not normal" );
               cuttingfee = days*reslease_itr->lease_num;
            }
            _reslease_tbl.modify( reslease_itr, [&]( auto& tot ) {
                  tot.lease_num += combosize;
                  tot.end_block_height  += days * seconds_per_day / block_interval_seconds();
                  tot.modify_block_height = (uint32_t)head_block_number();
               });
         }
         auto resourcefee = (int64_t)ceil((double)10000*640*cuttingfee/0.3/365);
         ultrainio_assert(resourcefee > 0, "resource lease resourcefee is abnormal" );

         INLINE_ACTION_SENDER(ultrainio::token, safe_transfer)( N(utrio.token), {from,N(active)},
                                             { from, N(utrio.fee), asset(resourcefee), std::string("buy resource lease") } );
         print("resourcelease calculatefee receiver:", name{receiver}," resourcenum_time:",cuttingfee, " resourcefee:",resourcefee);
         ultrainio_assert( 0 < reslease_itr->lease_num, "insufficient resource lease" );
         if (chain_itr == _subchains.end()) {
             set_resource_limits( receiver, int64_t(bytes*reslease_itr->lease_num), int64_t(reslease_itr->lease_num), int64_t(reslease_itr->lease_num) );
             print("current resource limit  receiver:", name{receiver}, " net_weight:",reslease_itr->lease_num," cpu:",reslease_itr->lease_num," ram:",int64_t(bytes*reslease_itr->lease_num));
         }
      } // tot_itr can be invalid, should go out of scope
   }

void system_contract::delegatecons(account_name from, account_name receiver, asset stake_cons_quantity)
   {
      ultrainio_assert( stake_cons_quantity > asset(0), "must stake a positive amount" );
      change_cons( from, receiver, stake_cons_quantity);
   } // delegatecons

    void system_contract::undelegatecons( account_name from, account_name receiver) {
        auto briefprod = _briefproducers.find(receiver);
        ultrainio_assert(briefprod != _briefproducers.end(), "Unable to undelegate cons for non-producer" );
        if (!briefprod->in_disable) {
            producers_table _producers(_self, briefprod->location);
            auto prod = _producers.find(receiver);
            ultrainio_assert( (prod != _producers.end()), "Producer is not found in its location" );
            ultrainio_assert(get_enabled_producers_number() > _gstate.min_committee_member_number,
                        "The number of committee member is too small, undelegatecons suspended for now");
        }
        change_cons(from, receiver, asset(-1));
    } // undelegatecons

   void system_contract::refundcons(const account_name owner ) {
      require_auth( owner );

      refunds_cons_table refunds_tbl( _self, owner );
      auto req = refunds_tbl.find( owner );
      ultrainio_assert( req != refunds_tbl.end(), "refunds_cons_table request not found" );
      ultrainio_assert( req->request_time + refund_delay <= now(), "refund is not available yet" );
      // Until now() becomes NOW, the fact that now() is the timestamp of the previous block could in theory
      // allow people to get their tokens earlier than the 3 day delay if the unstake happened immediately after many
      // consecutive missed blocks.

      INLINE_ACTION_SENDER(ultrainio::token, safe_transfer)( N(utrio.token), {N(utrio.stake),N(active)},
                                                    { N(utrio.stake), req->owner, req->cons_amount, std::string("unstake") } );

      refunds_tbl.erase( req );
   }
   void system_contract::recycleresource(const account_name owner ,uint64_t lease_num) {
      require_auth( _self );
      int64_t ram_bytes = 0;
      get_account_ram_usage( owner, &ram_bytes );
      print("checkresexpire  recycleresource account:",name{owner}," ram_used:",ram_bytes);
      set_resource_limits( owner, ram_bytes, 0, 0 );
      if(_gstate.total_resources_used_number >= lease_num)
         _gstate.total_resources_used_number -= lease_num;
      uint64_t bytes = (_gstate.max_ram_size-2ll*1024*1024*1024)/_gstate.max_resources_number;
      if(_gstate.total_ram_bytes_used >= (lease_num*bytes - (uint64_t)ram_bytes))
         _gstate.total_ram_bytes_used =_gstate.total_ram_bytes_used - lease_num*bytes + (uint64_t)ram_bytes;
   }

   void system_contract::checkresexpire(){
      uint32_t block_height = (uint32_t)head_block_number() + 1;
      uint32_t interval_num = seconds_per_day/block_interval_seconds()/3;
      if(block_height < 120 || block_height%interval_num != 0) {
         return;
      }

      uint64_t starttime = current_time();
      resources_lease_table _reslease_tbl( _self, master_chain_name);
      for(auto leaseiter = _reslease_tbl.begin(); leaseiter != _reslease_tbl.end(); ) {
         if(leaseiter->end_block_height <= block_height){
            print("checkresexpire reslease name:",name{leaseiter->owner}, " end_block_height:",leaseiter->end_block_height," cur block_height:",block_height);
            //drop contract account table
            db_drop_table(leaseiter->owner);
            //clear contract account code
            ultrainio::transaction codetrans;
            codetrans.actions.emplace_back( permission_level{ leaseiter->owner, N(active) }, _self, NEX(setcode), std::make_tuple(leaseiter->owner, 0, 0, bytes()) );
            codetrans.actions[0].authorization.emplace_back(permission_level{ N(ultrainio), N(active)});
            codetrans.delay_sec = 1;
            uint128_t trxid = now() + leaseiter->owner + N(setcode);
            cancel_deferred(trxid);
            codetrans.send( trxid, _self, true );
            print("checkresexpire set code:",current_time()," trxid:",trxid);
            //clear contract account abi
            ultrainio::transaction abitrans;
            abitrans.actions.emplace_back( permission_level{ leaseiter->owner, N(active) }, _self, NEX(setabi), std::make_tuple(leaseiter->owner, bytes()) );
            abitrans.actions[0].authorization.emplace_back(permission_level{ N(ultrainio), N(active)});
            abitrans.delay_sec = 1;
            trxid = now() +leaseiter->owner + N(setabi);
            cancel_deferred(trxid);
            abitrans.send( trxid, _self, true );
            print("checkresexpire set abi:",current_time()," trxid:",trxid);
            //recycle resource
            ultrainio::transaction recyclerestrans;
            recyclerestrans.actions.emplace_back( permission_level{ _self, N(active) }, _self, NEX(recycleresource), std::make_tuple(leaseiter->owner, leaseiter->lease_num) );
            recyclerestrans.delay_sec = 3;
            trxid = now() +leaseiter->owner + N(recycleres);
            cancel_deferred(trxid);
            recyclerestrans.send( trxid, _self, true );
            print("checkresexpire recycleres:",current_time()," trxid:",trxid);
            leaseiter = _reslease_tbl.erase(leaseiter);
         } else {
            ++leaseiter;
         }
      }

      auto chain_iter = _subchains.begin();
      for(; chain_iter != _subchains.end(); ++chain_iter) {
            if(chain_iter->global_resource.total_resources_used_number > 0) {
               resources_lease_table _reslease_sub( _self, chain_iter->chain_name);
               for(auto reslease_iter = _reslease_sub.begin(); reslease_iter != _reslease_sub.end(); ) {
                  if(reslease_iter->end_block_height <= block_height) {
                        uint64_t bytes = (chain_iter->global_resource.max_ram_size-2ll*1024*1024*1024)/chain_iter->global_resource.max_resources_number;
                        if(chain_iter->global_resource.total_resources_used_number >= reslease_iter->lease_num) {
                           _subchains.modify(chain_iter, [&]( auto& subchain ) {
                              subchain.global_resource.total_resources_used_number -= reslease_iter->lease_num;
                              subchain.global_resource.total_ram_bytes_used -= reslease_iter->lease_num*bytes;
                           });
                        }
                        reslease_iter = _reslease_sub.erase(reslease_iter);
                  } else {
                        ++reslease_iter;
                  }
               }
            }
      }
      uint64_t endtime = current_time();
      print("checkresexpire expend time:",(endtime - starttime));
   }

} //namespace ultrainiosystem
