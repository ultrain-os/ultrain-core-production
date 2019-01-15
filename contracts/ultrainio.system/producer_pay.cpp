#include "ultrainio.system.hpp"

#include <ultrainio.token/ultrainio.token.hpp>
#include <ultrainiolib/system.h>
#include <ultrainiolib/transaction.h>
#include <ultrainiolib/transaction.hpp>

namespace ultrainiosystem {

   const int64_t  min_pervote_daily_pay = 100'0000;
   uint32_t reward_preblock = 3;

   void system_contract::onblock( block_timestamp timestamp, account_name producer ) {
      using namespace ultrainio;

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
			_gstate.start_block=(uint64_t)tapos_block_num();
         }else{
            return;
         }
      }

	  auto prod = _producers.find(producer);
      if ( prod != _producers.end() ) {
         _gstate.total_unpaid_blocks++;
         _producers.modify( prod, 0, [&](auto& p ) {
               p.unpaid_blocks++;
               p.total_produce_block++;
         });
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
              idx.modify( highest, 0, [&]( auto& b ){
                                          b.high_bid = -b.high_bid;
                                      });
          }
      }
      checkresexpire();
      cleanvotetable();
      activate_committee_update();
      distributreward();
   }

   void system_contract::reportblocknumber( account_name producer, uint64_t number) {
      using namespace ultrainio;

      //require_auth(N(ultrainio));
      /** until activated stake crosses this threshold no new rewards are paid */
      if( _gstate.total_activated_stake < _gstate.min_activated_stake )
         return;
      if ( !_gstate.start_block){
         uint32_t i {};
         for(auto itr = _producers.begin(); i <= _gstate.min_committee_member_number && itr != _producers.end(); ++itr, ++i){}
		 if( i > _gstate.min_committee_member_number){
			_gstate.start_block=(uint64_t)tapos_block_num();
         }else{
            return;
         }
      }

      auto prod = _producers.find(producer);
      if ( prod != _producers.end() ) {
         _gstate.total_unpaid_blocks += number;
         _producers.modify( prod, 0, [&](auto& p ) {
               p.unpaid_blocks += number;
               p.total_produce_block++;
         });
      }
   }

   using namespace ultrainio;
   void system_contract::claimrewards() {
      require_auth(_self);
      uint64_t p10 = symbol_type(system_token_symbol).precision();
      int64_t new_tokens = 0;
      new_tokens = static_cast<int64_t>(_gstate.total_unpaid_blocks*reward_preblock);
      _gstate.total_unpaid_blocks = 0;
      new_tokens *= p10;
      asset fee_tokens = ultrainio::token(N(utrio.token)).get_balance(N(utrio.fee),symbol_type(CORE_SYMBOL).name());
      print("\nclaimrewards new_tokens:",new_tokens," fee_tokens:",fee_tokens.amount,"\n");
      if(fee_tokens.amount < new_tokens){
         INLINE_ACTION_SENDER(ultrainio::token, issue)( N(utrio.token), {{N(ultrainio),N(active)}},
                                                      {N(ultrainio), asset(new_tokens), std::string("issue tokens for claimrewards")} );
      }
      for(auto itr = _producers.begin(); itr != _producers.end(); ++itr){
         if(itr->unpaid_blocks > 0 && itr->is_active){
            ultrainio_assert( _gstate.total_activated_stake >= _gstate.min_activated_stake,
                        "cannot claim rewards until the chain is activated (at least 15% of all tokens participate in voting)" );
            int64_t producer_per_block_pay = 0;
            producer_per_block_pay += static_cast<int64_t>(itr->unpaid_blocks*reward_preblock);
            producer_per_block_pay *= p10;
            print("\nclaimrewards producer_pay:",producer_per_block_pay,"\n");
            _producers.modify( itr, 0, [&](auto& p) {
               p.unpaid_blocks = 0;
            });

            if( producer_per_block_pay > 0 ) {
               uint64_t pay_account = 0;
               if(fee_tokens.amount < producer_per_block_pay)
                  pay_account = N(ultrainio);
               else
                  pay_account = N(utrio.fee);
               INLINE_ACTION_SENDER(ultrainio::token, transfer)( N(utrio.token), {pay_account,N(active)},
                                                            { pay_account, itr->owner, asset(producer_per_block_pay), std::string("producer block pay") } );
            }
         }
      }
   }

   void system_contract::distributreward(){
      auto block_height = tapos_block_num();
      if(block_height > 120 && block_height%(20) != 0) {
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
