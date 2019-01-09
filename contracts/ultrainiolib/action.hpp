/**
 *  @file
 *  @copyright defined in ultrain/LICENSE.txt
 */
#pragma once
#include <ultrainiolib/action.h>
#include <ultrainiolib/name_ex.hpp>

#include <ultrainiolib/datastream.hpp>
#include <ultrainiolib/serialize.hpp>

#include <boost/preprocessor/variadic/size.hpp>
#include <boost/preprocessor/variadic/to_tuple.hpp>
#include <boost/preprocessor/tuple/enum.hpp>
#include <boost/preprocessor/facilities/overload.hpp>

namespace ultrainio {

   /**
    * @defgroup actioncppapi Action C++ API
    * @ingroup actionapi
    * @brief Defines type-safe C++ wrapers for querying action and sending action
    *
    * @note There are some methods from the @ref actioncapi that can be used directly from C++
    *
    * @{
    */

   /**
    *
    *  This method unpacks the current action at type T.
    *
    *  @brief Interpret the action body as type T.
    *  @return Unpacked action data casted as T.
    *
    *  Example:
    *
    *  @code
    *  struct dummy_action {
    *    char a; //1
    *    unsigned long long b; //8
    *    int  c; //4
    *
    *    ULTRAINLIB_SERIALIZE( dummy_action, (a)(b)(c) )
    *  };
    *  dummy_action msg = unpack_action_data<dummy_action>();
    *  @endcode
    */
   template<typename T>
   T unpack_action_data() {
      constexpr size_t max_stack_buffer_size = 512;
      size_t size = action_data_size();
      char* buffer = (char*)( max_stack_buffer_size < size ? malloc(size) : alloca(size) );
      read_action_data( buffer, size );
      return unpack<T>( buffer, size );
   }

   using ::require_auth;
   using ::require_recipient;

   /**
    *  All of the listed accounts will be added to the set of accounts to be notified
    *
    *  This helper method enables you to add multiple accounts to accounts to be notified list with a single
    *  call rather than having to call the similar C API multiple times.
    *
    *  @note action.code is also considered as part of the set of notified accounts
    *
    *  @brief Notify an account for this action
    *  @param name account to be notified
    *  @param remaining_accounts accounts to be notified
    *
    *  Example:
    *
    *  @code
    *  require_recipient(N(Account1), N(Account2), N(Account3)); // throws exception if any of them not in set.
    *  @endcode
    */
   template<typename... accounts>
   void require_recipient( account_name name, accounts... remaining_accounts ){
      require_recipient( name );
      require_recipient( remaining_accounts... );
   }

   /**
    * Packed representation of a permission level (Authorization)
    *
    * @brief Packed representation of a permission level (Authorization)
    */
   struct permission_level {
      /**
       * Construct a new permission level object with actor name and permission name
       *
       * @brief Construct a new permission level object
       * @param a - Name of the account who owns this authorization
       * @param p - Name of the permission
       */
      permission_level( account_name a, permission_name p ):actor(a),permission(p){}

      /**
       * Default Constructor
       *
       * @brief Construct a new permission level object
       */
      permission_level(){}

      /**
       * Name of the account who owns this permission
       *
       * @brief Name of the account who owns this permission
       */
      account_name    actor;
      /**
       * Name of the permission
       *
       * @brief Name of the permission
       */
      permission_name permission;

      /**
       * Check equality of two permissions
       *
       * @brief Check equality of two permissions
       * @param a - first permission to compare
       * @param b - second permission to compare
       * @return true if equal
       * @return false if unequal
       */
      friend bool operator == ( const permission_level& a, const permission_level& b ) {
         return std::tie( a.actor, a.permission ) == std::tie( b.actor, b.permission );
      }

      ULTRAINLIB_SERIALIZE( permission_level, (actor)(permission) )
   };

