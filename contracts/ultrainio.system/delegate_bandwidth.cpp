/**
 *  @file
 *  @copyright defined in ultrain/LICENSE.txt
 */
#include "ultrainio.system.hpp"

#include <ultrainiolib/ultrainio.hpp>
#include <ultrainiolib/print.hpp>
#include <ultrainiolib/datastream.hpp>
#include <ultrainiolib/serialize.hpp>
#include <ultrainiolib/multi_index.hpp>
#include <ultrainiolib/privileged.h>
#include <ultrainiolib/transaction.hpp>

#include <ultrainio.token/ultrainio.token.hpp>


#include <cmath>
#include <map>

namespace ultrainiosystem {
   using ultrainio::asset;
   using ultrainio::indexed_by;
   using ultrainio::const_mem_fun;
   using ultrainio::bytes;
   using ultrainio::print;
   using ultrainio::permission_level;
   using std::map;
   using std::pair;

   static constexpr time refund_delay = 3*24*3600;

   struct user_resources {
      account_name  owner;
      asset         net_weight;
      asset         cpu_weight;
      int64_t       ram_bytes = 0;

      uint64_t primary_key()const { return owner; }

      // explicit serialization macro is not necessary, used here only to improve compilation time
      ULTRAINLIB_SERIALIZE( user_resources, (owner)(net_weight)(cpu_weight)(ram_bytes) )
   };


   /**
    *  Every user 'from' has a scope/table that uses every receipient 'to' as the primary key.
    */
   struct delegated_bandwidth {
      account_name  from;
      account_name  to;
      asset         net_weight;
      asset         cpu_weight;
      asset         cons_weight;
      uint64_t  primary_key()const { return to; }

      // explicit serialization macro is not necessary, used here only to improve compilation time
      ULTRAINLIB_SERIALIZE( delegated_bandwidth, (from)(to)(net_weight)(cpu_weight)(cons_weight) )

   };
   struct refund_cons {
      account_name  owner;
      time          request_time;
      ultrainio::asset  cons_amount;

      uint64_t  primary_key()const { return owner; }

      // explicit serialization macro is not necessary, used here only to improve compilation time
      ULTRAINLIB_SERIALIZE( refund_cons, (owner)(request_time)(cons_amount) )
   };
   struct refund_request {
      account_name  owner;
      time          request_time;
      ultrainio::asset  net_amount;
      ultrainio::asset  cpu_amount;

      uint64_t  primary_key()const { return owner; }

      // explicit serialization macro is not necessary, used here only to improve compilation time
      ULTRAINLIB_SERIALIZE( refund_request, (owner)(request_time)(net_amount)(cpu_amount) )
   };



   /**
    *  These tables are designed to be constructed in the scope of the relevant user, this
    *  facilitates simpler API for per-user queries
    */
   typedef ultrainio::multi_index< N(userres), user_resources>      user_resources_table;
   typedef ultrainio::multi_index< N(delband), delegated_bandwidth> del_bandwidth_table;
   typedef ultrainio::multi_index< N(refunds), refund_request>      refunds_table;
   typedef ultrainio::multi_index< N(refundscons), refund_cons>      refunds_cons_table;


   /**
    *  This action will buy an exact amount of ram and bill the payer the current market price.
    */
   void system_contract::buyrambytes( account_name payer, account_name receiver, uint32_t bytes ) {
      auto itr = _rammarket.find(S(4,RAMCORE));
      auto tmp = *itr;
      auto ultrainout = tmp.convert( asset(bytes,S(0,RAM)), CORE_SYMBOL );

      buyram( payer, receiver, ultrainout );
   }


