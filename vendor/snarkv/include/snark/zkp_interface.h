#pragma once
#include "snark/stransaction.h"

namespace libsnark {
    void init_pp_public_params();

    bool interface_verify(char* proof);

    bool verify_zero_knowledge_proof(char* vk, char* primary_input, char* proof);

    /// Sprout JoinSplit proof verification.
    /*bool sprout_verify(
        const unsigned char *proof,
        const unsigned char *rt,
        const unsigned char *h_sig,
        const unsigned char *mac1,
        const unsigned char *mac2,
        const unsigned char *nf1,
        const unsigned char *nf2,
        const unsigned char *cm1,
        const unsigned char *cm2,
        uint64_t vpub_old,
        uint64_t vpub_new
    );

    bool init_zk_params();
    bool check_shielded_transaction(const shielded_transaction& tx);*/
}// end of namespace
