#pragma once

#include <stdint.h>
#include <string>

namespace ultrainio {
    class Hex {
    public:
        static std::string toHex(const uint8_t* c, size_t len);
        static size_t fromHex(const std::string& hexStr, uint8_t* data, size_t len);

    private:
        static uint8_t fromHex(char c);
    };
}