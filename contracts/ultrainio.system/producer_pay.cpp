#include "ultrainio.system.hpp"

#include <ultrainio.token/ultrainio.token.hpp>
#include <ultrainiolib/transaction.h>

namespace ultrainiosystem {

   const int64_t  min_pervote_daily_pay = 100'0000;
   const uint32_t rate1                 = 50;
   const uint32_t rate2                 = 100;
   const uint32_t rate3                 = 150;
   const uint32_t rate4                 = 200;
   const uint32_t rate[num_rate]        = {rate1,rate2,rate3,rate4,rate3,rate2,rate1};

   const uint32_t seconds_per_block     = 10;
   const uint32_t blocks_per_year       = 52*7*24*3600/seconds_per_block;   // half seconds per year
   const uint32_t seconds_per_year      = 52*7*24*3600;
   const uint32_t blocks_per_day        = 24 * 3600/seconds_per_block;
   const uint32_t blocks_per_hour       = 3600/seconds_per_block;
   const uint64_t useconds_per_day      = 24 * 3600 * uint64_t(1000000);
   const uint64_t useconds_per_year     = seconds_per_year*1000000ll;


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
         for(auto itr = _producers.begin(); i < _gstate.min_committee_member_number && itr != _producers.end(); ++itr, ++i){}
		 if( i == _gstate.min_committee_member_number){
			_gstate.start_block=(uint64_t)tapos_block_num();
         }else{
            return;
         }
      }
	  auto prod = _producers.find(producer);
      if ( prod != _producers.end() ) {
	 /** TODO: blocks_per_day is for testing; remind to remove it */
	 //int temp = 2*(tapos_block_num()-(int)_gstate.start_block)/(int)blocks_per_year;
     	 int temp = 12*(tapos_block_num()-(int)_gstate.start_block)/(int)blocks_per_hour;
         const int interval = temp < num_rate ? temp:(num_rate-1);
         _gstate.total_unpaid_blocks[interval]++;
         _producers.modify( prod, 0, [&](auto& p ) {
               p.unpaid_blocks[interval]++;
         });
      }


      if( (timestamp.slot - _gstate.last_name_close.slot) > blocks_per_day ) {
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

   }

   using namespace ultrainio;
   void system_contract::claimrewards( const account_name& owner ) {
      require_auth(owner);

      const auto& prod = _producers.get( owner );
      ultrainio_assert( prod.active(), "producer does not have an active key" );

      ultrainio_assert( _gstate.total_activated_stake >= _gstate.min_activated_stake,
                    "cannot claim rewards until the chain is activated (at least 15% of all tokens participate in voting)" );

      auto ct = current_time();

      ultrainio_assert( ct - prod.last_claim_time > useconds_per_day, "already claimed rewards within past day" );

      uint64_t p10 = symbol_type(system_token_symbol).precision();
      int64_t new_tokens = 0;
      print("claimrewards gloable:\n[");
      for(int i=0;i<num_rate;++i){
	 print("{",i,", ",_gstate.total_unpaid_blocks[i],"},");
	 new_tokens += static_cast<int64_t>(_gstate.total_unpaid_blocks[i]*rate[i]);
	 _gstate.total_unpaid_blocks[i] = 0;
      }
      new_tokens*=p10;
      print("]\nclaimrewards new_tokens:",new_tokens,"\n");
      INLINE_ACTION_SENDER(ultrainio::token, issue)( N(utrio.token), {{N(ultrainio),N(active)}},
                                                    {N(ultrainio), asset(new_tokens), std::string("issue tokens for claimrewards")} );

      int64_t producer_per_block_pay = 0;
      print("claimrewards proudcer:\n[");
      for(int i=0;i<num_rate;++i){
	 print("{",i,", ",prod.unpaid_blocks[i],"},");
	 producer_per_block_pay += static_cast<int64_t>(prod.unpaid_blocks[i]*rate[i]);
      }
      producer_per_block_pay*=p10;
      print("]\nclaimrewards producer_pay:",producer_per_block_pay,"\n");
      _producers.modify( prod, 0, [&](auto& p) {
          p.last_claim_time = ct;
          for(int i=0;i<num_rate;++i) {
	     p.unpaid_blocks[i] = 0;
	  }
      });

      if( producer_per_block_pay > 0 ) {
         INLINE_ACTION_SENDER(ultrainio::token, transfer)( N(utrio.token), {N(ultrainio),N(active)},
                                                       { N(ultrainio), owner, asset(producer_per_block_pay), std::string("producer block pay") } );
      }
   }

} //namespace ultrainiosystem
