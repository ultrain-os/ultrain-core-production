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

   static constexpr time refund_delay = 3*24*3600;

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
       check_producer_evillist( receiver );
       ultrainio_assert(_gstate.is_master_chain() ||  // on master chain
                        from == _self || // ultrainio account
                        name{from}.to_string().find( "utrio." ) == 0, // system account
                        "only master chain or privilaged account allow (un)delegate consensus" );
       int64_t precision = (int64_t)stake_cons_delta.symbol.precision();
       ultrainio_assert(stake_cons_delta.amount >= 100 * precision ||
                        stake_cons_delta.amount < 0 , "should stake at least 100 UGAS" );

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
            INLINE_ACTION_SENDER(ultrainio::token, transfer)( N(utrio.token), {source_stake_from, N(active)},
               { source_stake_from, N(utrio.stake), asset(stake_cons_delta), std::string("stake consensus") } );
         }
      }

      // if on master chain, update voting power; else add to subchain.
      {
         uint64_t curblocknum = (uint64_t)head_block_number() + 1;
         auto const check_isallow_undelegate_func = [=]( uint64_t delegated_cons_blocknum ){
            if(stake_cons_delta.amount >= 0) return;
            print("undelegatecons from:",name{from}," receiver:",name{receiver}," curblocknum:",curblocknum," delegated_cons_blocknum:", delegated_cons_blocknum);//
            if((name{from}.to_string().find( "utrio." ) == 0 ) || (from == _self))
               return;
            uint64_t undelegate_blocks_interval  = getglobalextenuintdata( ultrainio_global_state::global_state_exten_type_key::allow_undelegate_block_interval,
                                                                        seconds_per_year / block_interval_seconds() / 12 );
            std::string assert_msg = std::string("should stake at least more than block interval:") + std::to_string( undelegate_blocks_interval );
            ultrainio_assert( (curblocknum - delegated_cons_blocknum) > undelegate_blocks_interval , assert_msg.c_str() );
         };
         asset total_update = stake_cons_delta;
         auto briefprod = _briefproducers.find(receiver);
         ultrainio_assert(briefprod != _briefproducers.end(), "this account is not a producer, please regproducer first");

         int64_t producer_min_activated_stake = get_producer_min_stake(receiver, briefprod->location);
         if(briefprod->in_disable) {
            disabled_producers_table dp_tbl(_self, _self);
            auto dis_prod = dp_tbl.find(receiver);
            ultrainio_assert(dis_prod != dp_tbl.end(), "receiver is not found in its location");
            check_isallow_undelegate_func( dis_prod->delegated_cons_blocknum );
             if(briefprod->is_on_master_chain()) {
                 _gstate.total_activated_stake += total_update.amount;
             }
             auto enabled = ((dis_prod->total_cons_staked + total_update.amount) >= producer_min_activated_stake);
             if(enabled) {
                dp_tbl.modify(dis_prod, [&]( disabled_producer& _dis ) {
                    _dis.total_cons_staked       += total_update.amount;
                    _dis.delegated_cons_blocknum = curblocknum;
                });

                moveprod_param mv_prod(dis_prod->owner, dis_prod->producer_key, dis_prod->bls_key, true, name{N(disable)}, false, briefprod->location);
                send_defer_moveprod_action( mv_prod );
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
             check_isallow_undelegate_func( it->delegated_cons_blocknum );
             if(briefprod->is_on_master_chain()) {
                 _gstate.total_activated_stake += total_update.amount;
             }
             auto enabled = ((it->total_cons_staked + total_update.amount) >= producer_min_activated_stake);
             if(enabled) {
                 _producers.modify(it, [&](auto & v) {
                     v.total_cons_staked += total_update.amount;
                     v.delegated_cons_blocknum = curblocknum;
                 });
             }
             else {
                 _producers.modify(it, [&]( producer_info& info ) {
                    info.total_cons_staked       += total_update.amount;
                    info.delegated_cons_blocknum = curblocknum;
                 });

                 moveprod_param mv_prod(it->owner, it->producer_key, it->bls_key, false, briefprod->location, true, name{N(disable)});
                 send_defer_moveprod_action( mv_prod );
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
            vector<permission_level> pem = { { from, N(active) },
                                            { N(ultrainio),     N(active) } };
            out.actions.emplace_back( pem, _self, NEX(refundcons), from );
            out.delay_sec = (uint32_t)getglobalextenuintdata( ultrainio_global_state::global_state_exten_type_key::refund_delegate_consensus_seconds,
                                                   refund_delay );
            out.send( trxid, _self, true );
        }
    }
/*
   void system_contract::resourcelease( account_name from, account_name receiver,
                          uint64_t combosize, uint64_t days, name location){
      require_auth( from );
      bool is_allow_buy_res = false;
      for(auto extension : _gstate.table_extension){
         if(extension.key == ultrainio_global_state::global_state_exten_type_key::is_allow_buy_res
            && extension.value == "1"){
               is_allow_buy_res = true;
               break;
            }
      }
      if( !is_allow_buy_res )
         ultrainio_assert( from == _self, "only allow ultrainio account resourcelease" );
      ultrainio_assert(location != default_chain_name && location != N(master) , "wrong location");
      auto chain_itr = _chains.end();
      if(location != self_chain_name) {
         chain_itr = _chains.find(location);
         ultrainio_assert(chain_itr != _chains.end(), "this subchian location is not existed");
         ultrainio_assert(is_empowered(receiver, location), "the receiver is not yet empowered to this chain before");
      }else{
          ultrainio_assert( from == _self, "master chain not allow buy resourcelease" );
      }
      resources_lease_table _reslease_tbl( _self,location );

      auto max_availableused_size = _gstate.max_resources_number - _gstate.total_resources_used_number;
      if(chain_itr != _chains.end()) {
          max_availableused_size = chain_itr->global_resource.max_resources_number - chain_itr->global_resource.total_resources_used_number;
      }
      std::string availableuserstr = "resources lease package available amount:"+ std::to_string(max_availableused_size);
      ultrainio_assert( combosize <= uint64_t(max_availableused_size), availableuserstr.c_str() );
      uint64_t bytes = 0;
      if(chain_itr == _chains.end()) {
          bytes = _gstate.max_ram_size/_gstate.max_resources_number;
      }
      else {
          bytes = chain_itr->global_resource.max_ram_size/chain_itr->global_resource.max_resources_number;
      }
      print("resourcelease receiver:", name{receiver}, " combosize:",combosize," days:",days," location:",name{location});
      ultrainio_assert( days <= (365*30+7), "resource lease buy days must reserve a positive and less than 30 years" );
      uint32_t  free_account_per_res = 50; //The default free_account_per_res is 50
      for(auto extension : _gstate.table_extension){
         if(extension.key == ultrainio_global_state::global_state_exten_type_key::free_account_per_res) {
            if(extension.value.empty())
               continue;
            free_account_per_res = (uint32_t)std::stoi(extension.value);
            break;
         }
      }
      // update totals of "receiver"
      {
         uint64_t cuttingfee = 0;
         uint32_t cur_block_height = (uint32_t)head_block_number() + 1;
         auto reslease_itr = _reslease_tbl.find( receiver );
         if( reslease_itr ==  _reslease_tbl.end() ) {
            penddeltable pendingdel(_self,_self);
            auto deltab_itr = pendingdel.find(receiver);
            ultrainio_assert(deltab_itr == pendingdel.end(), "legacy resource is pending for clean up, please apply new resources later");

            ultrainio_assert( (combosize > 0) && (days > 0), "resource lease buy days and numbler must > 0" );
            cuttingfee = days*combosize;
            if(chain_itr == _chains.end()) {
                _gstate.total_resources_used_number += combosize;
                _gstate.total_ram_bytes_used += (uint64_t)combosize*bytes;
            }
            else {
                _chains.modify(chain_itr, [&]( auto& _subchain ) {
                    _subchain.global_resource.total_resources_used_number += combosize;
                    _subchain.global_resource.total_ram_bytes_used += (uint64_t)combosize*bytes;
                });
            }
            reslease_itr = _reslease_tbl.emplace( [&]( auto& tot ) {
                  tot.owner = receiver;
                  tot.lease_num = combosize;
                  tot.start_block_height = cur_block_height;
                  tot.end_block_height = tot.start_block_height + (uint32_t)days*seconds_per_day / block_interval_seconds();
                  tot.modify_block_height = cur_block_height;
                  if(_gstate.is_master_chain() && days*seconds_per_day >= seconds_per_year)
                     tot.free_account_number = (uint32_t)combosize* free_account_per_res;
               });
         } else {
            //handle at most 100 expired resource_lease every 8 hours, so maybe the resource has expired
            ultrainio_assert(reslease_itr->end_block_height > cur_block_height, "legacy resource is waiting for clean up, please apply new resource later");
            uint64_t free_account_number = 0;
            ultrainio_assert(((combosize > 0) && (days == 0))||((combosize == 0) && (days > 0)), "resource lease days and numbler can't increase them at the same time" );
            uint32_t  blocknumberhour = seconds_per_day/24/block_interval_seconds();
            ultrainio_assert( (reslease_itr->modify_block_height + blocknumberhour) <  cur_block_height, "Renewal resources cannot be operated within one hour" );
            if(combosize > 0)
            {
               ultrainio_assert(reslease_itr->end_block_height > cur_block_height, "resource lease endtime already expired" );
               double remain_time = block_interval_seconds()*(reslease_itr->end_block_height - cur_block_height)/(double)seconds_per_day;
               cuttingfee = uint64_t(ceil(remain_time))*combosize;
               if(chain_itr == _chains.end()) {
                   _gstate.total_resources_used_number += combosize;
                   _gstate.total_ram_bytes_used += (uint64_t)combosize*bytes;
               }
               else {
                   _chains.modify(chain_itr, [&]( auto& _subchain ) {
                       _subchain.global_resource.total_resources_used_number += combosize;
                       _subchain.global_resource.total_ram_bytes_used += (uint64_t)combosize*bytes;
                   });
               }
               if(_gstate.is_master_chain() &&  (reslease_itr->end_block_height > cur_block_height) &&
                 ((reslease_itr->end_block_height - cur_block_height)* block_interval_seconds() > seconds_per_year)){
                  free_account_number = combosize* free_account_per_res;
               }
            } else if(days > 0)
            {
               ultrainio_assert(reslease_itr->lease_num > 0, "resource lease number is not normal" );
               cuttingfee = days*reslease_itr->lease_num;
               if(_gstate.is_master_chain() && days*seconds_per_day > seconds_per_year){
                  free_account_number = reslease_itr->lease_num* free_account_per_res;
               }
            }
            _reslease_tbl.modify( reslease_itr, [&]( auto& tot ) {
                tot.lease_num += combosize;
                tot.end_block_height  += days * seconds_per_day / block_interval_seconds();
                tot.modify_block_height = cur_block_height;
                tot.free_account_number += free_account_number;
            });
         }
         auto blocks_per_day = seconds_per_day / block_interval_seconds();
         auto chain_reward_per_day = blocks_per_day * get_reward_per_block();
         auto resourcefee = int64_t(chain_reward_per_day / _gstate.max_resources_number * cuttingfee);
         ultrainio_assert(resourcefee > 0, "resource lease resourcefee is abnormal" );
         if( _gstate.is_master_chain() || from != _self )
            INLINE_ACTION_SENDER(ultrainio::token, transfer)( N(utrio.token), {from,N(active)},
                                { from, N(utrio.resfee), asset(resourcefee), std::string("buy resource lease") } );
         print("resourcelease calculatefee receiver:", name{receiver}," resourcenum_time:",cuttingfee, " resourcefee:",resourcefee);
         ultrainio_assert( 0 < reslease_itr->lease_num, "insufficient resource lease" );
         if (chain_itr == _chains.end()) {
             set_resource_limits( receiver, int64_t(bytes*reslease_itr->lease_num), int64_t(reslease_itr->lease_num), int64_t(reslease_itr->lease_num) );
             print("current resource limit  receiver:", name{receiver}, " net_weight:",reslease_itr->lease_num," cpu:",reslease_itr->lease_num," ram:",int64_t(bytes*reslease_itr->lease_num));
         }
      } // tot_itr can be invalid, should go out of scope
   }

   void system_contract::transferresource(account_name from, account_name to, uint64_t combosize, name location) {
       uint64_t bytes_per_combo = 0;
       if(_gstate.is_master_chain()) {
           require_auth( from );
           ultrainio_assert( location != self_chain_name, "master chain is not allowed to transfer resourcelease" );

           auto chain_ite = _chains.find(location);
           ultrainio_assert(chain_ite != _chains.end(), "this subchian location is not existed");
           ultrainio_assert(is_empowered(to, location), "the receiver was not yet empowered to this chain before");
           bytes_per_combo = chain_ite->global_resource.max_ram_size/chain_ite->global_resource.max_resources_number;
       } else {
           require_auth( _self );
           ultrainio_assert( location == self_chain_name, "wrong location to tranfer resources at sidechain" );
           bytes_per_combo = _gstate.max_ram_size/_gstate.max_resources_number;
       }

       resources_lease_table _reslease_tbl( _self,location );
       auto lease_from = _reslease_tbl.find(from);
       std::string assert_desc(name{from}.to_string());
       assert_desc.append(" has no resource in chain ").append(name{location}.to_string());
       ultrainio_assert(lease_from != _reslease_tbl.end(), assert_desc.c_str());
       assert_desc = name{from}.to_string();
       assert_desc.append(" has not enought resources to transfer");
       ultrainio_assert(lease_from->lease_num >= combosize, assert_desc.c_str());
       uint32_t cur_block_height = (uint32_t)head_block_number() + 1;
       ultrainio_assert(lease_from->end_block_height > cur_block_height, "resource is expired and waiting for clean up");

       uint64_t combosize_to = 0;
       auto lease_to = _reslease_tbl.find(to);
       if(lease_to != _reslease_tbl.end()) {
           assert_desc = "receiver ";
           assert_desc.append(name{to}.to_string()).append(" already has resources with different endtime");
           ultrainio_assert(lease_to->end_block_height == lease_from->end_block_height, assert_desc.c_str());
           combosize_to = lease_to->lease_num;
       }

       uint32_t free_account_size = 0;
       if(_gstate.is_master_chain()) {
           //check free_account_number
           uint32_t  free_account_per_res = 50; //The default free_account_per_res is 50
           for(auto extension : _gstate.table_extension){
               if(extension.key == ultrainio_global_state::global_state_exten_type_key::free_account_per_res) {
                   if(extension.value.empty())
                       continue;
                   free_account_per_res = (uint32_t)std::stoi(extension.value);
                   break;
               }
           }
           free_account_size = free_account_per_res * uint32_t(combosize);
           ultrainio_assert(lease_from->free_account_number >= free_account_size, "no enough free account number");
       } else {
           //calculate how much resource left
           int64_t  total_ram = 0;
           int64_t  total_net = 0;
           int64_t  total_cpu = 0;
           get_resource_limits(from, &total_ram, &total_net, &total_cpu);
           int64_t  used_ram = 0;
           get_account_ram_usage(from, &used_ram);
           auto need_bytes = bytes_per_combo * combosize;
           assert_desc = name{from}.to_string();
           assert_desc.append(" has not enought memory left to transer to others");
           ultrainio_assert(need_bytes <= uint64_t(total_ram - used_ram), assert_desc.c_str());
           int64_t left_size = int64_t(lease_from->lease_num - combosize);
           int64_t new_size = int64_t(combosize_to + combosize);
           set_resource_limits( from, int64_t(bytes_per_combo) * left_size, left_size, left_size );
           set_resource_limits( to, int64_t(bytes_per_combo) * new_size, new_size, new_size );
       }

       if( _gstate.is_master_chain()) {
           uint16_t days = (lease_from->end_block_height - cur_block_height) * block_interval_seconds() / seconds_per_day;
           ultrainio_assert(days > 0, "resource will expire in one day");
           int64_t fee = int64_t(getglobalextenuintdata(ultrainio_global_state::res_transfer_fee, asset().symbol.precision()));
           INLINE_ACTION_SENDER(ultrainio::token, transfer)( N(utrio.token), {from,N(active)},
                    { from, N(utrio.resfee), asset(fee), std::string("transfer resource") } );
       }

       _reslease_tbl.modify( lease_from, [&]( auto& res ) {
           res.lease_num -= combosize;
           res.modify_block_height = cur_block_height;
           res.free_account_number -= free_account_size;
       });

       if(lease_to == _reslease_tbl.end()) {
           _reslease_tbl.emplace([&]( auto& res ) {
               res.owner               = to;
               res.lease_num           = combosize;
               res.start_block_height  = cur_block_height;
               res.end_block_height    = lease_from->end_block_height;
               res.modify_block_height = cur_block_height;
               res.free_account_number = free_account_size;
           });
       }
       else {
           _reslease_tbl.modify( lease_to, [&]( auto& res ) {
               res.lease_num += combosize;
               res.modify_block_height = cur_block_height;
               res.free_account_number += free_account_size;
           });
       }
   }
*/
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
            ultrainio_assert(_gstate.cur_committee_number > _gstate.min_committee_member_number,
                        "The number of committee member is too small, undelegatecons suspended for now");
        }
        change_cons(from, receiver, asset(-1));
    } // undelegatecons

   void system_contract::refundcons(const account_name owner ) {
      require_auth( owner );

      refunds_cons_table refunds_tbl( _self, owner );
      auto req = refunds_tbl.find( owner );
      ultrainio_assert( req != refunds_tbl.end(), "refunds_cons_table request not found" );
      auto refund_delay_sec = (uint32_t)getglobalextenuintdata( ultrainio_global_state::global_state_exten_type_key::refund_delegate_consensus_seconds,
                                             refund_delay );
      ultrainio_assert( req->request_time + refund_delay_sec <= now(), "refund is not available yet" );
      // Until now() becomes NOW, the fact that now() is the timestamp of the previous block could in theory
      // allow people to get their tokens earlier than the refund_delay_sec delay if the unstake happened immediately after many
      // consecutive missed blocks.

      INLINE_ACTION_SENDER(ultrainio::token, transfer)( N(utrio.token), {N(utrio.stake),N(active)},
                                                    { N(utrio.stake), req->owner, req->cons_amount, std::string("unstake") } );

      refunds_tbl.erase( req );
   }
   /*
   void system_contract::recycleresource(const account_name owner) {
      require_auth( _self );
      int64_t ram_bytes = 0;
      get_account_ram_usage( owner, &ram_bytes );
      print("checkresexpire  recycleresource account:",name{owner}," ram_used:",ram_bytes);
      ram_bytes = 0;
      set_resource_limits( owner, ram_bytes, 0, 0 );
   }

   void system_contract::check_res_expire(uint64_t current_block_height){
      uint32_t interval_num = 1;
      if(1 == getglobalextenuintdata(ultrainio_global_state::pending_resource_check, 0)) {
          interval_num = seconds_per_day/block_interval_seconds()/48; //check every half hour
      } else {
          interval_num = seconds_per_day/block_interval_seconds()/3; //check every eight hours
      }

      if(current_block_height < 120 || current_block_height%interval_num != 0) {
         return;
      }
      uint64_t starttime = current_time();
      int64_t ram_bytes = 0;
      int64_t net_bytes = 0;
      int64_t cpu_bytes = 0;
      uint32_t  calc_num = 0;
      std::string pending_res_status("0");
      resources_lease_table _reslease_tbl( _self, self_chain_name);
      for(auto leaseiter = _reslease_tbl.begin(); leaseiter != _reslease_tbl.end(); ) {
         if(leaseiter->end_block_height <= current_block_height){
            calc_num++;
            if(calc_num > 100) {
                pending_res_status = "1";
                break;
            }
            const auto& owner = leaseiter->owner;
            get_resource_limits( owner, &ram_bytes, &net_bytes, &cpu_bytes );
            if(ram_bytes == 0 && net_bytes == 0 && cpu_bytes == 0){
               if(_gstate.total_resources_used_number >= leaseiter->lease_num)
                  _gstate.total_resources_used_number -= leaseiter->lease_num;
               else
                  _gstate.total_resources_used_number = 0;
               uint64_t bytes = _gstate.max_ram_size/_gstate.max_resources_number;
               if(_gstate.total_ram_bytes_used >= leaseiter->lease_num*bytes )
                  _gstate.total_ram_bytes_used -= leaseiter->lease_num*bytes;
               else
                  _gstate.total_ram_bytes_used = 0;
               leaseiter = _reslease_tbl.erase(leaseiter);
            }else{
               set_resource_limits( owner, ram_bytes, 0, 0 ); //Resource expired, no action allowed
               penddeltable pendingdel(_self,_self);
               auto deltab_itr = pendingdel.find(owner);
               if (deltab_itr == pendingdel.end()) {
                  pendingdel.emplace( [&]( auto& d ){ d.owner = owner; });
               }
               ++leaseiter;
            }
         } else {
            ++leaseiter;
         }
      }
      setglobalextendata(ultrainio_global_state::pending_resource_check, pending_res_status);

      for(auto chain_iter = _chains.begin(); chain_iter != _chains.end(); ++chain_iter) {
          if (chain_iter->chain_name == N(master))
              continue;
          const auto& chain_gs = chain_iter->global_resource;
          if (chain_gs.total_resources_used_number <= 0)
              continue;
          uint64_t bytes_per_combo = chain_gs.max_ram_size / chain_gs.max_resources_number;
          resources_lease_table _reslease_sub(_self, chain_iter->chain_name);
          for(auto reslease_iter = _reslease_sub.begin(); reslease_iter != _reslease_sub.end(); ) {
              if(reslease_iter->end_block_height <= current_block_height) {
                  auto old_lease_num = reslease_iter->lease_num;
                  uint64_t lease_bytes = old_lease_num * bytes_per_combo;
                  uint64_t new_lease_number = (chain_gs.total_resources_used_number >= old_lease_num) ?
                      (chain_gs.total_resources_used_number - old_lease_num) : 0;
                  uint64_t new_used_ram = (chain_gs.total_ram_bytes_used >= lease_bytes) ?
                      (chain_gs.total_ram_bytes_used - lease_bytes) : 0;

                  _chains.modify(chain_iter, [&]( auto& subchain ) {
                     subchain.global_resource.total_resources_used_number = new_lease_number;
                     subchain.global_resource.total_ram_bytes_used        = new_used_ram;
                  });

                  reslease_iter = _reslease_sub.erase(reslease_iter);
              } else {
                  ++reslease_iter;
              }
          }
      }
      uint64_t endtime = current_time();
      print("checkresexpire expend time:",(endtime - starttime));
   }
   */
   void system_contract::setfreeacc( account_name account, uint64_t number){
      require_auth( _self );
      freeaccount free_acc(_self,_self);
      auto itr = free_acc.find(account);
      if (itr == free_acc.end()) {
         itr = free_acc.emplace( [&]( auto& f ){
            f.owner = account;
            f.acc_num = number;
         });
      } else {
         free_acc.modify( itr, [&]( auto& f  ) {
            f.acc_num = number;
         });
      }
   }

   void system_contract::del_expire_table(){
      penddeltable pendingdeltab(_self,_self);
      if(pendingdeltab.begin() != pendingdeltab.end()) {
         auto del_iter = pendingdeltab.begin();
         auto owner = del_iter->owner;
         int dropstatus = db_drop_table(owner);   //drop contract account table
         if(dropstatus == 0){
            del_iter = pendingdeltab.erase(del_iter);
            clear_expire_contract( owner );
         }
      }
   }

   void system_contract::clear_expire_contract( account_name owner ){
      vector<permission_level> pem = { { owner, N(active) },
                                       { N(ultrainio),     N(active) } };
      {
         // clear contract
         ultrainio::transaction trx;
         trx.actions.emplace_back(pem, _self, NEX(setcode), std::make_tuple(owner, 0, 0, bytes()) );   //clear contract account code
         trx.actions.emplace_back(pem, _self, NEX(setabi), std::make_tuple(owner, bytes()) );   //clear contract account abi
         trx.delay_sec = 0;
         uint128_t trxid = now() + owner + N(clrcontract);
         cancel_deferred(trxid);
         trx.send( trxid, _self, true );
         print("checkresexpire  clear contract account name: ",name{owner}, " trxid:",trxid);
      }
      {
         //recycle resource
         ultrainio::transaction recyclerestrans;
         recyclerestrans.actions.emplace_back( permission_level{ _self, N(active) }, _self,
                                                NEX(recycleresource), std::make_tuple(owner) );
         recyclerestrans.delay_sec = 10;
         uint128_t trxid = now() + owner + N(recycleres);
         cancel_deferred(trxid);
         recyclerestrans.send( trxid, _self, true );
         print("checkresexpire  recycle resource account name: ",name{owner}, " trxid:",trxid);
      }
   }
} //namespace ultrainiosystem
