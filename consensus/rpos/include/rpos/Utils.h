#pragma once

#include <stddef.h>
#include <stdint.h>

#include <fc/crypto/sha256.hpp>

namespace ultrainio {
    class Utils {
    public:
        static uint32_t toInt(uint8_t* str, size_t len, int from);
    };

    std::string short_sig(const std::string& s);
    std::string short_hash(const fc::sha256& id);
}
