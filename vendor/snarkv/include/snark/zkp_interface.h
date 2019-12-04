#pragma once
#include "snark/stransaction.h"

namespace libsnark {

    // Groth16 verify proof algorithm
    // vk - verification key
    // primary_input - primary input
    // proof - proof
    bool verify_zero_knowledge_proof(char* vk, char* primary_input, char* proof);

    // Groth16 verify proof algorithm
    // data - string of format "vk,primary_input,proof"
    bool groth16_verify(char* data);

    // check shielded transaction
    bool check_shielded_transaction(const shielded_transaction& tx);
}// end of namespace
