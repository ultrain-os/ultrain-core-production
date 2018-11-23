#include "rpos/RoleRandom.h"

namespace ultrainio {
    RoleRandom::RoleRandom(const BlockIdType& r) : m_rand(r) {};

    uint32_t RoleRandom::toInt() {
        uint32_t rand = 0;
        size_t startIndex = 24; // 24 * 8 = 192
        size_t byteNum = 4;
        uint8_t* raw = (uint8_t*)m_rand.data();
        for (size_t i = 0; i < byteNum; i++) {
            rand += raw[startIndex + i];
            if (i != byteNum - 1) {
                rand = rand << 8;
            }
        }
        return rand;
    }
}