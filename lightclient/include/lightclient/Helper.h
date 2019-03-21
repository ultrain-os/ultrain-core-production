#pragma once

#include <core/types.h>

namespace ultrainio {
    class Helper {
    public:
        static bool isGenesis(const BlockHeader& blockHeader);
    };
}