   /**
    *  When buying ram the payer irreversiblly transfers quant to system contract and only
    *  the receiver may reclaim the tokens via the sellram action. The receiver pays for the
    *  storage of all database records associated with this action.
    *
    *  RAM is a scarce resource whose supply is defined by global properties max_ram_size. RAM is
    *  priced using the bancor algorithm such that price-per-byte with a constant reserve ratio of 100:1.
    */
   void system_contract::buyram( account_name payer, account_name receiver, asset quant )
   {
      require_auth( payer );
      ultrainio_assert( quant.amount > 0, "must purchase a positive amount" );

      auto fee = quant;
      fee.amount = ( fee.amount + 199 ) / 200; /// .5% fee (round up)
      // fee.amount cannot be 0 since that is only possible if quant.amount is 0 which is not allowed by the assert above.
      // If quant.amount == 1, then fee.amount == 1,
      // otherwise if quant.amount > 1, then 0 < fee.amount < quant.amount.
      auto quant_after_fee = quant;
      quant_after_fee.amount -= fee.amount;
      // quant_after_fee.amount should be > 0 if quant.amount > 1.
      // If quant.amount == 1, then quant_after_fee.amount == 0 and the next inline transfer will fail causing the buyram action to fail.

      INLINE_ACTION_SENDER(ultrainio::token, transfer)( N(utrio.token), {payer,N(active)},
         { payer, N(utrio.ram), quant_after_fee, std::string("buy ram") } );

      if( fee.amount > 0 ) {
         INLINE_ACTION_SENDER(ultrainio::token, transfer)( N(utrio.token), {payer,N(active)},
                                                       { payer, N(utrio.ramfee), fee, std::string("ram fee") } );
      }

      int64_t bytes_out;

      const auto& market = _rammarket.get(S(4,RAMCORE), "ram market does not exist");
      _rammarket.modify( market, 0, [&]( auto& es ) {
          bytes_out = es.convert( quant_after_fee,  S(0,RAM) ).amount;
      });

      ultrainio_assert( bytes_out > 0, "must reserve a positive amount" );

      _gstate.total_ram_bytes_reserved += uint64_t(bytes_out);
      _gstate.total_ram_stake          += quant_after_fee.amount;

      user_resources_table  userres( _self, receiver );
      auto res_itr = userres.find( receiver );
      if( res_itr ==  userres.end() ) {
         res_itr = userres.emplace( receiver, [&]( auto& res ) {
               res.owner = receiver;
               res.ram_bytes = bytes_out;
            });
      } else {
         userres.modify( res_itr, receiver, [&]( auto& res ) {
               res.ram_bytes += bytes_out;
            });
      }
      set_resource_limits( res_itr->owner, res_itr->ram_bytes, res_itr->net_weight.amount, res_itr->cpu_weight.amount );
   }


   /**
    *  The system contract now buys and sells RAM allocations at prevailing market prices.
    *  This may result in traders buying RAM today in anticipation of potential shortages
    *  tomorrow. Overall this will result in the market balancing the supply and demand
    *  for RAM over time.
    */
   void system_contract::sellram( account_name account, int64_t bytes ) {
      require_auth( account );
      ultrainio_assert( bytes > 0, "cannot sell negative byte" );

      user_resources_table  userres( _self, account );
      auto res_itr = userres.find( account );
      ultrainio_assert( res_itr != userres.end(), "no resource row" );
      ultrainio_assert( res_itr->ram_bytes >= bytes, "insufficient quota" );

      asset tokens_out;
      auto itr = _rammarket.find(S(4,RAMCORE));
      _rammarket.modify( itr, 0, [&]( auto& es ) {
          /// the cast to int64_t of bytes is safe because we certify bytes is <= quota which is limited by prior purchases
          tokens_out = es.convert( asset(bytes,S(0,RAM)), CORE_SYMBOL);
      });

      ultrainio_assert( tokens_out.amount > 1, "token amount received from selling ram is too low" );

      _gstate.total_ram_bytes_reserved -= static_cast<decltype(_gstate.total_ram_bytes_reserved)>(bytes); // bytes > 0 is asserted above
      _gstate.total_ram_stake          -= tokens_out.amount;

      //// this shouldn't happen, but just in case it does we should prevent it
      ultrainio_assert( _gstate.total_ram_stake >= 0, "error, attempt to unstake more tokens than previously staked" );

      userres.modify( res_itr, account, [&]( auto& res ) {
          res.ram_bytes -= bytes;
      });
      set_resource_limits( res_itr->owner, res_itr->ram_bytes, res_itr->net_weight.amount, res_itr->cpu_weight.amount );

      INLINE_ACTION_SENDER(ultrainio::token, transfer)( N(utrio.token), {N(utrio.ram),N(active)},
                                                       { N(utrio.ram), account, asset(tokens_out), std::string("sell ram") } );

      auto fee = ( tokens_out.amount + 199 ) / 200; /// .5% fee (round up)
      // since tokens_out.amount was asserted to be at least 2 earlier, fee.amount < tokens_out.amount

      if( fee > 0 ) {
         INLINE_ACTION_SENDER(ultrainio::token, transfer)( N(utrio.token), {account,N(active)},
            { account, N(utrio.ramfee), asset(fee), std::string("sell ram fee") } );
      }
   }

