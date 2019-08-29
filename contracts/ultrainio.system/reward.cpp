#include "ultrainio.system.hpp"

#include <ultrainio.token/ultrainio.token.hpp>
#include <ultrainiolib/system.h>
#include <ultrainiolib/transaction.hpp>
#include <cmath>
namespace ultrainiosystem {
   using namespace ultrainio;
   void system_contract::onblock( block_timestamp timestamp, account_name producer ) {

      require_auth(N(ultrainio));
      if(_gstate.cur_committee_number < _gstate.min_committee_member_number){
         print("func onblock:the current number of committees must be greater than the minimum to be eligible for new awards");
         return;
      }

      /**
       * At startup the initial producer may not be one that is registered / elected
       * and therefore there may be no producer object for them.
       */
      producers_table _producers(_self, self_chain_name);
      auto head_block_height = (uint64_t)head_block_number() + 1;
      if ( !_gstate.start_block){
         uint32_t i {};
         for(auto itr = _producers.begin(); i <= _gstate.min_committee_member_number && itr != _producers.end(); ++itr, ++i){}
		 if( i > _gstate.min_committee_member_number){
			_gstate.start_block = head_block_height;
         }else{
            return;
         }
      }

      auto prod = _producers.find(producer);
      if ( prod != _producers.end() ) {
         _gstate.total_cur_chain_block++;
         _producers.modify( prod, [&](auto& p ) {
               p.total_produce_block++;
               p.last_record_blockheight = head_block_height;
         });
         print( "onblock timestamp:", timestamp.abstime, " producer:", name{producer}," produce_block:", prod->total_produce_block, "\n" );
      }
      del_expire_table(); //Delete the expired account table
      check_res_expire();
      check_bulletin();
      pre_schedule();
      distribut_reward();  //automatically send rewards
      check_producer_lastblock( self_chain_name, head_block_height );
   }
   float system_contract::get_reward_fee_ratio() const {
      uint32_t charge_ratio = 1; //The default handling charge is 1%
      for( auto extension : _gstate.table_extension ){
         if(extension.key == ultrainio_global_state::global_state_exten_type_key::sidechain_charge_ratio) {
            if(extension.value.empty())
               break;
            charge_ratio = std::stoul(extension.value);
            if( charge_ratio > 100 )
               charge_ratio = 1;
            break;
         }
      }
      return (100 - charge_ratio) / (float)100;
   }
   uint64_t system_contract::get_reward_per_block() const {
      uint64_t  reward_per_block = 0;
      auto const reward_iter = std::find_if( _gstate.block_reward_vec.begin(), _gstate.block_reward_vec.end(), [&](ultrainio::block_reward const& obj){
                                       return obj.consensus_period == block_interval_seconds();
                                 } );
      if(reward_iter != _gstate.block_reward_vec.end()) {
         reward_per_block = reward_iter->reward;
      }
      return reward_per_block;
   }
   uint32_t system_contract::check_previous_claimed_reward( const ultrainiosystem::producer_info& prod, uint32_t block_height ) const {
      uint32_t previous_blockheight_sub = 0;
      for( auto& exten : prod.table_extension ){
         if( exten.key == producer_info::producers_state_exten_type_key::claim_rewards_block_height ){
            uint32_t pre_rewards_blockheight = std::stoul(exten.value);
            ultrainio_assert( (block_height > pre_rewards_blockheight)
               && (block_height - pre_rewards_blockheight) > seconds_per_hour/block_interval_seconds(),
               " already claimed rewards within past hour" );
            previous_blockheight_sub = block_height - pre_rewards_blockheight;
            break;
         }
      }
      return previous_blockheight_sub;
   }

