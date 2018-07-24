/**
 *  @file
 *  @copyright defined in ultrain/LICENSE.txt
 */
#pragma once
#include <ultrainiolib/types.h>
#include <ultrainiolib/serialize.hpp>
#include <functional>
#include <tuple>
#include <string>

namespace ultrainio {
   static void trim_right_dots_ex(std::string& str ) {
        const auto last = str.find_last_not_of('.');
        if (last != std::string::npos)
            str = str.substr(0, last + 1);
   }

   struct name_ex {
        uint64_t valueH = 0;
        uint64_t valueL = 0;

        constexpr name_ex(): valueH(0), valueL(0) {
        }

        constexpr name_ex(uint64_t h, uint64_t l): valueH(h), valueL(l) {
        }

         ULTRAINLIB_SERIALIZE( name_ex, (valueH)(valueL))

        std::string to_string() const {
            static const char* charmaps =
            "._0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

            std::string str(21, '.');

            uint64_t h = valueH;
            uint64_t l = valueL;
            uint64_t sym = 0;
            for (uint32_t i = 0; i <= 20; ++i) {
                if (i <= 9) {
                    sym = (l & 0x3F);
                    str[i] = charmaps[sym];
                    l = (l >> 6);
                } else if (i == 10) {
                    uint64_t rb2 = (h & 0x3);
                    rb2 = (rb2 << 4);
                    sym = (rb2 | l);
                    str[i] = charmaps[sym];
                    h = (h >> 2);
                } else {
                    sym = (h & 0x3F);
                    str[i] = charmaps[sym];
                    h = (h >> 6);
                }
            }

            trim_right_dots_ex(str);
            return str;
        }

        friend bool operator==(const name_ex& rhs, const name_ex& lhs) { return rhs.valueH == lhs.valueH && rhs.valueL == lhs.valueL; }
        friend bool operator!=(const name_ex& rhs, const name_ex& lhs) { return rhs.valueH != lhs.valueH || rhs.valueL !=  lhs.valueL; }
   };

    static constexpr char char_to_symbol_ex( char c ) {
        if (c == '.')
           return 0;
        if (c == '_')
           return 1;

        if (c >= '0' && c <= '9')
           return (c - '0') + 2;

        if (c >= 'a' && c <= 'z')
           return (c - 'a') + 12;

        if (c >= 'A' && c <= 'Z')
           return (c - 'A') + 38;

        return 0; // ERROR
    }

    static constexpr name_ex string_to_name_ex( const char* str ) {

        name_ex result;

        uint64_t name = 0;
//        size_t len = strlen(str);
        size_t i = 0;
        while(str[i] != '\0') {
            uint64_t sym = (char_to_symbol_ex(str[i]) & 0x3F);
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

            ++i;
        }

        if (i <= 10)
          result.valueL = name;
        else
          result.valueH = name;

        return result;
   }

   #define NEX(X) ::ultrainio::string_to_name_ex(#X)


} // namespace ultrainio

using action_name = ::ultrainio::name_ex;