   void validate_b1_vesting( int64_t stake ) {
      const int64_t base_time = 1527811200; /// 2018-06-01
      const int64_t max_claimable = 100'000'000'0000ll;
      const int64_t claimable = int64_t(max_claimable * double(now()-base_time) / (10*seconds_per_year) );

      ultrainio_assert( max_claimable - claimable <= stake, "b1 can only claim their tokens over 10 years" );
   }

   void system_contract::changebw( account_name from, account_name receiver,
                                   const asset stake_net_delta, const asset stake_cpu_delta, bool transfer )
   {
      require_auth( from );
      ultrainio_assert( stake_net_delta != asset(0) || stake_cpu_delta != asset(0), "should stake non-zero amount" );
      ultrainio_assert( std::abs( (stake_net_delta + stake_cpu_delta).amount )
                     >= std::max( std::abs( stake_net_delta.amount ), std::abs( stake_cpu_delta.amount ) ),
                    "net and cpu deltas cannot be opposite signs" );

      account_name source_stake_from = from;
      if ( transfer ) {
         from = receiver;
      }

      // update stake delegated from "from" to "receiver"
      {
         del_bandwidth_table     del_tbl( _self, from);
         auto itr = del_tbl.find( receiver );
         if( itr == del_tbl.end() ) {
            itr = del_tbl.emplace( from, [&]( auto& dbo ){
                  dbo.from          = from;
                  dbo.to            = receiver;
                  dbo.net_weight    = stake_net_delta;
                  dbo.cpu_weight    = stake_cpu_delta;
               });
         }
         else {
            del_tbl.modify( itr, 0, [&]( auto& dbo ){
                  dbo.net_weight    += stake_net_delta;
                  dbo.cpu_weight    += stake_cpu_delta;
               });
         }
         ultrainio_assert( asset(0) <= itr->net_weight, "insufficient staked net bandwidth" );
         ultrainio_assert( asset(0) <= itr->cpu_weight, "insufficient staked cpu bandwidth" );
         if ( itr->net_weight == asset(0) && itr->cpu_weight == asset(0) && itr->cons_weight == asset(0)) {
            del_tbl.erase( itr );
         }
      } // itr can be invalid, should go out of scope

      // update totals of "receiver"
      {
         user_resources_table   totals_tbl( _self, receiver );
         auto tot_itr = totals_tbl.find( receiver );
         if( tot_itr ==  totals_tbl.end() ) {
            tot_itr = totals_tbl.emplace( from, [&]( auto& tot ) {
                  tot.owner = receiver;
                  tot.net_weight    = stake_net_delta;
                  tot.cpu_weight    = stake_cpu_delta;
               });
         } else {
            totals_tbl.modify( tot_itr, from == receiver ? from : 0, [&]( auto& tot ) {
                  tot.net_weight    += stake_net_delta;
                  tot.cpu_weight    += stake_cpu_delta;
               });
         }
         ultrainio_assert( asset(0) <= tot_itr->net_weight, "insufficient staked total net bandwidth" );
         ultrainio_assert( asset(0) <= tot_itr->cpu_weight, "insufficient staked total cpu bandwidth" );

         set_resource_limits( receiver, tot_itr->ram_bytes, tot_itr->net_weight.amount, tot_itr->cpu_weight.amount );

         if ( tot_itr->net_weight == asset(0) && tot_itr->cpu_weight == asset(0)  && tot_itr->ram_bytes == 0 ) {
            totals_tbl.erase( tot_itr );
         }
      } // tot_itr can be invalid, should go out of scope

      // create refund or update from existing refund
      if ( N(utrio.stake) != source_stake_from ) { //for ultrainio both transfer and refund make no sense
         refunds_table refunds_tbl( _self, from );
         auto req = refunds_tbl.find( from );

         //create/update/delete refund
         auto net_balance = stake_net_delta;
         auto cpu_balance = stake_cpu_delta;
         bool need_deferred_trx = false;


         // net and cpu are same sign by assertions in delegatebw and undelegatebw
         // redundant assertion also at start of changebw to protect against misuse of changebw
         bool is_undelegating = (net_balance.amount + cpu_balance.amount ) < 0;
         bool is_delegating_to_self = (!transfer && from == receiver);

         if( is_delegating_to_self || is_undelegating ) {
            if ( req != refunds_tbl.end() ) { //need to update refund
               refunds_tbl.modify( req, 0, [&]( refund_request& r ) {
                  if ( net_balance < asset(0) || cpu_balance < asset(0) ) {
                     r.request_time = now();
                  }
                  r.net_amount -= net_balance;
                  if ( r.net_amount < asset(0) ) {
                     net_balance = -r.net_amount;
                     r.net_amount = asset(0);
                  } else {
                     net_balance = asset(0);
                  }
                  r.cpu_amount -= cpu_balance;
                  if ( r.cpu_amount < asset(0) ){
                     cpu_balance = -r.cpu_amount;
                     r.cpu_amount = asset(0);
                  } else {
                     cpu_balance = asset(0);
                  }
               });

               ultrainio_assert( asset(0) <= req->net_amount, "negative net refund amount" ); //should never happen
               ultrainio_assert( asset(0) <= req->cpu_amount, "negative cpu refund amount" ); //should never happen

               if ( req->net_amount == asset(0) && req->cpu_amount == asset(0) ) {
                  refunds_tbl.erase( req );
                  need_deferred_trx = false;
               } else {
                  need_deferred_trx = true;
               }

            } else if ( net_balance < asset(0) || cpu_balance < asset(0) ) { //need to create refund
               refunds_tbl.emplace( from, [&]( refund_request& r ) {
                  r.owner = from;
                  if ( net_balance < asset(0) ) {
                     r.net_amount = -net_balance;
                     net_balance = asset(0);
                  } // else r.net_amount = 0 by default constructor
                  if ( cpu_balance < asset(0) ) {
                     r.cpu_amount = -cpu_balance;
                     cpu_balance = asset(0);
                  } // else r.cpu_amount = 0 by default constructor
                  r.request_time = now();
               });
               need_deferred_trx = true;
            } // else stake increase requested with no existing row in refunds_tbl -> nothing to do with refunds_tbl
         } /// end if is_delegating_to_self || is_undelegating

         if ( need_deferred_trx ) {
            ultrainio::transaction out;
            out.actions.emplace_back( permission_level{ from, N(active) }, _self, NEX(refund), from );
            out.delay_sec = refund_delay;
            cancel_deferred( from ); // TODO: Remove this line when replacing deferred trxs is fixed
            out.send( from, from, true );
         } else {
            cancel_deferred( from );
         }

         auto transfer_amount = net_balance + cpu_balance;
         if ( asset(0) < transfer_amount ) {
            INLINE_ACTION_SENDER(ultrainio::token, transfer)( N(utrio.token), {source_stake_from, N(active)},
               { source_stake_from, N(utrio.stake), asset(transfer_amount), std::string("stake bandwidth") } );
         }
      }
   }

