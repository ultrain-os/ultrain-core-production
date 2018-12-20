#pragma once

#include <stddef.h>
#include <stdint.h>

namespace ultrainio {
    class Utils {
    public:
        static uint32_t toInt(uint8_t* str, size_t len, int from);
    };
}