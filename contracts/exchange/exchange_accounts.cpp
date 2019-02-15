#include <exchange/exchange_accounts.hpp>

namespace ultrainio {

   void exchange_accounts::adjust_balance( account_name owner, extended_asset delta, const string& reason ) {
      (void)reason;

      auto table = exaccounts_cache.find( owner );
      if( table == exaccounts_cache.end() ) {
         table = exaccounts_cache.emplace( owner, exaccounts(_this_contract, owner )  ).first;
      }
      auto useraccounts = table->second.find( owner );
      if( useraccounts == table->second.end() ) {
         table->second.emplace([&]( auto& exa ){
           exa.owner = owner;
           exa.balances[delta.get_extended_symbol()] = delta.amount;
           ultrainio_assert( delta.amount >= 0, "overdrawn balance 1" );
         });
      } else {
         table->second.modify( useraccounts, [&]( auto& exa ) {
           const auto& b = exa.balances[delta.get_extended_symbol()] += delta.amount;
           ultrainio_assert( b >= 0, "overdrawn balance 2" );
         });
      }
   }

} /// namespace ultrainio
