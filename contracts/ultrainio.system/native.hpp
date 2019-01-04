/**
 *  @file
 *  @copyright defined in ultrain/LICENSE.txt
 */
#pragma once

#include <ultrainiolib/action.hpp>
#include <ultrainiolib/public_key.hpp>
#include <ultrainiolib/types.hpp>
#include <ultrainiolib/print.hpp>
#include <ultrainiolib/privileged.h>
#include <ultrainiolib/optional.hpp>
#include <ultrainiolib/producer_schedule.hpp>
#include <ultrainiolib/contract.hpp>

namespace ultrainiosystem {
   using ultrainio::permission_level;
   using ultrainio::public_key;

   typedef std::vector<char> bytes;

   struct permission_level_weight {
      permission_level  permission;
      weight_type       weight;

      // explicit serialization macro is not necessary, used here only to improve compilation time
      ULTRAINLIB_SERIALIZE( permission_level_weight, (permission)(weight) )
   };

   struct key_weight {
      ultrainio::public_key  key;
      weight_type        weight;

      // explicit serialization macro is not necessary, used here only to improve compilation time
      ULTRAINLIB_SERIALIZE( key_weight, (key)(weight) )
   };
   struct wait_weight {
      uint32_t     wait_sec;
      weight_type  weight;
       // explicit serialization macro is not necessary, used here only to improve compilation time
      ULTRAINLIB_SERIALIZE( wait_weight, (wait_sec)(weight) )
   };
   struct authority {
      uint32_t                              threshold = 0;
      std::vector<key_weight>               keys;
      std::vector<permission_level_weight>  accounts;
      std::vector<wait_weight>              waits;
      // explicit serialization macro is not necessary, used here only to improve compilation time
      ULTRAINLIB_SERIALIZE( authority, (threshold)(keys)(accounts)(waits) )
   };

   /*
    * Method parameters commented out to prevent generation of code that parses input data.
    */
   class native : public ultrainio::contract {
      public:

         using ultrainio::contract::contract;

         /**
          *  Called after a new account is created. This code enforces resource-limits rules
          *  for new accounts as well as new account naming conventions.
          *
          *  1. accounts cannot contain '.' symbols which forces all acccounts to be 12
          *  characters long without '.' until a future account auction process is implemented
          *  which prevents name squatting.
          *
          *  2. new accounts must stake a minimal number of tokens (as set in system parameters)
          *     therefore, this method will execute an inline buyram from receiver for newacnt in
          *     an amount equal to the current new account creation fee.
          */
         void newaccount( account_name     creator,
                          account_name     newact
                          /*  no need to parse authorites
                          const authority& owner,
                          const authority& active*/ );


         void updateauth( /*account_name     account,
                                 permission_name  permission,
                                 permission_name  parent,
                                 const authority& data*/ ) {}

         void deleteauth( /*account_name account, permission_name permission*/ ) {}

         void linkauth( /*account_name    account,
                               account_name    code,
                               action_name     type,
                               permission_name requirement*/ ) {}

         void unlinkauth( /*account_name account,
                                 account_name code,
                                 action_name  type*/ ) {}

         void canceldelay( /*permission_level canceling_auth, transaction_id_type trx_id*/ ) {}

         void onerror( /*const bytes&*/ ) {}

         void deletetable( account_name code );
   };
}
