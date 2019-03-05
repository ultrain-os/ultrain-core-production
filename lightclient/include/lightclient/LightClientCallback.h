#pragma once

#include <list>

#include <core/types.h>

namespace ultrainio {
    class LightClientCallback {
    public:
        virtual ~LightClientCallback();
        virtual void onConfirmed(const std::list<BlockHeader>&) = 0;
    };
}