   void system_contract::change_cons( account_name from, account_name receiver, asset stake_cons_delta)
   {
      require_auth( from );
      ultrainio_assert( stake_cons_delta != asset(0), "should stake non-zero amount" );
      ultrainio_assert( stake_cons_delta.amount >= 1000000 || stake_cons_delta.amount < 0 , "should stake at least 100 amount" );
      // update consensus stake delegated from "from" to "receiver"
      {
         del_bandwidth_table     del_tbl( _self, from);
         auto itr = del_tbl.find( receiver );
         if( itr == del_tbl.end() ) {
            itr = del_tbl.emplace( from, [&]( auto& dbo ){
                  dbo.from          = from;
                  dbo.to            = receiver;
                  dbo.cons_weight    = stake_cons_delta;
               });
         }
         else {
            del_tbl.modify( itr, 0, [&]( auto& dbo ){
                  if(stake_cons_delta.amount < 0)
                  {
                    stake_cons_delta = asset(0) - dbo.cons_weight;
                    dbo.cons_weight = asset(0);
                  }else{
                     dbo.cons_weight    += stake_cons_delta;
                  }
               });
         }
         ultrainio_assert( from == receiver ||(name{from}.to_string().find( "utrio." ) == 0) , "Ordinary account cannot be others delegatecons/undelegatecons" );
         ultrainio_assert( asset(0) <= itr->cons_weight, "insufficient staked consensous bandwidth" );
         if ( itr->net_weight == asset(0) && itr->cpu_weight == asset(0) && itr->cons_weight == asset(0)) {
            del_tbl.erase( itr );
         }
      } // itr can be invalid, should go out of scope
      account_name source_stake_from = from;
      // create refund_cons or update from existing refund_cons
      if ( N(utrio.stake) != source_stake_from ) { //for ultrainio both transfer and refund make no sense
         refunds_cons_table refunds_tbl( _self, from );
         auto req = refunds_tbl.find( from );

         //create/update/delete refund_cons
         auto cons_balance = stake_cons_delta;
         bool need_deferred_trx = false;

         // redundant assertion also at start of change_cons to protect against misuse of change_cons
         bool is_undelegating = cons_balance.amount < 0;

         if( is_undelegating ) {
            if ( req != refunds_tbl.end() ) { //need to update refund_cons
               refunds_tbl.modify( req, 0, [&]( refund_cons& r ) {
                  if ( cons_balance < asset(0)) {
                     r.request_time = now();
                  }
                  r.cons_amount -= cons_balance;
                  if ( r.cons_amount < asset(0) ) {
                     cons_balance = -r.cons_amount;
                     r.cons_amount = asset(0);
                  } else {
                     cons_balance = asset(0);
                  }
               });

               ultrainio_assert( asset(0) <= req->cons_amount, "negative refund_cons amount" ); //should never happen

               if ( req->cons_amount == asset(0)) {
                  refunds_tbl.erase( req );
                  need_deferred_trx = false;
               } else {
                  need_deferred_trx = true;
               }

            } else if ( cons_balance < asset(0) ) { //need to create refund_cons
               refunds_tbl.emplace( from, [&]( refund_cons& r ) {
                  r.owner = from;
                  if ( cons_balance < asset(0) ) {
                     r.cons_amount = -cons_balance;
                     cons_balance = asset(0);
                  }
                  r.request_time = now();
               });
               need_deferred_trx = true;
            } // else stake increase requested with no existing row in refunds_tbl -> nothing to do with refunds_tbl
         }

         if ( need_deferred_trx ) {
            ultrainio::transaction out;
            out.actions.emplace_back( permission_level{ from, N(active) }, _self, NEX(refundcons), from );
            out.delay_sec = refund_delay;
            cancel_deferred( from ); // TODO: Remove this line when replacing deferred trxs is fixed
            out.send( from, _self, true );
         } else {
            cancel_deferred( from );
         }

         auto transfer_amount = cons_balance;
         if ( asset(0) < transfer_amount ) {
            INLINE_ACTION_SENDER(ultrainio::token, transfer)( N(utrio.token), {source_stake_from, N(active)},
               { source_stake_from, N(utrio.stake), asset(transfer_amount), std::string("stake bandwidth") } );
         }
      }

      // if on master chain, update voting power; else add to pending chain or subchain.
      {
         asset total_update = stake_cons_delta;
         auto it = _producers.find(receiver);
         ultrainio_assert( (it != _producers.end()) && it->is_active, "Unable to delegate cons, you need to regproducer" );
         uint64_t curblocknum = (uint64_t)tapos_block_num();
         if(stake_cons_delta.amount < 0){
            print("undelegatecons from:",name{from}," receiver:",name{receiver}," tapos_block_num:",curblocknum," it->last_operate_blocknum:",it->last_operate_blocknum);//
            const uint32_t seconds_per_block     = block_interval_seconds();
            uint32_t blocks_per_month            = seconds_per_year / seconds_per_block / 12;
            ultrainio_assert( (curblocknum - it->last_operate_blocknum) > blocks_per_month , "should stake at least more than one month" );
         }
         auto enabled = ((it->total_cons_staked+total_update.amount) >=
                  _gstate.min_activated_stake/_gstate.min_committee_member);
         _producers.modify(it, 0 , [&](auto & v) {
                  v.total_cons_staked += total_update.amount;
                  v.is_enabled = enabled;
                  v.last_operate_blocknum = curblocknum;
                  });
         if(it->is_on_master_chain()) {
             if(enabled && !it->hasactived) {
                 update_activated_stake(it->total_cons_staked);
                 _producers.modify(it, 0 , [&](auto & v) {
                         v.hasactived = true;
                     });
             }
             else if(it->hasactived){
                 update_activated_stake(total_update.amount);
             }
         }
         else if (enabled) {
             if(it->is_in_pending_queue()) {
                 add_to_pending_queue(it->owner, it->producer_key);
             }
             else {
                 add_to_subchain(it->location, it->owner, it->producer_key);
             }
         }
         else {
             if(it->is_in_pending_queue()) {
                 remove_from_pending_queue(it->owner);
             }
             else {
                 remove_from_subchain(it->location, it->owner);
             }
         }
      }
   }
   void system_contract::resourcelease( account_name from, account_name receiver,
                          int64_t combosize, int64_t days, uint64_t location){
      require_auth( from );
      ultrainio_assert( combosize >= 0, "must stake a positive resources package  amount" );
      auto max_availableused_size = _gstate.max_resources_size - _gstate.total_resources_staked;
      std::string availableuserstr = "resources lease package available amount:"+ std::to_string(max_availableused_size);
      ultrainio_assert( combosize <= max_availableused_size, availableuserstr.c_str() );
      uint64_t bytes = (_gstate.max_ram_size-2ll*1024*1024*1024)/_gstate.max_resources_size;
      ultrainio_assert( days >= 0 && days <=365*30, "resource lease buy days must reserve a positive and less than 30 years" );

      // update totals of "receiver"
      {
         int64_t cuttingfee = 0;
         auto reslease_itr = _reslease_tbl.find( receiver );
         if( reslease_itr ==  _reslease_tbl.end() ) {
            ultrainio_assert( (combosize > 0) && (days > 0), "resource lease buy days and numbler must > 0" );
            cuttingfee = days*combosize;
            _gstate.total_resources_staked += combosize;
            reslease_itr = _reslease_tbl.emplace( from, [&]( auto& tot ) {
                  tot.owner = receiver;
                  tot.lease_num = combosize;
                  tot.start_time = now();
                  tot.end_time = tot.start_time + (uint32_t)days*seconds_per_day;
               });
            _gstate.total_ram_bytes_reserved += (uint64_t)combosize*bytes;
         } else {
            ultrainio_assert(((combosize > 0) && (days == 0))||((combosize == 0) && (days > 0)), "resource lease days and numbler can't increase them at the same time" );
            if(combosize > 0)
            {
               _gstate.total_resources_staked += combosize;
               ultrainio_assert(reslease_itr->end_time > now(), "resource lease endtime already expired" );
               double remain_time = (reslease_itr->end_time - now())/(double)seconds_per_day;
               cuttingfee = (int64_t)(ceil(remain_time))*combosize;
               print("resourcelease remain_time:",remain_time," cuttingfee:",cuttingfee);
               _gstate.total_ram_bytes_reserved += (uint64_t)combosize*bytes;
            } else if(days > 0)
            {
               ultrainio_assert(reslease_itr->lease_num > 0, "resource lease number is not normal" );
               cuttingfee = days*reslease_itr->lease_num;
            }
            _reslease_tbl.modify( reslease_itr, 0, [&]( auto& tot ) {
                  tot.lease_num += combosize;
                  tot.end_time  += days*seconds_per_day;
               });
         }
         ultrainio_assert(cuttingfee > 0, "resource lease cuttingfee is not normal" );
         INLINE_ACTION_SENDER(ultrainio::token, transfer)( N(utrio.token), {from,N(active)},
                                             { from, N(utrio.fee), asset((int64_t)ceil((double)10000*640*cuttingfee/0.3/365)), std::string("buy resource lease") } );

         ultrainio_assert( 0 < reslease_itr->lease_num, "insufficient resource lease" );
         set_resource_limits( receiver, (int64_t)bytes*reslease_itr->lease_num, reslease_itr->lease_num, reslease_itr->lease_num );
         print("current resource limit net_weight:",reslease_itr->lease_num," cpu:",reslease_itr->lease_num," ram:",(int64_t)bytes*reslease_itr->lease_num);

      } // tot_itr can be invalid, should go out of scope
   }

