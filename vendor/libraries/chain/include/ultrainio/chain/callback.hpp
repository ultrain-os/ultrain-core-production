#pragma once

#include <stdint.h>
#include <string>

#include <ultrainio/chain/block_header.hpp>

namespace ultrainio {
    namespace chain {

        struct evildoer {
            account_name account;
            std::string commitee_pk;
            std::string bls_pk;
        };

        // interface
        class callback {
        public:
            virtual ~callback();

            virtual bool on_accept_block_header(uint64_t chain_name, const chain::signed_block_header &,
                                                chain::block_id_type& id);

            virtual bool on_replay_block(const chain::block_header& header);

            virtual int on_verify_evil(const std::string& evidence, const evildoer& evil);
        };
    }
} // namespace ultrainio::chain

FC_REFLECT( ultrainio::chain::evildoer, (account)(commitee_pk)(bls_pk) )
