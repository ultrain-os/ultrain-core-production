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
}