   /**
    * proposeminer_info  vote
    *
    * @brief Packed representation of proposeminer_info  vote
    */
   struct proposeminer_info {
      /**
       * Construct a new proposeminer info object
       *
       * @brief Construct a new proposeminer info object
       * @param a - Name of the account who proposal pending miner
       * @param p - Name of the public keys
       * @param u - Name of the url
       */
      proposeminer_info( account_name a, std::string p, std::string u, uint16_t l,bool adddel ):account(a),public_key(p),url(u),location(l),adddel_miner(adddel){}

      /**
       * Default Constructor
       *
       * @brief Construct a new proposeminer info object
       */
      proposeminer_info(){}

      account_name      account;
      std::string       public_key;
      std::string       url;
      uint64_t          location;
      bool              adddel_miner;

      ULTRAINLIB_SERIALIZE( proposeminer_info, (account)(public_key)(url)(location)(adddel_miner) )
   };
   /**
    * provided_proposer  info
    *
    * @brief Packed representation of provided_proposer
    */
   struct provided_proposer {

      provided_proposer( account_name a, uint64_t l, uint64_t r):account(a),last_vote_time(l),resource_index(r){}

      /**
       * Default Constructor
       *
       * @brief Construct a new provided_proposer info object
       */
      provided_proposer(){}

      account_name      account;
      uint64_t          last_vote_time = 0;
      uint64_t          resource_index = 0;
      bool operator==(const provided_proposer& c) const
      {
         return c.account == account;
      }
      ULTRAINLIB_SERIALIZE( provided_proposer, (account)(last_vote_time)(resource_index) )
   };

   /**
    * proposeaccount_info  vote
    *
    * @brief Packed representation of proposeaccount_info  vote
    */
   struct proposeaccount_info {
      /**
       * Construct a new proposeaccount_info object
       *
       * @brief Construct a proposeaccount_info object
       * @param a - Name of the account who proposal pending miner
       * @param ok - Name of the owner keys
       * @param ak - Name of the active keys
       */
      proposeaccount_info( account_name a, std::string ok, std::string ak, uint16_t l ):account(a),owner_key(ok),active_key(ak),location(l){}

      /**
       * Default Constructor
       *
       * @brief Construct a new proposeaccount_info object
       */
      proposeaccount_info(){}

      account_name      account;
      std::string       owner_key;
      std::string       active_key;
      uint64_t          location;
      ULTRAINLIB_SERIALIZE( proposeaccount_info, (account)(owner_key)(active_key)(location) )
   };

   /**
    * proposeresource_info  vote
    *
    * @brief Packed representation of proposeresource_info  vote
    */
   struct proposeresource_info {
      /**
       * Construct a new proposeresource_info object
       *
       * @brief Construct a proposeresource_info object
       * @param a - Name of the account who proposal pending miner
       * @param ok - Name of the owner keys
       * @param ak - Name of the active keys
       */
      proposeresource_info( account_name a, uint64_t lease, uint64_t d, uint16_t l, int64_t num ):account(a),lease_num(lease),days(d),location(l),approve_num(num){}

      /**
       * Default Constructor
       *
       * @brief Construct a new proposeresource_info object
       */
      proposeresource_info(){}

      account_name      account;
      uint64_t          lease_num;
      uint64_t          days;
      uint64_t          location;
      int64_t           approve_num;
      ULTRAINLIB_SERIALIZE( proposeresource_info, (account)(lease_num)(days)(location)(approve_num) )
   };
   /**
    * Require the specified authorization for this action. If this action doesn't contain the specified auth, it will fail.
    *
    * @brief Require the specified authorization for this action
    *
    * @param level - Authorization to be required
    */
   void require_auth(const permission_level& level) {
      require_auth2( level.actor, level.permission );
   }

   /**
    * This is the packed representation of an action along with
    * meta-data about the authorization levels.
    *
    * @brief Packed representation of an action
    */
   struct action {
      /**
       * Name of the account the action is intended for
       *
       * @brief Name of the account the action is intended for
       */
      account_name               account;

