/**
 *  @file
 *  @copyright defined in ultrain/LICENSE.txt
 */
#include <utility>
#include <vector>
#include <string>
#include <ultrainiolib/ultrainio.hpp>
#include <ultrainiolib/time.hpp>
#include <ultrainiolib/asset.hpp>
#include <ultrainiolib/contract.hpp>
#include <ultrainiolib/crypto.h>

using ultrainio::key256;
using ultrainio::indexed_by;
using ultrainio::const_mem_fun;
using ultrainio::asset;
using ultrainio::permission_level;
using ultrainio::action;
using ultrainio::print;
using ultrainio::name;

class economic : public ultrainio::contract {
   public:
      economic(account_name self)
      :ultrainio::contract(self),
       accounts(_self, _self)
      {}

      //@abi action
      void test1(const account_name player) {
         require_auth( player );
         offer_index       offers(_self,player);
         offers.emplace(_self, [&](auto& offer){
            offer.id         = offers.available_primary_key();
            offer.bet        = asset(1);
            offer.owner      = player;
            offer.gameid     = 0;
         });

      }

      //@abi action
      void test2( const account_name from, const asset& quantity ) {

         ultrainio_assert( quantity.is_valid(), "invalid quantity" );
         ultrainio_assert( quantity.amount > 0, "quanity must positive quantity" );

         auto itr = accounts.find(from);
         if( itr == accounts.end() ) {
            itr = accounts.emplace(_self, [&](auto& acnt){
               acnt.owner = from;
               acnt.ultrain_balance = quantity;
            });
         }
      }

   private:
      //@abi table offer i64
      struct offer {
         uint64_t          id;
         account_name      owner;
         asset             bet;
         checksum256       commitment;
         uint64_t          gameid = 0;

         uint64_t primary_key()const { return id; }

         uint64_t by_bet()const { return (uint64_t)bet.amount; }

         key256 by_commitment()const { return get_commitment(commitment); }

         static key256 get_commitment(const checksum256& commitment) {
            const uint64_t *p64 = reinterpret_cast<const uint64_t *>(&commitment);
            return key256::make_from_word_sequence<uint64_t>(p64[0], p64[1], p64[2], p64[3]);
         }

         ULTRAINLIB_SERIALIZE( offer, (id)(owner)(bet)(commitment)(gameid) )
      };

      typedef ultrainio::multi_index< N(offer), offer,
         indexed_by< N(bet), const_mem_fun<offer, uint64_t, &offer::by_bet > >,
         indexed_by< N(commitment), const_mem_fun<offer, key256,  &offer::by_commitment> >
      > offer_index;

      //@abi table account i64
      struct account {
         account( account_name o = account_name() ):owner(o){}

         account_name owner;
         asset        ultrain_balance;
         uint32_t     open_offers = 0;
         uint32_t     open_games = 0;

         bool is_empty()const { return !( ultrain_balance.amount | open_offers | open_games ); }

         uint64_t primary_key()const { return owner; }

         ULTRAINLIB_SERIALIZE( account, (owner)(ultrain_balance)(open_offers)(open_games) )
      };
      typedef ultrainio::multi_index< N(account), account> account_index;
      account_index     accounts;
};

ULTRAINIO_ABI( economic, (test1)(test2) )
