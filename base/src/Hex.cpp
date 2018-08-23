#include "base/Hex.h"

namespace ultrainio {
    uint8_t Hex::fromHex(char c) {
        if( c >= '0' && c <= '9' )
            return c - '0';
        if( c >= 'a' && c <= 'f' )
            return c - 'a' + 10;
        if( c >= 'A' && c <= 'F' )
            return c - 'A' + 10;
        return 0;
    }

    std::string Hex::toHex(const uint8_t* c, size_t len) {
        std::string r;
        const char* hexStr="0123456789abcdef";
        r.resize(len * 2);
        for (size_t i = 0; i < len; i++) {
            r[i*2] = hexStr[(c[i]>>4)];
            r[i*2 + 1] = hexStr[(c[i] & 0x0f)];
        }
        return r;
    }

    size_t Hex::fromHex( const std::string& hexStr, uint8_t* out, size_t len ) {
        std::string::const_iterator i = hexStr.begin();
        uint8_t* outPos = out;
        uint8_t* outEnd = outPos + len;
        while( i != hexStr.end() && outEnd != outPos ) {
            *outPos = fromHex( *i ) << 4;
            ++i;
            if( i != hexStr.end() )  {
                *outPos |= fromHex( *i );
                ++i;
            }
            ++outPos;
        }
        return outPos - out;
    }

}
