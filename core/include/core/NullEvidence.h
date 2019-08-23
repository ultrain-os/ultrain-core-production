#pragma once

#include "core/Evidence.h"

namespace ultrainio {
    class NullEvidence : public Evidence {
    public:
        virtual bool isNull() const;
    };
}