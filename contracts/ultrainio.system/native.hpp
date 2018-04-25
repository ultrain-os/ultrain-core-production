/**
 *  @file
 *  @copyright defined in ultrain/LICENSE.txt
 */
#pragma once

#include <ultrainiolib/types.hpp>

namespace ultrainiosystem {

   typedef std::vector<char> bytes;
   typedef std::string type_name;
   typedef std::string field_name;

   struct permission_level_weight {
      permission_level  permission;
      weight_type       weight;

      ULTRAINLIB_SERIALIZE( permission_level_weight, (permission)(weight) )
   };

   struct key_weight {
      public_key   key;
      weight_type  weight;

      ULTRAINLIB_SERIALIZE( key_weight, (key)(weight) )
   };

   struct authority {
      uint32_t                              threshold;
      std::vector<key_weight>               keys;
      std::vector<permission_level_weight>  accounts;

      ULTRAINLIB_SERIALIZE( authority, (threshold)(keys)(accounts) )
   };

   template <account_name SystemAccount>
   class native {
      public:
         ACTION( SystemAccount, newaccount ) {
            account_name                     creator;
            account_name                     name;
            authority                        owner;
            authority                        active;
            authority                        recovery;

            ULTRAINLIB_SERIALIZE( newaccount, (creator)(name)(owner)(active)(recovery) )
         };

         static void on( const newaccount& ) {
         }

         ACTION( SystemAccount, updateauth ) {
            account_name                      account;
            permission_name                   permission;
            permission_name                   parent;
            authority                         data;

            ULTRAINLIB_SERIALIZE( updateauth, (account)(permission)(parent)(data) )
         };

         static void on( const updateauth& ) {
         }

         ACTION( SystemAccount, deleteauth ) {
            account_name                      account;
            permission_name                   permission;

            ULTRAINLIB_SERIALIZE( deleteauth, (account)(permission) )
         };

         static void on( const deleteauth& ) {
         }

         ACTION( SystemAccount, linkauth ) {
            account_name                      account;
            account_name                      code;
            action_name                       type;
            permission_name                   requirement;

            ULTRAINLIB_SERIALIZE( linkauth, (account)(code)(type)(requirement) )
         };

         static void on( const linkauth& ) {
         }

         ACTION( SystemAccount, unlinkauth ) {
            account_name                      account;
            account_name                      code;
            action_name                       type;

            ULTRAINLIB_SERIALIZE( unlinkauth, (account)(code)(type) )
         };

         static void on( const unlinkauth& ) {
         }

         ACTION( SystemAccount, postrecovery ) {
            account_name       account;
            authority          data;
            std::string        memo;

            ULTRAINLIB_SERIALIZE( postrecovery, (account)(data)(memo) )
         };

         static void on( const postrecovery& ) {
         }

         ACTION( SystemAccount, passrecovery ) {
            account_name   account;

            ULTRAINLIB_SERIALIZE( passrecovery, (account) )
         };

         static void on( const passrecovery& ) {
         }

         ACTION( SystemAccount, vetorecovery ) {
            account_name   account;

            ULTRAINLIB_SERIALIZE( vetorecovery, (account) )
         };

         static void on( const vetorecovery& ) {
         }

         struct onerror: ultrainio::action_meta<SystemAccount, N(onerror)>, bytes {
            ULTRAINLIB_SERIALIZE_DERIVED( onerror, bytes, BOOST_PP_SEQ_NIL )
         };

         static void on( const onerror& ) {
         }

         ACTION( SystemAccount, canceldelay ) {
            transaction_id_type   trx_id;

            ULTRAINLIB_SERIALIZE( canceldelay, (trx_id) )
         };

         static void on( const canceldelay& ) {
         }

   };
}
