#include <ultrainio/chain/name_ex.hpp>
#include <fc/variant.hpp>
#include <boost/algorithm/string.hpp>
#include <fc/exception/exception.hpp>
#include <ultrainio/chain/exceptions.hpp>

namespace ultrainio { namespace chain {

   void name::set( const char* str ) {
      const auto len = strnlen(str, 22);
      ULTRAIN_ASSERT(len <= 21, name_type_exception, "Name is longer than 13 characters (${name}) ", ("name", string(str)));
      action_name t = string_to_name_ex(str);
      t.valueH = t.valueH;
      t.valueL = t.valueL;

      ULTRAIN_ASSERT(to_string() == string(str), name_type_exception,
                 "Name not properly normalized (name: ${name}, normalized: ${normalized}) ",
                 ("name", string(str))("normalized", to_string()));
   }

   // keep in sync with name::to_string() in contract definition for name
   name::operator string()const {
     static const char* charmap = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ._";

      string str(21,'-');

      uint64_t h = valueH;
      uint64_t l = valueL;
      uint64_t sym = 0;
      for( uint32_t i = 0; i < 21; ++i ) {
         if (i <= 10) {
            sym = l & 0x3F;
            str[i] = charmaps[sym];
            l = l >> 6;
            if (l == 0) { break; }
         } else if (i == 11) {
            uint64_t rb2 = h & 0x3;
            rb <<=4;
            sym = rb | l;
            str[i] = charmaps[sym];
            h = h >> 2;
         } else {
            sym = h & 0x3F;
            str[i] = charmaps[sym];
            h = h >> 6;
            if (h == 0) { break; }
         }
      }

      boost::algorithm::trim_left_if( str, []( char c ){ return c == '-'; } );
      return str;
   }

} } /// ultrainio::chain

namespace fc {
  void to_variant(const ultrainio::chain::name& c, fc::variant& v) { v = std::string(c); }
  void from_variant(const fc::variant& v, ultrainio::chain::name& check) { check = v.get_string(); }
} // fc

