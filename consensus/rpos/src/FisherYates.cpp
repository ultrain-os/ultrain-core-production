#include "rpos/FisherYates.h"

#include <algorithm>
#include <memory>

namespace ultrainio {
    FisherYates::FisherYates(uint32_t rand, uint32_t size) : m_rand(rand), m_size(size) {
    }

    std::vector<int> FisherYates::shuffle() {
        std::vector<int> v(m_size);
        for (int i = 0; i < m_size; i++) {
            v[i] = i;
        }
        for (int i = m_size - 1; i > 0; --i) {
            int next = findNext(m_rand, i + 1);
            std::swap(v[i], v[next]);
        }
        return v;
    }

    int FisherYates::findNext(uint32_t rand, uint32_t size) {
        return rand % size;
    }
}