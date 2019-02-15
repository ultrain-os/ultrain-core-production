/**
 *  @file
 *  @copyright defined in ultrain/LICENSE.txt
 */

#include "ultrainio.token.hpp"

namespace ultrainio {
const int  interval_sec = 10;
const int  operatefee = 100;
void token::create( account_name issuer,
                    asset        maximum_supply )
{
    require_auth( _self );

    auto sym = maximum_supply.symbol;
    ultrainio_assert( sym.is_valid(), "invalid symbol name" );
    ultrainio_assert( maximum_supply.is_valid(), "invalid supply");
    ultrainio_assert( maximum_supply.amount > 0, "max-supply must be positive");

    stats statstable( _self, sym.name() );
    auto existing = statstable.find( sym.name() );
    ultrainio_assert( existing == statstable.end(), "token with symbol already exists" );

    statstable.emplace( [&]( auto& s ) {
       s.supply.symbol = maximum_supply.symbol;
       s.max_supply    = maximum_supply;
       s.issuer        = issuer;
    });
}


void token::issue( account_name to, asset quantity, string memo )
{
    auto sym = quantity.symbol;
    ultrainio_assert( sym.is_valid(), "invalid symbol name" );
    ultrainio_assert( memo.size() <= 256, "memo has more than 256 bytes" );

    auto sym_name = sym.name();
    stats statstable( _self, sym_name );
    auto existing = statstable.find( sym_name );
    ultrainio_assert( existing != statstable.end(), "token with symbol does not exist, create token before issue" );
    const auto& st = *existing;

    require_auth( st.issuer );
    ultrainio_assert( quantity.is_valid(), "invalid quantity" );
    ultrainio_assert( quantity.amount > 0, "must issue positive quantity" );

    ultrainio_assert( quantity.symbol == st.supply.symbol, "symbol precision mismatch" );
    ultrainio_assert( quantity.amount <= st.max_supply.amount - st.supply.amount, "quantity exceeds available supply");

    statstable.modify( st, [&]( auto& s ) {
       s.supply += quantity;
    });

    add_balance( st.issuer, quantity );

    if( to != st.issuer ) {
       SEND_INLINE_ACTION( *this, transfer, {st.issuer,N(active)}, {st.issuer, to, quantity, memo} );
    }
}

void token::transfer( account_name from,
                      account_name to,
                      asset        quantity,
                      string       memo )
{
    ultrainio_assert( from != to, "cannot transfer to self" );
    require_auth( from );
    ultrainio_assert( is_account( to ), "to account does not exist");
    auto sym = quantity.symbol.name();
    stats statstable( _self, sym );
    const auto& st = statstable.get( sym );

    require_recipient( from );
    require_recipient( to );

    ultrainio_assert( quantity.is_valid(), "invalid quantity" );
    ultrainio_assert( quantity.amount > 0, "must transfer positive quantity" );
    ultrainio_assert( quantity.symbol == st.supply.symbol, "symbol precision mismatch" );
    ultrainio_assert( memo.size() <= 256, "memo has more than 256 bytes" );


    sub_balance( from, quantity );
    add_balance( to, quantity );
}

void token::sub_balance( account_name owner, asset value ) {
   accounts from_acnts( _self, owner );

   const auto& from = from_acnts.get( value.symbol.name(), "no balance object found" );
   auto ct = now();
   asset fee(0);
   if((ct - from.last_time < interval_sec) && (owner != N(ultrainio))){
      if(value.amount >= 100000)
         fee = value/1000;
      else
         fee = asset(operatefee);
      ultrainio_assert( fee.amount > 0, "fee must a positive value" );
      add_balance( N(utrio.fee), fee );
      value += fee;
   }

   ultrainio_assert( from.balance.amount >= value.amount, "overdrawn balance" );

   if( from.balance.amount == value.amount ) {
      from_acnts.erase( from );
   } else {
      from_acnts.modify( from, [&]( auto& a ) {
          a.balance -= value;
          a.last_time = ct;
      });
   }
}

void token::add_balance( account_name owner, asset value )
{
   accounts to_acnts( _self, owner );
   auto to = to_acnts.find( value.symbol.name() );
   if( to == to_acnts.end() ) {
      to_acnts.emplace( [&]( auto& a ){
        a.balance = value;
        a.last_time = now();
      });
   } else {
      to_acnts.modify( to, [&]( auto& a ) {
        a.balance += value;
      });
   }
}

} /// namespace ultrainio

ULTRAINIO_ABI( ultrainio::token, (create)(issue)(transfer) )
