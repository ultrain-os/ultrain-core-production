#pragma once
#include <memory>
#include <string>

#include "rpos/Evidence.h"

namespace ultrainio {
    class EvidenceFactory {
    public:
        static std::shared_ptr<Evidence> create(const std::string& str);
    };
}
