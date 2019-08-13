#pragma once

#include <string>

#include "rpos/Evidence.h"

namespace ultrainio {
    class EvidenceFactory {
    public:
        static Evidence create(const std::string& str);
    };
}
