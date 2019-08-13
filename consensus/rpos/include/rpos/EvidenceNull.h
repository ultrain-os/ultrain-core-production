#pragma once

#include "rpos/Evidence.h"

namespace ultrainio {
    class EvidenceNull : public Evidence {
    public:
        virtual bool isNull() const;
    };
}