      /**
       * Name of the action
       *
       * @brief Name of the action
       */
      action_name                name;

      /**
       * List of permissions that authorize this action
       *
       * @brief List of permissions that authorize this action
       */
      vector<permission_level>   authorization;

      /**
       * Payload data
       *
       * @brief Payload data
       */
      bytes                      data;

      /**
       * Default Constructor
       *
       * @brief Construct a new action object
       */
      action() = default;

      /**
       * Construct a new action object with the given permission and action struct
       *
       * @brief Construct a new action object with the given permission and action struct
       * @tparam Action  - Type of action struct
       * @param auth - The permission that authorizes this action
       * @param value - The action struct that will be serialized via pack into data
       */
      template<typename Action>
      action( vector<permission_level>&& auth, const Action& value ) {
         account       = Action::get_account();
         name          = Action::get_name();
         authorization = move(auth);
         data          = pack(value);
      }

      /**
       * Construct a new action object with the given list of permissions and action struct
       *
       * @brief Construct a new action object with the given list of permissions and action struct
       * @tparam Action  - Type of action struct
       * @param auth - The list of permissions that authorizes this action
       * @param value - The action struct that will be serialized via pack into data
       */
      template<typename Action>
      action( const permission_level& auth, const Action& value )
      :authorization(1,auth) {
         account       = Action::get_account();
         name          = Action::get_name();
         data          = pack(value);
      }


      /**
       * Construct a new action object with the given action struct
       *
       * @brief Construct a new action object with the given action struct
       * @tparam Action  - Type of action struct
       * @param value - The action struct that will be serialized via pack into data
       */
      template<typename Action>
      action( const Action& value ) {
         account       = Action::get_account();
         name          = Action::get_name();
         data          = pack(value);
      }

      /**
       * Construct a new action object with the given action struct
       *
       * @brief Construct a new action object with the given permission, action receiver, action name, action struct
       * @tparam T  - Type of action struct, must be serializable by `pack(...)`
       * @param auth - The permissions that authorizes this action
       * @param a -  The name of the account this action is intended for (action receiver)
       * @param n - The name of the action
       * @param value - The action struct that will be serialized via pack into data
       */
      template<typename T>
      action( const permission_level& auth, account_name a, action_name n, T&& value )
      :account(a), name(n), authorization(1,auth), data(pack(std::forward<T>(value))) {}

      /**
       * Construct a new action object with the given action struct
       *
       * @brief Construct a new action object with the given list of permissions, action receiver, action name, action struct
       * @tparam T  - Type of action struct, must be serializable by `pack(...)`
       * @param auths - The list of permissions that authorize this action
       * @param a -  The name of the account this action is intended for (action receiver)
       * @param n - The name of the action
       * @param value - The action struct that will be serialized via pack into data
       */
      template<typename T>
      action( vector<permission_level> auths, account_name a, action_name n, T&& value )
      :account(a), name(n), authorization(std::move(auths)), data(pack(std::forward<T>(value))) {}

      ULTRAINLIB_SERIALIZE( action, (account)(name)(authorization)(data) )

      /**
       * Send the action as inline action
       *
       * @brief Send the action as inline action
       */
      void send() const {
         auto serialize = pack(*this);
         ::send_inline(serialize.data(), serialize.size());
      }

      /**
       * Send the action as inline context free action
       *
       * @brief Send the action as inline context free action
       * @pre This action should not contain any authorizations
       */
      void send_context_free() const {
         ultrainio_assert( authorization.size() == 0, "context free actions cannot have authorizations");
         auto serialize = pack(*this);
         ::send_context_free_inline(serialize.data(), serialize.size());
      }

