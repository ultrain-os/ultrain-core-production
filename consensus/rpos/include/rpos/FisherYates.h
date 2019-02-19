#pragma once

#include <cstdint>
#include <vector>
#include <core/types.h>

namespace ultrainio {

    class FisherYates {
    public:
        FisherYates(const fc::sha256& rand, uint32_t size);

        std::vector<int> shuffle();

    private:
        int findNext(const fc::sha256& rand, uint32_t size);
        fc::sha256 m_rand;
        uint32_t m_size;
    };
}
