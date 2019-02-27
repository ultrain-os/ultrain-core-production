#include "ultrainio.system.hpp"

#include <ultrainio.token/ultrainio.token.hpp>
#include <ultrainiolib/system.h>
#include <ultrainiolib/transaction.h>
#include <ultrainiolib/transaction.hpp>
#include <cmath>
namespace ultrainiosystem {
   using namespace ultrainio;
   void system_contract::onblock( block_timestamp timestamp, account_name producer ) {

      require_auth(N(ultrainio));

      /** until activated stake crosses this threshold no new rewards are paid */
      if( _gstate.total_activated_stake < _gstate.min_activated_stake )
         return;

      /**
       * At startup the initial producer may not be one that is registered / elected
       * and therefore there may be no producer object for them.
       */
      if ( !_gstate.start_block){
         uint32_t i {};
         for(auto itr = _producers.begin(); i <= _gstate.min_committee_member_number && itr != _producers.end(); ++itr, ++i){}
		 if( i > _gstate.min_committee_member_number){
			_gstate.start_block=(uint64_t)head_block_number() + 1;
         }else{
            return;
         }
      }

      auto prod = _producers.find(producer);
      if ( prod != _producers.end() ) {
        auto const iter = std::find_if( _gstate.block_reward_vec.begin(), _gstate.block_reward_vec.end(), [&](ultrainio::block_reward const& obj){
                            return obj.consensus_period == block_interval_seconds();
                        } );
        uint64_t curblockreward = 0;
        if(iter != _gstate.block_reward_vec.end()){
            curblockreward = iter->reward;
        }
         _gstate.total_unpaid_balance += curblockreward;
         _producers.modify( prod, [&](auto& p ) {
               p.unpaid_balance += curblockreward;
               p.total_produce_block++;
         });
         print( "onblock timestamp:", timestamp.abstime, " producer:", name{producer}," produce_block:", prod->total_produce_block, "\n" );
      }

      if( (timestamp.abstime - _gstate.last_name_close.abstime) > seconds_per_day ) {
          name_bid_table bids(_self,_self);
          auto idx = bids.get_index<N(highbid)>();
          auto highest = idx.begin();
          if( highest != idx.end() &&
              highest->high_bid > 0 &&
              highest->last_bid_time < (current_time() - useconds_per_day) &&
              _gstate.thresh_activated_stake_time > 0 &&
              (current_time() - _gstate.thresh_activated_stake_time) > 14 * useconds_per_day ) {
              _gstate.last_name_close = timestamp;
              idx.modify( highest, [&]( auto& b ){
                                          b.high_bid = -b.high_bid;
                                      });
          }
      }
      checkresexpire();
      cleanvotetable();
      checkbulletin();
      activate_committee_update();
      schedule(); //do it after activate committee update
      distributreward();
   }

   void system_contract::reportblocknumber( uint64_t chain_type, account_name producer, uint64_t number) {
      auto prod = _producers.find(producer);
      if ( prod != _producers.end() ) {
        uint64_t curblockreward = 0;
        chaintypes_table type_tbl(_self, _self);
        auto typeiter = type_tbl.find(chain_type);
        if (typeiter != type_tbl.end()) {
            auto const iter = std::find_if( _gstate.block_reward_vec.begin(), _gstate.block_reward_vec.end(), [&](ultrainio::block_reward const& obj){
                                return obj.consensus_period == typeiter->consensus_period;
                            } );
            if(iter != _gstate.block_reward_vec.end()){
                curblockreward = iter->reward;
            }
        }
         _gstate.total_unpaid_balance += curblockreward;
         _producers.modify( prod, [&](auto& p ) {
               p.unpaid_balance += curblockreward;
               p.total_produce_block += number;
         });
         print( "reportblocknumber number:", number, " producer:", name{producer}, " produce_block:", prod->total_produce_block,"\n" );
      }
   }

   void system_contract::claimrewards() {
      require_auth(_self);
      int64_t new_tokens = (int64_t)_gstate.total_unpaid_balance;
      _gstate.total_unpaid_balance = 0;
      asset fee_tokens = ultrainio::token(N(utrio.token)).get_balance(N(utrio.fee),symbol_type(CORE_SYMBOL).name());
      print("\nclaimrewards new_tokens:",new_tokens," fee_tokens:",fee_tokens.amount,"\n");
      if(fee_tokens.amount < new_tokens){
         INLINE_ACTION_SENDER(ultrainio::token, issue)( N(utrio.token), {{N(ultrainio),N(active)}},
                                                      {N(ultrainio), asset(new_tokens), std::string("issue tokens for claimrewards")} );
      }
      for(auto itr = _producers.begin(); itr != _producers.end(); ++itr){
         if(itr->unpaid_balance > 0 && itr->is_active){
            ultrainio_assert( _gstate.total_activated_stake >= _gstate.min_activated_stake,
                        "cannot claim rewards until the chain is activated" );
            auto producer_per_block_pay = (int64_t)itr->unpaid_balance;
            print("\nclaimrewards producer_pay:",producer_per_block_pay,"\n");
            _producers.modify( itr, [&](auto& p) {
               p.unpaid_balance = 0;
            });

            uint64_t pay_account = 0;
            if(fee_tokens.amount < producer_per_block_pay)
                pay_account = N(ultrainio);
            else
                pay_account = N(utrio.fee);
            INLINE_ACTION_SENDER(ultrainio::token, safe_transfer)( N(utrio.token), {pay_account,N(active)},
                                                        { pay_account, itr->owner, asset(producer_per_block_pay), std::string("producer block pay") } );
         }
      }
   }

   void system_contract::distributreward(){
      if(!is_master_chain())
         return;
      uint32_t block_height = (uint32_t)head_block_number() + 1;
      uint32_t interval_num = seconds_per_day*7/block_interval_seconds();
      if(block_height < 120 || block_height%interval_num != 0) {
         return;
      }
      //distribute rewards
      ultrainio::transaction rewardtrans;
      rewardtrans.actions.emplace_back( permission_level{ _self, N(active) }, _self, NEX(claimrewards), std::make_tuple() );
      rewardtrans.actions[0].authorization.emplace_back(permission_level{ N(ultrainio), N(active)});
      rewardtrans.delay_sec = 1;
      uint128_t trxid = now() + _self + N(claimrewards);
      cancel_deferred(trxid);
      rewardtrans.send( trxid, _self, true );
      print("distributreward currenttime:",current_time()," trxid:",trxid);
   }
} //namespace ultrainiosystem
