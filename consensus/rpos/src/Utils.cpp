#include "rpos/Utils.h"

#include <ultrainio/chain/exceptions.hpp>

#include <base/StringUtils.h>

namespace ultrainio {
    uint32_t Utils::toInt(uint8_t* str, size_t len, int from) {
        uint32_t rand = 0;
        ULTRAIN_ASSERT(str, chain::chain_exception, "nullptr str");
        ULTRAIN_ASSERT(from >= 0, chain::chain_exception, "from : ${from} out of range", ("from", from));
        ULTRAIN_ASSERT(len > from + sizeof(uint32_t), chain::chain_exception, "out of range");
        size_t byteNum = sizeof(uint32_t);
        uint8_t* raw = str;
        for (size_t i = 0; i < byteNum; i++) {
            rand += raw[from + i];
            if (i != byteNum - 1) {
                rand = rand << 8;
            }
        }
        return rand;
    }

    std::string short_sig(const std::string& s) {
        return StringUtils::shorten(s);
    }

    std::string short_hash(const fc::sha256& id) {
        return StringUtils::shorten(id.str());
    }
}
