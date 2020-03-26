#pragma once

#include <stdint.h>
#include <string>

namespace ultrainio {
    class Hex {
    public:
        template <class T>
        static std::string toHex(const T* c, size_t len, bool lower = true) {
            std::string r;
            const char* hexStr="0123456789abcdef";
            const char* upperHexStr="0123456789ABCDEF";
            r.resize(len * 2);
            for (size_t i = 0; i < len; i++) {
                if (lower) {
                    r[i*2] = hexStr[(c[i]>>4)];
                    r[i*2 + 1] = hexStr[(c[i] & 0x0f)];
                } else {
                    r[i*2] = upperHexStr[(c[i]>>4)];
                    r[i*2 + 1] = upperHexStr[(c[i] & 0x0f)];
                }

            }
            return r;
        }

        template <class T>
        static size_t fromHex(const std::string& hexStr, T* out, size_t len) {
            std::string::const_iterator i = hexStr.begin();
            T* outPos = out;
            T* outEnd = outPos + len;
            while( i != hexStr.end() && outEnd != outPos ) {
                *outPos = Hex::fromHex<T>( *i ) << 4;
                ++i;
                if( i != hexStr.end() )  {
                    *outPos |= Hex::fromHex<T>( *i );
                    ++i;
                }
                ++outPos;
            }
            return outPos - out;
        }

    private:
        template <class T>
        static T fromHex(char c) {
            if( c >= '0' && c <= '9' )
                return c - '0';
            if( c >= 'a' && c <= 'f' )
                return c - 'a' + 10;
            if( c >= 'A' && c <= 'F' )
                return c - 'A' + 10;
            return 0;
        }
    };
}