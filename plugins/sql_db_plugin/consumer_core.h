/**
 *  @file
 *  @copyright defined in ultrain/LICENSE.txt
 */

#pragma once

#include <vector>

namespace ultrainio {

template<typename T>
class consumer_core {
public:
    virtual ~consumer_core() {}
    virtual void consume(const std::vector<T>& elements) = 0;
};

} // namespace


