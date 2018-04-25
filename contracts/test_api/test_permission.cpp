/**
 * @file action_test.cpp
 * @copyright defined in ultrain/LICENSE.txt
 */
#include <ultrainiolib/permission.h>
#include <ultrainiolib/db.h>

#include <ultrainiolib/ultrainio.hpp>
#include <ultrainiolib/print.hpp>
#include <ultrainiolib/compiler_builtins.h>
#include <ultrainiolib/serialize.hpp>
#include <ultrainiolib/action.hpp>

#include "test_api.hpp"

struct check_auth_msg {
   account_name         account;
   permission_name      permission;
   std::vector<public_key>   pubkeys;

   ULTRAINLIB_SERIALIZE( check_auth_msg, (account)(permission)(pubkeys)  )
};

void test_permission::check_authorization(uint64_t receiver, uint64_t code, uint64_t action) {
   (void)code;
   (void)action;
   using namespace ultrainio;

   auto self = receiver;
   auto params = unpack_action_data<check_auth_msg>();
   uint64_t res64 = (uint64_t)::check_authorization( params.account, params.permission, 
        (char*)params.pubkeys.data(), params.pubkeys.size()*sizeof(public_key) );

   auto itr = db_lowerbound_i64(self, self, self, 1);
   if(itr == -1) {
      db_store_i64(self, self, self, 1, &res64, sizeof(uint64_t));
   } else {
      db_update_i64(itr, self, &res64, sizeof(uint64_t));
   }
}
