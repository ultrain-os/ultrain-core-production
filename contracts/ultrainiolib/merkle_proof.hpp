/**
 *  @file
 *  @author fanliangqin@ultrain.io
 */
#pragma once
#include <ultrainiolib/crypto.h>
#include <ultrainiolib/datastream.hpp>
#include <ultrainiolib/asset.hpp>

#include <vector>
#include <string>

namespace ultrainio {
    struct merkle_proof {
        constexpr static size_t max_stack_buffer_size = 512;

        std::vector<std::string> proofs;
        std::vector<char> tx_bytes;

        bool verify(std::string transaction_mroot) {
            size_t proofs_size = pack_size(proofs);
            void* proofs_buffer = max_stack_buffer_size < proofs_size ? malloc(proofs_size) : alloca(proofs_size);
            datastream<char*> pbds((char*)proofs_buffer, proofs_size);
            pbds << proofs;

            int r = ts_verify_merkle_proof(transaction_mroot.c_str(), (const char*)proofs_buffer, proofs_size, (const char*)(&tx_bytes[0]), tx_bytes.size());
            return r != 0;
        }

        transaction recover_transaction() {
            void* buffer = alloca(max_stack_buffer_size);
            size_t size = max_stack_buffer_size;
            size_t ret = ts_recover_transaction(buffer, size, (const char*)(&tx_bytes[0]), tx_bytes.size());
            ultrainio_assert(ret != -1, "transaction_receipt only contains transaction id, no packed_transaction contains.");
            if (ret != 0) {
                buffer = malloc(ret);
                size = ret;
                ret = ts_recover_transaction(buffer, size, (const char*)(&tx_bytes[0]), tx_bytes.size());
                ultrainio_assert(ret == 0, "read packed_transaction raw data failed.");
            }

            transaction tx;
            datastream<char *> ds((char*) buffer, size);
            ds >> tx;

            return tx;
        }

        static merkle_proof get_merkle_proof(uint32_t block_number, std::string tx_id) {
            void* buffer = alloca(max_stack_buffer_size);
            size_t size = max_stack_buffer_size;
            size_t ret = ts_merkle_proof(block_number, tx_id.c_str(), buffer, max_stack_buffer_size);
            if (ret != 0) {
                buffer = malloc(ret);
                size = ret;
                ret = ts_merkle_proof(block_number, tx_id.c_str(), buffer, ret);
                ultrainio_assert(ret == 0, "read merkle proof failed.");
            }
            merkle_proof mklp;
            datastream<char*> ds( (char*)buffer, size );
            ds >> mklp.proofs;
            ds >> mklp.tx_bytes;

            return mklp;
        }
    };
}