   void system_contract::delegatebw( account_name from, account_name receiver,
                                     asset stake_net_quantity,
                                     asset stake_cpu_quantity, bool transfer )
   {
      ultrainio_assert( stake_cpu_quantity >= asset(0), "must stake a positive amount" );
      ultrainio_assert( stake_net_quantity >= asset(0), "must stake a positive amount" );
      ultrainio_assert( stake_net_quantity + stake_cpu_quantity > asset(0), "must stake a positive amount" );
      ultrainio_assert( !transfer || from != receiver, "cannot use transfer flag if delegating to self" );

      changebw( from, receiver, stake_net_quantity, stake_cpu_quantity, transfer);
   } // delegatebw

   void system_contract::undelegatebw( account_name from, account_name receiver,
                                       asset unstake_net_quantity, asset unstake_cpu_quantity )
   {
      ultrainio_assert( asset() <= unstake_cpu_quantity, "must unstake a positive amount" );
      ultrainio_assert( asset() <= unstake_net_quantity, "must unstake a positive amount" );
      ultrainio_assert( asset() < unstake_cpu_quantity + unstake_net_quantity, "must unstake a positive amount" );
      ultrainio_assert( _gstate.total_activated_stake >= _gstate.min_activated_stake,
                    "cannot undelegate bandwidth until the chain is activated (at least 15% of all tokens participate in voting)" );

      changebw( from, receiver, -unstake_net_quantity, -unstake_cpu_quantity, false);
   } // undelegatebw

void system_contract::delegatecons( account_name from, account_name receiver,asset stake_cons_quantity)
   {
      ultrainio_assert( stake_cons_quantity >= asset(0), "must stake a positive amount" );
      change_cons( from, receiver, stake_cons_quantity);
   } // delegatecons

