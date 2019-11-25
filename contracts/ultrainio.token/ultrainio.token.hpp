/**
 *  @file
 *  @copyright defined in ultrain/LICENSE.txt
 */
#pragma once

#include <ultrainiolib/asset.hpp>
#include <ultrainiolib/ultrainio.hpp>

#include <string>

namespace ultrainiosystem {
   class system_contract;
}

namespace ultrainio {
   static constexpr int64_t default_transfer_fee = 2000; //The default transfer fee is 0.2UGAS
   using std::string;

   class token : public contract {
      public:
         token( account_name self ):contract(self){}

         void create( account_name issuer,
                      asset        maximum_supply);

         void issue( account_name to, asset quantity, string memo );

         void burn( asset quantity );

         void transfer( account_name from,
                        account_name to,
                        asset        quantity,
                        string       memo );

         void set_chargeparams( std::string symbol, uint8_t precision, uint32_t operate_interval, uint32_t operate_fee, bool is_forbid_trans);

         void set_trans_fee( int64_t fee );

         inline asset get_supply( symbol_name sym )const;

         inline asset get_balance( account_name owner, symbol_name sym )const;

         inline int64_t get_transfer_fee() const;

      private:
         struct account {
            asset    balance;
            uint32_t  last_block_height;
            uint64_t primary_key()const { return balance.symbol.name(); }
         };

         struct currency_stats {
            asset          supply;
            asset          max_supply;
            account_name   issuer;
            uint32_t       operate_interval_sec = 60;  //default transfer interval of more than one minute does not charge
            uint32_t       operate_fee = 100;
            bool           is_forbid_transfer = false;

            uint64_t primary_key()const { return supply.symbol.name(); }
         };

         struct trans_fee {
            uint64_t       block_height = 0;  //The middle fee from the block height (not included) to the next block height greater than it is fee
            int64_t        fee = 2000;
            uint64_t  primary_key()const { return block_height; }
            ULTRAINLIB_SERIALIZE( trans_fee, (block_height)(fee) )
         };

         typedef ultrainio::multi_index<N(accounts), account> accounts;
         typedef ultrainio::multi_index<N(stat), currency_stats> stats;
         typedef ultrainio::multi_index< N(transfee), trans_fee>  transfee;

         void sub_balance( account_name owner, asset value );
         void add_balance( account_name owner, asset value );

      public:
         struct transfer_args {
            account_name  from;
            account_name  to;
            asset         quantity;
            string        memo;
         };
   };

   asset token::get_supply( symbol_name sym )const
   {
      stats statstable( _self, sym );
      const auto& st = statstable.get( sym );
      return st.supply;
   }

   asset token::get_balance( account_name owner, symbol_name sym )const
   {
      accounts accountstable( _self, owner );
      auto symitr = accountstable.find( sym );
      if(symitr == accountstable.end())
         return asset(0);
      else
         return symitr->balance;
   }

} /// namespace ultrainio
