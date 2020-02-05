/**
 *  @file
 *  @author fanliangqin@ultrain.io
 */
#pragma once
#include <ultrainiolib/crypto.h>
#include <ultrainiolib/datastream.hpp>
#include <ultrainiolib/asset.hpp>
#include <ultrainiolib/transaction.hpp>

#include <vector>
#include <string>

namespace ultrainio {
    struct stateful_transaction : public transaction {
        enum status_enum {
         executed  = 0, ///< succeed, no error handler executed
         soft_fail = 1, ///< objectively failed (not executed), error handler executed
         hard_fail = 2, ///< objectively failed and error handler objectively failed thus no state change
         delayed   = 3, ///< transaction delayed/deferred/scheduled for future execution
         expired   = 4  ///< transaction expired and storage space refuned to user
        };
        uint8_t status = soft_fail;
        std::string tx_id = "";

        ULTRAINLIB_SERIALIZE_DERIVED( stateful_transaction, transaction, (status)(tx_id) )
    };

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

        stateful_transaction recover_transaction() {
            void* buffer = alloca(max_stack_buffer_size);
            size_t size = max_stack_buffer_size;
            int ret = ts_recover_transaction(buffer, size, (const char*)(&tx_bytes[0]), tx_bytes.size());
            ultrainio_assert(ret != -1, "transaction_receipt only contains transaction id, no packed_transaction contains.");
            if (ret != 0) {
                size = (size_t)ret;
                buffer = malloc(size);
                ret = ts_recover_transaction(buffer, size, (const char*)(&tx_bytes[0]), tx_bytes.size());
                ultrainio_assert(ret == 0, "read packed_transaction raw data failed.");
            }

            stateful_transaction tx;
            datastream<char *> ds((char*) buffer, size);
            ds >> tx;

            return tx;
        }

        static merkle_proof get_merkle_proof(uint32_t block_number, std::string tx_id) {
            void* buffer = alloca(max_stack_buffer_size);
            size_t size = max_stack_buffer_size;
            int ret = ts_merkle_proof(block_number, tx_id.c_str(), buffer, max_stack_buffer_size);
            if (ret != 0) {
                size = (size_t)ret;
                buffer = malloc(size);
                ret = ts_merkle_proof(block_number, tx_id.c_str(), buffer, size);
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
