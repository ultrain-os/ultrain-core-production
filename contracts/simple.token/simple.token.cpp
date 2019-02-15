#include <ultrainiolib/ultrainio.hpp>

class simpletoken : public ultrainio::contract {
   public:
      simpletoken( account_name self )
      :contract(self),_accounts( _self, _self){}

      void transfer( account_name from, account_name to, uint64_t quantity ) {
         require_auth( from );

         const auto& fromacnt = _accounts.get( from );
         ultrainio_assert( fromacnt.balance >= quantity, "overdrawn balance" );
         _accounts.modify( fromacnt, [&]( auto& a ){ a.balance -= quantity; } );

         add_balance( to, quantity );
      }

      void issue( account_name to, uint64_t quantity ) {
         require_auth( _self );
         add_balance( to, quantity );
      }

   private:
      struct account {
         account_name owner;
         uint64_t     balance;

         uint64_t primary_key()const { return owner; }
      };

      ultrainio::multi_index<N(accounts), account> _accounts;

      void add_balance( account_name to, uint64_t q ) {
         auto toitr = _accounts.find( to );
         if( toitr == _accounts.end() ) {
           _accounts.emplace( [&]( auto& a ) {
              a.owner = to;
              a.balance = q;
           });
         } else {
           _accounts.modify( toitr, [&]( auto& a ) {
              a.balance += q;
              ultrainio_assert( a.balance >= q, "overflow detected" );
           });
         }
      }
};

ULTRAINIO_ABI( simpletoken, (transfer)(issue) )
