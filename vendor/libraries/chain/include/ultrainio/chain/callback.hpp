#pragma once

#include <stdint.h>
#include <string>

#include <ultrainio/chain/block_header.hpp>

namespace ultrainio {
    namespace chain {
        // interface
        class callback {
        public:
            virtual ~callback();

            virtual bool on_accept_block_header(uint64_t chain_name, const chain::block_header &,
                                                chain::block_id_type& id) = 0;

            virtual bool on_set_master_start_point(uint64_t chain_name, const std::string &committeeSetStr,
                                                   const chain::block_id_type &blockId) = 0;
        };
    }
} // namespace ultrainio::chain
