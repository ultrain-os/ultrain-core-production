#pragma once

#include <string>
#include "core/types.h"

namespace ultrainio {
    struct ExtType {
        uint32_t key;
        std::string value;
        bool operator == (const ExtType& rhs) const;
        bool operator != (const ExtType& rhs) const;
    };
}

FC_REFLECT( ultrainio::ExtType, (key)(value))
