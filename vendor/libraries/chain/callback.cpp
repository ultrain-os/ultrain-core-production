#include <ultrainio/chain/callback.hpp>

namespace ultrainio { namespace chain {
    callback::~callback() {}

    bool callback::on_accept_block_header(uint64_t chain_name, const chain::signed_block_header &,
            chain::block_id_type& id) {  return false; };

    bool callback::on_replay_block(const chain::block_header& header) { return false; }
} } // namespace ultrainio::chain