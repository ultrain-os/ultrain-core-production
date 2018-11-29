#pragma once

#include <cstdint>
#include <vector>

namespace ultrainio {

    class FisherYates {
    public:
        FisherYates(uint32_t rand, uint32_t size);

        std::vector<int> shuffle();

    private:
        int findNext(uint32_t random, uint32_t size);
        uint32_t m_rand;
        uint32_t m_size;
    };
}
