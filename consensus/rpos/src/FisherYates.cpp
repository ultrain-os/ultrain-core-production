#include "rpos/FisherYates.h"

#include <algorithm>
#include <memory>

#include <rpos/Utils.h>

namespace ultrainio {
    FisherYates::FisherYates(const fc::sha256& rand, uint32_t size) : m_rand(rand), m_size(size) {
    }

    std::vector<int> FisherYates::shuffle() {
        std::vector<int> v(m_size);
        for (int i = 0; i < m_size; i++) {
            v[i] = i;
        }
        fc::sha256 rand = m_rand;
        for (int i = 0; i < m_size - 1; i++) {
            int next = findNext(rand, m_size - i);
            next += i;
            std::swap(v[i], v[next]);
            rand = fc::sha256::hash(rand);
        }
        return v;
    }

    int FisherYates::findNext(const fc::sha256& rand, uint32_t size) {
        return Utils::toInt((uint8_t*)rand.data(), rand.data_size(), 24) % size;
    }
}