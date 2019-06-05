/**
 *  @file
 *  @copyright defined in ultrain/LICENSE.txt
 */
#pragma once

#include <ultrainio/chain/types.hpp>
#include <ultrainio/chain/exceptions.hpp>

namespace ultrainio { namespace chain {

   struct permission_level {
      account_name    actor;
      permission_name permission;
   };

   struct block_reward {
       uint16_t         consensus_period;
       uint64_t         reward;
   };

   inline bool operator== (const permission_level& lhs, const permission_level& rhs) {
      return std::tie(lhs.actor, lhs.permission) == std::tie(rhs.actor, rhs.permission);
   }

   inline bool operator!= (const permission_level& lhs, const permission_level& rhs) {
      return std::tie(lhs.actor, lhs.permission) != std::tie(rhs.actor, rhs.permission);
   }

   inline bool operator< (const permission_level& lhs, const permission_level& rhs) {
      return std::tie(lhs.actor, lhs.permission) < std::tie(rhs.actor, rhs.permission);
   }

   inline bool operator<= (const permission_level& lhs, const permission_level& rhs) {
      return std::tie(lhs.actor, lhs.permission) <= std::tie(rhs.actor, rhs.permission);
   }

   inline bool operator> (const permission_level& lhs, const permission_level& rhs) {
      return std::tie(lhs.actor, lhs.permission) > std::tie(rhs.actor, rhs.permission);
   }

   inline bool operator>= (const permission_level& lhs, const permission_level& rhs) {
      return std::tie(lhs.actor, lhs.permission) >= std::tie(rhs.actor, rhs.permission);
   }

   /**
    *  An action is performed by an actor, aka an account. It may
    *  be created explicitly and authorized by signatures or might be
    *  generated implicitly by executing application code.
    *
    *  This follows the design pattern of React Flux where actions are
    *  named and then dispatched to one or more action handlers (aka stores).
    *  In the context of ultrainio, every action is dispatched to the handler defined
    *  by account 'scope' and function 'name', but the default handler may also
    *  forward the action to any number of additional handlers. Any application
    *  can write a handler for "scope::name" that will get executed if and only if
    *  this action is forwarded to that application.
    *
    *  Each action may require the permission of specific actors. Actors can define
    *  any number of permission levels. The actors and their respective permission
    *  levels are declared on the action and validated independently of the executing
    *  application code. An application code will check to see if the required authorization
    *  were properly declared when it executes.
    */
   struct action {
      enum AbilityType { Normal = 0, PureView = 1 };
      account_name                           account;
      action_name                            name;
      vector<permission_level>               authorization;
      bytes                                  data;
      fc::enum_type<uint8_t, AbilityType>    ability;

      action(): ability(Normal) {}

      template<typename T, std::enable_if_t<std::is_base_of<bytes, T>::value, int> = 1>
      action( vector<permission_level> auth, const T& value ) {
         account     = T::get_account();
         name        = T::get_name();
         authorization = move(auth);
         data.assign(value.data(), value.data() + value.size());
         ability     = AbilityType::Normal;
      }

      template<typename T, std::enable_if_t<!std::is_base_of<bytes, T>::value, int> = 1>
      action( vector<permission_level> auth, const T& value ) {
         account     = T::get_account();
         name        = T::get_name();
         authorization = move(auth);
         data        = fc::raw::pack(value);
         ability     = AbilityType::Normal;
      }

      action( vector<permission_level> auth, account_name account, action_name name, const bytes& data, action::AbilityType ablty = action::Normal )
            : account(account), name(name), authorization(move(auth)), data(data), ability(ablty) {
      }

      template<typename T>
      T data_as()const {
         ULTRAIN_ASSERT( account == T::get_account(), action_type_exception, "account is not consistent with action struct" );
         ULTRAIN_ASSERT( name == T::get_name(), action_type_exception, "action name is not consistent with action struct"  );
         return fc::raw::unpack<T>(data);
      }
   };

   struct action_notice : public action {
      account_name receiver;
   };

} } /// namespace ultrainio::chain

FC_REFLECT_ENUM( ultrainio::chain::action::AbilityType, (Normal)(PureView) )
FC_REFLECT( ultrainio::chain::permission_level, (actor)(permission) )
FC_REFLECT( ultrainio::chain::block_reward , (consensus_period)(reward) )
FC_REFLECT( ultrainio::chain::action, (account)(name)(authorization)(data) )

