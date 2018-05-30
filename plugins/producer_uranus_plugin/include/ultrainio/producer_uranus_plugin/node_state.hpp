#pragma once

namespace ultrainio {
    struct node_state {
        uranus_role role;
        consensus_phase phase;
        uint32_t block_id;
        std::string self_txs_hash; // not null if it is a proposer
        std::string self_proof;
        std::string min_txs_hash;
        std::string min_proof;
    };
} // namespace ultrainio
