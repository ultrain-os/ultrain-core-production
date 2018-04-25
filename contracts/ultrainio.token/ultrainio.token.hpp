/**
 *  @file
 *  @copyright defined in ultrain/LICENSE.txt
 */

#include <ultrainiolib/ultrainio.hpp>
#include <ultrainiolib/asset.hpp>
#include <string>

namespace ultrainio {

   using std::string;

   class token : public contract {
      public:
         token( account_name self ):contract(self){}

         void create( account_name issuer,
                      asset        maximum_supply,
                      uint8_t      issuer_can_freeze,
                      uint8_t      issuer_can_recall,  
                      uint8_t      issuer_can_whitelist );


         void issue( account_name to, asset quantity, string memo );

         void transfer( account_name from, 
                        account_name to,
                        asset        quantity,
                        string       memo );


      private:
         struct account {
            asset    balance;
            bool     frozen    = false;
            bool     whitelist = true;

            uint64_t primary_key()const { return balance.symbol; }
         };

         struct currency_stats {
            asset          supply;
            asset          max_supply;
            account_name   issuer;
            bool           can_freeze         = true;
            bool           can_recall         = true;
            bool           can_whitelist      = true;
            bool           is_frozen          = false;
            bool           enforce_whitelist  = false;

            uint64_t primary_key()const { return supply.symbol.name(); }
         };

         typedef ultrainio::multi_index<N(accounts), account> accounts;
         typedef ultrainio::multi_index<N(stat), currency_stats> stats;

         void sub_balance( account_name owner, asset value, const currency_stats& st );
         void add_balance( account_name owner, asset value, const currency_stats& st, 
                           account_name ram_payer );
   };

} /// namespace ultrainio
