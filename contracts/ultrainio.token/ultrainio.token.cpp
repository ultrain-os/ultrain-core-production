/**
 *  @file
 *  @copyright defined in ultrain/LICENSE.txt
 */

#include "ultrainio.token.hpp"

namespace ultrainio {
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


    sub_balance( from, quantity, st );
    add_balance( to, quantity );
}

void token::safe_transfer( account_name from,
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

    ultrainio_assert( quantity.is_valid(), "invalid quantity" );
    ultrainio_assert( quantity.amount > 0, "must transfer positive quantity" );
    ultrainio_assert( quantity.symbol == st.supply.symbol, "symbol precision mismatch" );
    ultrainio_assert( memo.size() <= 256, "memo has more than 256 bytes" );


    sub_balance( from, quantity, st );
    add_balance( to, quantity );
}

void token::set_chargeparams( std::string symbol, uint8_t precision, uint32_t operate_interval, uint32_t operate_fee)
{
    require_auth( _self );
    symbol_name sym = string_to_symbol(precision, symbol.c_str()) >> 8;
    stats statstable( _self, sym );
    auto stat = statstable.find( sym );
    ultrainio_assert( stat != statstable.end(), "symbol not exists" );
    statstable.modify( stat, [&]( currency_stats& s ) {
        s.operate_interval_sec = operate_interval;
        s.operate_fee = operate_fee;
    });
}

void token::sub_balance( account_name owner, asset value, const currency_stats& st ) {
   accounts from_acnts( _self, owner );

   const auto& from = from_acnts.get( value.symbol.name(), "there may be a charge for this account, but the balance is 0" );
   uint32_t cur_block_height= (uint32_t)head_block_number();
   if((cur_block_height - from.last_block_height < st.operate_interval_sec/block_interval_seconds())
      && (owner != N(ultrainio)) && name{owner}.to_string().find( "utrio." ) != 0){
      asset fee( st.operate_fee, value.symbol );
      int64_t p10 = (int64_t)value.symbol.precision();
      if(value.amount > p10*10)
         fee = value*10/p10;
      ultrainio_assert( fee.amount > 0, "fee must a positive value" );
      add_balance( N(utrio.fee), fee );
      value += fee;
   }

   ultrainio_assert( from.balance.amount >= value.amount, "there may be a charge for this account, but overdrawn balance" );

   if( from.balance.amount == value.amount ) {
      from_acnts.erase( from );
   } else {
      from_acnts.modify( from, [&]( auto& a ) {
          a.balance -= value;
          a.last_block_height = cur_block_height;
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
        a.last_block_height = (uint32_t)head_block_number();
      });
   } else {
      to_acnts.modify( to, [&]( auto& a ) {
        a.balance += value;
      });
   }
}

} /// namespace ultrainio

ULTRAINIO_ABI( ultrainio::token, (create)(issue)(transfer)(safe_transfer)(set_chargeparams) )
