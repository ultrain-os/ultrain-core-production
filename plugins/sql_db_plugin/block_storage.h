/**
 *  @file
 *  @copyright defined in ultrain/LICENSE.txt
 */

#pragma once

#include "consumer_core.h"

#include <ultrainio/chain/block_state.hpp>

namespace ultrainio {

class block_storage : public consumer_core<chain::block_state_ptr>
{
public:
    void consume(const std::vector<chain::block_state_ptr>& blocks) override;
};

} // namespace
