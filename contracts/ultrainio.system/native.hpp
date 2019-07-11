/**
 *  @file
 *  @copyright defined in ultrain/LICENSE.txt
 */
#pragma once

#include <ultrainiolib/action.hpp>
#include <ultrainiolib/public_key.hpp>
#include <ultrainiolib/types.hpp>
#include <ultrainiolib/privileged.h>
#include <ultrainiolib/contract.hpp>

namespace ultrainiosystem {
   struct permission_level_weight {
      ultrainio::permission_level  permission;
      weight_type       weight;

      ULTRAINLIB_SERIALIZE( permission_level_weight, (permission)(weight) )
   };

   struct key_weight {
      ultrainio::public_key  key;
      weight_type        weight;

      ULTRAINLIB_SERIALIZE( key_weight, (key)(weight) )
   };

   struct wait_weight {
      uint32_t     wait_sec;
      weight_type  weight;

      ULTRAINLIB_SERIALIZE( wait_weight, (wait_sec)(weight) )
   };

   struct authority {
      uint32_t                              threshold = 0;
      std::vector<key_weight>               keys;
      std::vector<permission_level_weight>  accounts;
      std::vector<wait_weight>              waits;

      ULTRAINLIB_SERIALIZE( authority, (threshold)(keys)(accounts)(waits) )
   };

   struct action_actor_type {
      account_name   actor;
      action_name    action;

      ULTRAINLIB_SERIALIZE( action_actor_type, (actor)(action) )
   };

   class native : public ultrainio::contract {
      public:

         using ultrainio::contract::contract;

         void newaccount( account_name     creator,
                          account_name     newact
                          /*  no need to parse authorites
                          const authority& owner,
                          const authority& active*/ );


         void updateauth( account_name     account,
                                 permission_name  permission,
                                 permission_name  parent,
                                 const authority& data);

         void deleteauth( /*account_name account, permission_name permission*/ ) {}

         void linkauth( account_name    account,
                        account_name    code,
                        action_name     type,
                        permission_name requirement );

         void unlinkauth( /*account_name account,
                                 account_name code,
                                 action_name  type*/ ) {}

         void canceldelay( /*permission_level canceling_auth, transaction_id_type trx_id*/ ) {}

         void onerror( /*const bytes&*/ ) { require_auth( N(ultrainio) );}

         void deletetable( account_name code );

         void delaccount( account_name account );

         void addwhiteblack( /*std::vector<account_name>           actor_black;
                           std::vector<account_name>           actor_white;
                           std::vector<account_name>           contract_black;
                           std::vector<account_name>           contract_white;
                           std::vector<action_actor_type>      action_black;
                           std::vector<ultrainio::public_key>  key_black; */) { }

         void rmwhiteblack( /*std::vector<account_name>           actor_black;
                           std::vector<account_name>           actor_white;
                           std::vector<account_name>           contract_black;
                           std::vector<account_name>           contract_white;
                           std::vector<action_actor_type>      action_black;
                           std::vector<ultrainio::public_key>  key_black; */){ }
   };
}
