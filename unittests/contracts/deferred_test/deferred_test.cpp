/**
 *  @file
 *  @copyright defined in ultrain/LICENSE.txt
 */

#include <ultrainiolib/ultrainio.hpp>
#include <ultrainiolib/transaction.hpp>
#include <ultrainiolib/dispatcher.hpp>

using namespace ultrainio;

class deferred_test : public ultrainio::contract {
   public:
      using contract::contract;

      struct deferfunc_args {
         uint64_t payload;
      };

      //@abi action
      void defercall( account_name payer, uint64_t sender_id, account_name contract, uint64_t payload ) {
         print( "defercall called on ", name{_self}, "\n" );
         require_auth( payer );

         print( "deferred send of deferfunc action to ", name{contract}, " by ", name{payer}, " with sender id ", sender_id );
         transaction trx;
         deferfunc_args a = {.payload = payload};
         trx.actions.emplace_back(permission_level{_self, N(active)}, contract, NEX(deferfunc), a);
         trx.send( (static_cast<uint128_t>(payer) << 64) | sender_id, payer);
      }

      //@abi action
      void deferfunc( uint64_t payload ) {
         print("deferfunc called on ", name{_self}, " with payload = ", payload, "\n");
         ultrainio_assert( payload != 13, "value 13 not allowed in payload" );
      }

   private:
};

void apply_onerror(uint64_t receiver, const onerror& error ) {
   print("onerror called on ", name{receiver}, "\n");
}

extern "C" {
    /// The apply method implements the dispatch of events to this contract
    void apply( uint64_t receiver, uint64_t code, uint64_t actH, uint64_t actL) {
      name_ex action(actH, actL);
      if( code == N(ultrainio) && action == NEX(onerror) ) {
         apply_onerror( receiver, onerror::from_current_action() );
      } else if( code == receiver ) {
         deferred_test thiscontract(receiver);
         if( action == NEX(defercall) ) {
            execute_action( &thiscontract, &deferred_test::defercall );
         } else if( action == NEX(deferfunc) ) {
            execute_action( &thiscontract, &deferred_test::deferfunc );
         }
      }
   }
}

//ULTRAINIO_ABI( deferred_test, (defercall)(deferfunc) )
