#pragma once

#include <memory>
#include <ultrainio/chain/block.hpp>
#include <ultrainio/chain/transaction.hpp>

namespace ultrainio {
    using TransactionHeader = chain::transaction_header;
    using Transaction = chain::transaction;
    using SignedTransaction = chain::signed_transaction;
    using PackedTransaction = chain::packed_transaction;
    using PackedTransactionPtr = std::shared_ptr<PackedTransaction>;
    using TransactionReceipt = chain::transaction_receipt;
    using ExtensionsType = chain::extensions_type;
    using BlockIdType = chain::block_id_type;
    using AccountName = chain::account_name;
    using SHA256 = fc::sha256;
    using Checksum256Type = chain::checksum256_type;
    using Block = chain::signed_block;
    using BlockHeader = chain::block_header;
    using SignedBlockHeader = chain::signed_block_header;
    using ChainIdType = chain::chain_id_type;
}