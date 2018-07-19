#pragma once
#include <string>
#include <fc/reflect/reflect.hpp>
#include <iosfwd>

namespace ultrainio { namespace chain {
   using std::string;

   struct name_ex {
      uint64_t valueH = 0;
      uint64_t valueL = 0;

      bool empty()const { return 0 == valueH && 0 == valueL; }
      bool good()const  { return !empty();   }

      name_ex( const char* str )   { set(str);           }
      name_ex( const string& str ) { set( str.c_str() ); }
      constexpr name_ex(uint64_t h, uint64_t l): valueH(h), valueL(l) {}

      void set( const char* str );

      template <typename T> name_ex(T v) : valueL(v) {
          // TODO (liangqin)
      }
      constexpr name_ex(): valueH(0), valueL(0) {}

      explicit operator string()const;

      string to_string() const { return string(*this); }

      name_ex& operator=( const string& n ) {
         name_ex t(n);
         valueH = t.valueH;
         valueL = t.valueL;

         return *this;
      }
      name_ex& operator=( const char* n ) {
         name_ex t(n);
         valueH = t.valueH;
         valueL = t.valueL;

         return *this;
      }

      friend std::ostream& operator << ( std::ostream& out, const name_ex& n ) {
         return out << string(n);
      }

      friend bool operator < ( const name_ex& a, const name_ex& b ) { return a.valueH < b.valueH || (a.valueH == b.valueH && a.valueL < b.valueL); }
      friend bool operator <= ( const name_ex& a, const name_ex& b ) { return a.valueH < b.valueH || (a.valueH == b.valueH && a.valueL <= b.valueL); }
      friend bool operator > ( const name_ex& a, const name_ex& b ) { return a.valueH > b.valueH || (a.valueH == b.valueH && a.valueL > b.valueL); }
      friend bool operator >=( const name_ex& a, const name_ex& b ) { return a.valueH > b.valueH || (a.valueH == b.valueH && a.valueL >= b.valueL); }
      friend bool operator == ( const name_ex& a, const name_ex& b ) { return a.valueH == b.valueH && a.valueL == b.valueL; }

      friend bool operator != ( const name_ex& a, const name_ex& b ) { return a.valueH != b.valueH || a.valueL != b.valueL; }
   };


   inline std::vector<name_ex> sort_names( std::vector<name_ex>&& names ) {
      fc::deduplicate(names);
      return names;
   }

   /**
    * Encode an ascii string to u128.
    * there are 64 characters as total, and the order as below:
    * [0-9A-Za-z._]
    */
   static constexpr uint64_t char_to_symbol_ex(char c) {
       if (c == '.')
           return 0;
       if (c == '_')
           return 1;

       if (c >= '0' && c <= '9')
           return (c - '0') + 2;

       if (c >= 'a' && c <= 'z')
           return (c - 'a') + 12;

       if (c >= 'A' && c <= 'Z')
           return (c - 'Z') + 38;

       return 0xFF; // ERROR
   }

   // Each char of the string is encoded into 6-bit chunk and left-shifted
   // to its 6-bit slot starting with the highest slot for the first char.
    constexpr name_ex string_to_name_ex(const char* str) {
        name_ex result;

        uint64_t name = 0;
        int i = 0;//strlen(str);
        while(str[i] != '\0') {
            uint64_t sym = char_to_symbol_ex(str[i]) & 0x3F;
            if (i <= 9) {
                name |= (sym << (6 * i));
            } else if (i == 10) {
                uint64_t rb4 = (sym & 0xF);
                name |= (rb4 << (6 * i));
                result.valueL = name;

                uint64_t lb2 = ((sym & 0x30) >> 4);
                name = lb2;
            } else {
                name |= (sym << (6* (i - 11) + 2));
            }

            i++;
        }
        if (i <= 10)
          result.valueL = name;
        else
          result.valueH = name;

        return result;
    }
    #define NEX(X) ultrainio::chain::string_to_name_ex(#X)
} } // ultrainio::chain

namespace std {
   template<> struct hash<ultrainio::chain::name_ex> : private hash<uint64_t> {
      typedef ultrainio::chain::name_ex argument_type;
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
  void to_variant(const ultrainio::chain::name_ex& c, fc::variant& v);
  void from_variant(const fc::variant& v, ultrainio::chain::name_ex& check);
} // fc


FC_REFLECT( ultrainio::chain::name_ex, (valueH)(valueL) )

