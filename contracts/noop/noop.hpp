/**
 *  @file
 *  @copyright defined in ultrain/LICENSE.txt
 */
#pragma once

#include <ultrainiolib/ultrainio.hpp>
#include <ultrainiolib/dispatcher.hpp>

namespace noop {
   using std::string;


   /**
      noop contract
      All it does is require sender authorization.
      Actions: anyaction
    */
   class noop {
      public:
         
         ACTION(N(noop), anyaction) {
            anyaction() { }
            anyaction(account_name f, const string& t, const string& d): from(f), type(t), data(d) { }
            
            account_name from;
            string type;
            string data;
            
            ULTRAINLIB_SERIALIZE(anyaction, (from)(type)(data))
         };

         static void on(const anyaction& act)
         {
            require_auth(act.from);
         }
   };
} /// noop