   void system_contract::undelegatecons( account_name from, account_name receiver)
   {
      ultrainio_assert( _gstate.total_activated_stake >= _gstate.min_activated_stake,
                    "cannot undelegate cons until the chain is activated (at least 15% of all tokens participate in voting)" );

      change_cons( from, receiver, asset(-1));
   } // undelegatecons

   void system_contract::refund( const account_name owner ) {
      require_auth( owner );

      refunds_table refunds_tbl( _self, owner );
      auto req = refunds_tbl.find( owner );
      ultrainio_assert( req != refunds_tbl.end(), "refund request not found" );
      ultrainio_assert( req->request_time + refund_delay <= now(), "refund is not available yet" );
      // Until now() becomes NOW, the fact that now() is the timestamp of the previous block could in theory
      // allow people to get their tokens earlier than the 3 day delay if the unstake happened immediately after many
      // consecutive missed blocks.

      INLINE_ACTION_SENDER(ultrainio::token, transfer)( N(utrio.token), {N(utrio.stake),N(active)},
                                                    { N(utrio.stake), req->owner, req->net_amount + req->cpu_amount, std::string("unstake") } );

      refunds_tbl.erase( req );
   }

   void system_contract::refundcons(const account_name owner ) {
      require_auth( owner );

      refunds_cons_table refunds_tbl( _self, owner );
      auto req = refunds_tbl.find( owner );
      ultrainio_assert( req != refunds_tbl.end(), "refunds_cons_table request not found" );
      ultrainio_assert( req->request_time + refund_delay <= now(), "refund is not available yet" );
      // Until now() becomes NOW, the fact that now() is the timestamp of the previous block could in theory
      // allow people to get their tokens earlier than the 3 day delay if the unstake happened immediately after many
      // consecutive missed blocks.

      INLINE_ACTION_SENDER(ultrainio::token, transfer)( N(utrio.token), {N(utrio.stake),N(active)},
                                                    { N(utrio.stake), req->owner, req->cons_amount, std::string("unstake") } );

      refunds_tbl.erase( req );
   }
   void system_contract::recycleresource(const account_name owner ,uint64_t lease_num) {
      require_auth( _self );
      int64_t ram_bytes = 0;
      get_account_ram_usage( owner, &ram_bytes );
      print("checkresexpire  recycleresource account:",name{owner}," ram_used:",ram_bytes);
      set_resource_limits( owner, ram_bytes, 0, 0 );
      if(_gstate.total_resources_staked >= lease_num)
         _gstate.total_resources_staked -= lease_num;
      uint64_t bytes = (_gstate.max_ram_size-2ll*1024*1024*1024)/_gstate.max_resources_size;
      if(_gstate.total_ram_bytes_reserved >= (lease_num*bytes - (uint64_t)ram_bytes))
         _gstate.total_ram_bytes_reserved =_gstate.total_ram_bytes_reserved - lease_num*bytes + (uint64_t)ram_bytes;
   }

   void system_contract::checkresexpire(){
      time curtime = now();
      if(_gstate.last_check_resexpiretime == 0)
         _gstate.last_check_resexpiretime = now();
      if(_gstate.last_check_resexpiretime < curtime && (curtime - _gstate.last_check_resexpiretime) >= seconds_per_day){
         _gstate.last_check_resexpiretime = curtime;
         for(auto leaseiter = _reslease_tbl.begin(); leaseiter != _reslease_tbl.end(); ){
            if(leaseiter->end_time <= curtime){
               print("checkresexpire reslease name:",name{leaseiter->owner}, " leaseiter->end_time:",leaseiter->end_time," curtime:",curtime);
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
      }
   }

} //namespace ultrainiosystem