      /**
       * Retrieve the unpacked data as T
       *
       * @brief Retrieve the unpacked data as T
       * @tparam T expected type of data
       * @return the action data
       */
      template<typename T>
      T data_as() {
         ultrainio_assert( name == T::get_name(), "Invalid name" );
         ultrainio_assert( account == T::get_account(), "Invalid account" );
         return unpack<T>( &data[0], data.size() );
      }

   };

   /**
    * Base class to derive a new defined action from so it can take advantage of the dispatcher
    *
    * @brief Base class to derive a new defined action from
    * @tparam Account - The account this action is intended for
    * @tparam Name - The name of the action
    */
   template<uint64_t Account, uint64_t actH, uint64_t actL>
   struct action_meta {
      /**
       * Get the account this action is intended for
       *
       * @brief Get the account this action is intended for
       * @return uint64_t The account this action is intended for
       */
      static uint64_t get_account() { return Account; }
      /**
       * Get the name of this action
       *
       * @brief Get the name of this action
       * @return uint64_t Name of the action
       */
      static action_name get_name()  { return action_name(actH, actL); }
   };

   ///@} actioncpp api

   template<typename... Args>
   void dispatch_inline( account_name code, uint64_t actH, uint64_t actL,
                         vector<permission_level> perms,
                         std::tuple<Args...> args ) {
      action( perms, code, action_name(actH, actL), std::move(args) ).send();
   }


   template<typename, uint64_t, uint64_t>
   struct inline_dispatcher;

   template<typename T, uint64_t NameH, uint64_t NameL, typename... Args>
   struct inline_dispatcher<void(T::*)(Args...), NameH, NameL> {
      static void call(account_name code, const permission_level& perm, std::tuple<Args...> args) {
         dispatch_inline(code, NameH, NameL, vector<permission_level>(1, perm), std::move(args));
      }
      static void call(account_name code, vector<permission_level> perms, std::tuple<Args...> args) {
         dispatch_inline(code, NameH, NameL, std::move(perms), std::move(args));
      }
   };

 ///@} actioncpp api

} // namespace ultrainio

#define INLINE_ACTION_SENDER3( CONTRACT_CLASS, FUNCTION_NAME, ACTION_NAME  )\
::ultrainio::inline_dispatcher<decltype(&CONTRACT_CLASS::FUNCTION_NAME), (NEX(ACTION_NAME).valueH), (NEX(ACTION_NAME).valueL)>::call

#define INLINE_ACTION_SENDER2( CONTRACT_CLASS, NAME )\
INLINE_ACTION_SENDER3( CONTRACT_CLASS, NAME, NAME )

#define INLINE_ACTION_SENDER(...) BOOST_PP_OVERLOAD(INLINE_ACTION_SENDER,__VA_ARGS__)(__VA_ARGS__)

/**
 * @addtogroup actioncppapi
 * Additional documentation for group
 * @{
 */

/**
 * Send inline action
 *
 * @brief Send inline action
 * @param CONTRACT - The account this action is intended for
 * @param NAME - The name of the action
 * @param ... - The member of the action specified as ("action_member1_name", action_member1_value)("action_member2_name", action_member2_value)
 */
#define SEND_INLINE_ACTION( CONTRACT, NAME, ... )\
INLINE_ACTION_SENDER(std::decay_t<decltype(CONTRACT)>, NAME)( (CONTRACT).get_self(),\
BOOST_PP_TUPLE_ENUM(BOOST_PP_VARIADIC_SIZE(__VA_ARGS__), BOOST_PP_VARIADIC_TO_TUPLE(__VA_ARGS__)) );

/**
 * Extend a new defined action with theaction meta, so it can work with the dispatcher
 *
 * @brief Extend a new defined action with the action meta
 * @param CODE - The account this action is intended for
 * @param NAME - The name of the action
 */
#define ACTION( CODE, NAME ) struct NAME : ::ultrainio::action_meta<CODE, (NEX(NAME).valueH), (NEX(NAME).valueL)>

   /// @}