   void system_contract::send_rewards_for_producer( account_name producer, account_name reward_account, const name& chain_name, uint64_t unpaid_balance ) {
      if( !has_auth( N(u.reward.1) ) ) {
         require_auth( reward_account );
      }
      account_name pay_account = ( chain_name == self_chain_name ) ? N(utrio.mfee) : N(utrio.resfee) ;
      asset pay_tokens = ultrainio::token(N(utrio.token)).get_balance( pay_account,symbol_type(CORE_SYMBOL).name());
      ultrainio_assert( pay_tokens.amount >= (int64_t)unpaid_balance, " Insufficient funds to claim rewards" );

      INLINE_ACTION_SENDER(ultrainio::token, safe_transfer)( N(utrio.token), {pay_account,N(active)},
         { pay_account, reward_account, asset((int64_t)unpaid_balance), name{producer}.to_string() + std::string(" produce block pay") } );
      print("\nsend_rewards_for_producer subchainname:",name{chain_name}," pay_tokens:",pay_tokens.amount," producer:",name{producer},
         " reward_account:",name{reward_account}," unpaid_balance:",unpaid_balance, "\n");
      char result_str[512] = "";
      std::sprintf( result_str,"{\"chain_name\":\"%s\",\"producer\":\"%s\",\"reward_account\":\"%s\",\"token\":\"%s\"}",
                                       name{chain_name}.to_string().c_str(),
                                       name{producer}.to_string().c_str(),
                                       name{reward_account}.to_string().c_str(),
                                       std::to_string( unpaid_balance ).c_str() );
      set_result_str( result_str );
      ultrainio_assert( _gstate.total_unpaid_balance >= unpaid_balance, " unpaid balance exception" );
      _gstate.total_unpaid_balance -= unpaid_balance;
   }

   void system_contract::onfinish() {
      print("onfinish in ultrainio.system");
      require_auth(N(ultrainio));
   }

   void system_contract::record_rewards_for_disproducer( account_name producer, account_name reward_account, uint64_t unpaid_balance ) {
      if( unpaid_balance <= 0 ) return;
      unpaid_disprod _unpaid_disproducer( _self, _self );
      auto unprod_itr = _unpaid_disproducer.find( producer );
      ultrainio_assert( unprod_itr == _unpaid_disproducer.end(), " _unpaid_disproducer alreay exist " );
      _unpaid_disproducer.emplace( [&]( auto& u ) {
         u.owner = producer;
         u.reward_account = reward_account;
         u.unpaid_balance = unpaid_balance;
      });
   }

   uint64_t system_contract::remove_rewards_for_enableproducer( account_name producer ) {
      uint64_t unpaid_balance = 0;
      unpaid_disprod _unpaid_disproducer( _self, _self );
      auto unprod_itr = _unpaid_disproducer.find( producer );
      if( unprod_itr == _unpaid_disproducer.end() )
         return unpaid_balance;
      unpaid_balance = unprod_itr->unpaid_balance;
      _unpaid_disproducer.erase( unprod_itr );
      return unpaid_balance;
   }

   void system_contract::record_rewards_for_maintainer( account_name maintainer, uint64_t unpaid_balance ) {
      unpaid_disprod _unpaid_maintainer( _self, N(maintainer) );
      auto unmaint_itr = _unpaid_maintainer.find( maintainer );
      if( unmaint_itr == _unpaid_maintainer.end() ){
         _unpaid_maintainer.emplace( [&]( auto& u ) {
            u.owner = maintainer;
            u.unpaid_balance = unpaid_balance;
         });
      } else {
         _unpaid_maintainer.modify( unmaint_itr, [&](auto& u ) {
            u.unpaid_balance += unpaid_balance;
         });
      }
   }

   void system_contract::send_rewards_for_maintainer( account_name maintainer ) {
      unpaid_disprod _unpaid_maintainer( _self, N(maintainer) );
      auto unmaint_itr = _unpaid_maintainer.find( maintainer );
      ultrainio_assert( unmaint_itr != _unpaid_maintainer.end(), " _unpaid_maintainer not exist " );
      ultrainio_assert( unmaint_itr->unpaid_balance > 0, " _unpaid_maintainer balance is zero " );
      send_rewards_for_producer( maintainer, maintainer, default_chain_name, unmaint_itr->unpaid_balance );
      _unpaid_maintainer.modify( unmaint_itr, [&](auto& u ) {
         u.unpaid_balance = 0;
      });
   }

