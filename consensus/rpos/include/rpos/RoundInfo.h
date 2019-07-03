#pragma once

#include <stdint.h>

namespace ultrainio {
    struct RoundInfo {
        uint32_t blockNum;
        uint16_t phase;

        RoundInfo(uint32_t b, uint16_t p) : blockNum(b), phase(p) {}

        RoundInfo() {}

        bool operator < (const RoundInfo& rhs) const {
            if (blockNum < rhs.blockNum) {
                return true;
            } else if (blockNum == rhs.blockNum) {
                return phase < rhs.phase;
            }
            return false;
        }
    };
}
