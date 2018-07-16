#pragma once
#include <string>
#include <fc/reflect/reflect.hpp>
#include <iosfwd>

namespace ultrainio { namespace chain {
   using std::string;

    /**
     * Encode an ascii string to u128.
     * there are 64 characters as total, and the order as below:
     * [0-9A-Za-z._]
     */
   static constexpr uint64_t char_to_symbol_ex( char c ) {
        if( c >= '0' && c <= '9' )
            return (c - '0');

        if( c >= 'A' && c <= 'Z' )
            return (c - 'A') + 10;

        if (c >= 'a' && c <= 'z')
            return (c - 'a') + 36;

        if (c == '.') return 62;
        if (c == '_') return 63;

        return 0xFF; // ERROR
   }

   // Each char of the string is encoded into 6-bit chunk and left-shifted
   // to its 6-bit slot starting with the highest slot for the first char.
   static constexpr action_name string_to_name_ex( const char* str )
   {
      action_name result;
      uint64_t name = 0;
      for (int i = 0; i < 21; i++) {
        uint64_t sym = char_to_symbol_ex(str[i]) & 0x3F;
        if (i == 11) {
            uint64_t rb4 = sym & 0xF;
            name = name << 4 | rb4;
            result.valueH = name;

            uint64_t lb2 = sym & 0x30 >> 4;
            name = lb2;
        } else {
            name = (name << 6) | sym;
        }
      }
      result.valueL = name;
      return result;
   }

#define NEX(X) ultrainio::chain::string_to_name_ex(#X)

   struct action_name {
      uint64_t valueH = 0;
      uint64_t valueL = 0;

      bool empty()const { return 0 == valueH && 0 == valueL; }
      bool good()const  { return !empty();   }

      action_name( const char* str )   { set(str);           }
      action_name( const string& str ) { set( str.c_str() ); }

      void set( const char* str );

      action_name(){}

      explicit operator string()const;

      string to_string() const { return string(*this); }

      action_name& operator=( const string& n ) {
         action_name t(n);
         valueH = t.valueH;
         valueL = t.valueL;

         return *this;
      }
      action_name& operator=( const char* n ) {
         action_name t(n);
         valueH = t.valueH;
         valueL = t.valueL;

         return *this;
      }

      friend std::ostream& operator << ( std::ostream& out, const action_name& n ) {
         return out << string(n);
      }

      friend bool operator < ( const action_name& a, const action_name& b ) { return a.valueH < b.valueH || (a.valueH == b.valueH && a.valueL < b.valueL); }
      friend bool operator <= ( const action_name& a, const action_name& b ) { return a.valueH < b.valueH || (a.valueH == b.valueH && a.valueL <= b.valueL); }
      friend bool operator > ( const action_name& a, const action_name& b ) { return a.value > b.value || (a.valueH == b.valueH && a.valueL > b.valueL); }
      friend bool operator >=( const action_name& a, const action_name& b ) { return a.valueH > b.valueH || (a.valueH == b.valueH && a.valueL >= b.valueL); }
      friend bool operator == ( const action_name& a, const action_name& b ) { return a.valueH == b.valueH && a.valueL == b.valueL; }

      friend bool operator != ( const action_name& a, const action_name& b ) { return a.valueH != b.valueH || a.valueL != b.valueL; }
   };


   inline std::vector<action_name> sort_names( std::vector<action_name>&& names ) {
      fc::deduplicate(names);
      return names;
   }

} } // ultrainio::chain

namespace std {
   template<> struct hash<ultrainio::chain::action_name> : private hash<uint64_t> {
      typedef ultrainio::chain::action_name argument_type;
      typedef typename hash<uint64_t>::result_type result_type;
      result_type operator()(const argument_type& name) const noexcept
      {
        result_type h = hash<uint64_t>::operator()(name.valueH);
        result_type l = hash<uint64_t>::operator()(name.valueL);
         return h^l;
      }
   };
};

namespace fc {
  class variant;
  void to_variant(const ultrainio::chain::action_name& c, fc::variant& v);
  void from_variant(const fc::variant& v, ultrainio::chain::action_name& check);
} // fc


FC_REFLECT( ultrainio::chain::action_name, (valueH)(valueL) )