   void system_contract::report_subchain_block( account_name producer, uint64_t block_height ) {
      auto briefprod = _briefproducers.find(producer);
      if(briefprod == _briefproducers.end()) {
          print("error: block proposer ", name{producer}, " is not a producer\n");
          return;
      }
      producers_table _producers(_self, briefprod->location);
      auto prod = _producers.find(producer);
      if ( prod == _producers.end() ) {
         print("error: block proposer ", name{producer}, " is not found in its location\n");
         return;
      }
      const uint64_t rewardvalue = get_reward_per_block();
      const uint64_t reward_percentage = rewardvalue / 100;
      record_rewards_for_maintainer( ultrainio_community_name, reward_percentage * 5 );
      record_rewards_for_maintainer( ultrainio_technical_team_name, reward_percentage * 5 );
      record_rewards_for_maintainer( ultrainio_dapp_name, reward_percentage * 10 );
      uint64_t realreward = (uint64_t)( reward_percentage * 80 * get_reward_fee_ratio() );
      _gstate.total_unpaid_balance += realreward + reward_percentage * 20;
      if( rewardvalue > (realreward + reward_percentage * 20) )
         _gstate.master_chain_pay_fee += rewardvalue - realreward - reward_percentage * 20;
      _producers.modify( prod, [&](auto& p ) {
          p.unpaid_balance += realreward;
          p.total_produce_block++;
          p.last_record_blockheight = block_height;
      });
      check_producer_lastblock( briefprod->location, block_height );
      print( "reportsubchainblock chain_name:", name{briefprod->location}, " producer:", name{producer}, " produce_block:", prod->total_produce_block,"\n" );
   }

   void system_contract::claimrewards( account_name producer ) {
      ultrainio_assert( _gstate.is_master_chain(), "It's not that the master chain can't get rewards" );
      ultrainio_assert( _gstate.cur_committee_number >= _gstate.min_committee_member_number,
         "The current number of committees must be greater than the minimum to be eligible for new awards");
      if( producer == ultrainio_community_name
         || producer == ultrainio_technical_team_name
         || producer == ultrainio_dapp_name ){
         send_rewards_for_maintainer( producer );
         return;
      }
      const auto& briefprod = _briefproducers.get( producer, "producer not found" );
      if( briefprod.in_disable ) {  //disabled producer claimrewards
         unpaid_disprod _unpaid_disproducer( _self, _self );
         const auto& unprod = _unpaid_disproducer.get( producer ,"unpaid_disproducer not found" );
         ultrainio_assert( unprod.unpaid_balance != 0, " unpaid balance is zero" );
         send_rewards_for_producer( producer, unprod.reward_account, briefprod.location, unprod.unpaid_balance );
         _unpaid_disproducer.erase( unprod );
      } else {  //enabled producer claimrewards
         producers_table _producers( _self, briefprod.location );
         const auto& prod = _producers.get( producer,"chain name is not found" );
         ultrainio_assert( prod.unpaid_balance != 0, " unpaid balance is zero" );

         auto block_height = (uint32_t)head_block_number() + 1;
         check_previous_claimed_reward( prod, block_height );
         send_rewards_for_producer( producer, prod.claim_rewards_account, briefprod.location, prod.unpaid_balance );
         _producers.modify( prod, [&](auto& p) {
            p.unpaid_balance = 0;
            bool is_exist_previous_reward_block = false;
            for( auto& exten : p.table_extension ){
               if( exten.key == producer_info::producers_state_exten_type_key::claim_rewards_block_height ){
                  exten.value = std::to_string(block_height);
                  is_exist_previous_reward_block = true;
                  break;
               }
            }
            if( !is_exist_previous_reward_block )
               p.table_extension.push_back(exten_type(producer_info::producers_state_exten_type_key::claim_rewards_block_height ,std::to_string(block_height)));
         });
      }
   }

   void system_contract::calcmasterrewards() {
      require_auth(_self);
      if(!_gstate.is_master_chain())
         return;
      ultrainio_assert( _gstate.cur_committee_number >= _gstate.min_committee_member_number,
         "calcmasterrewards The current number of committees must be greater than the minimum to be eligible for new awards");
      asset fee_tokens = ultrainio::token(N(utrio.token)).get_balance( N(utrio.fee),symbol_type(CORE_SYMBOL).name());
      asset master_reward_tokens = fee_tokens;
      if(_gstate.master_chain_pay_fee > 0){
         master_reward_tokens += asset(_gstate.master_chain_pay_fee);
         asset resfee_tokens = ultrainio::token(N(utrio.token)).get_balance( N(utrio.resfee),symbol_type(CORE_SYMBOL).name());
         if( resfee_tokens.amount > _gstate.master_chain_pay_fee )
            INLINE_ACTION_SENDER(ultrainio::token, safe_transfer)( N(utrio.token), { N(utrio.resfee),N(active) },
               { N(utrio.resfee), N(utrio.mfee), asset(_gstate.master_chain_pay_fee), std::string("master producers block pay") } );
         _gstate.master_chain_pay_fee = 0;
      }
      if( master_reward_tokens.amount <= 0 )
         return;
      if ( fee_tokens.amount > 0 ) {
         INLINE_ACTION_SENDER(ultrainio::token, safe_transfer)( N(utrio.token), { N(utrio.fee),N(active) },
            { N(utrio.fee), N(utrio.mfee), fee_tokens, std::string("master producers block pay") } );
      }
      uint64_t master_paid_balance = 0;
      producers_table _producers(_self, self_chain_name);
      for(auto itr = _producers.begin(); itr != _producers.end(); ++itr){
         if(itr->total_produce_block == 0 || _gstate.total_cur_chain_block <= 0)
            continue;
         auto producer_unpaid_balance = (uint64_t)floor((double)itr->total_produce_block/_gstate.total_cur_chain_block * master_reward_tokens.amount );
         master_paid_balance += producer_unpaid_balance;
         print("\ncalcmasterrewards master producer:",name{itr->owner}," reward_account:",name{itr->claim_rewards_account}," cur_producer_block:",itr->total_produce_block," producer_pay_balance:",producer_unpaid_balance,"\n");
         _producers.modify( itr, [&](auto& p) {
            p.unpaid_balance += producer_unpaid_balance;
         });
      }
      print("\ncalcmasterrewards total cur_chain_block:",_gstate.total_cur_chain_block,
            " master_paid_balance:",master_paid_balance," master_reward_tokens:",master_reward_tokens.amount,"\n");
      _gstate.total_unpaid_balance += master_paid_balance;
   }

   void system_contract::distribut_reward(){
      if(!_gstate.is_master_chain())
         return;
      uint32_t block_height = (uint32_t)head_block_number() + 1;
      uint32_t interval_num = seconds_per_day/block_interval_seconds()/24;   // calcmasterrewards once an hour
      if(block_height < 20 || block_height%interval_num != 0) {
         return;
      }
      //distribute rewards
      ultrainio::transaction rewardtrans;
      rewardtrans.actions.emplace_back(
          permission_level{ _self, N(active) }, _self, NEX(calcmasterrewards), std::make_tuple() );
      rewardtrans.actions[0].authorization.emplace_back(permission_level{ N(ultrainio), N(active)});
      rewardtrans.delay_sec = 1;
      uint128_t trxid = now() + _self + N(calcmasterrewards);
      cancel_deferred(trxid);
      rewardtrans.send( trxid, _self, true );
      print("distributreward currenttime:",current_time()," trxid:",trxid);
   }

} //namespace ultrainiosystem
