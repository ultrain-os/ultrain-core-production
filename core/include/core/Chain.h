/**
 *  @file
 *  @copyright defined in ultrain-core-for-open-source/LICENSE.txt
 */
#pragma once

#include <memory>

#include "Transaction.h"

namespace ultrainio {
    class Block;
    class BlockState;
    class PendingBlockState;
    class Transaction;
    class TransactionTrace;

    class Chain {
    public:
        static std::shared_ptr<Chain> getInstance();

        uint32_t headBlockNum();
        std::shared_ptr<Block> fetchBlockByNumber(uint32_t blockNum);
        void abortBlock();
        void startBlock();
        void pushBlock(const std::shared_ptr<SignedBlock> &block);
        std::shared_ptr<PendingBlockState> pendingBlockState();
        std::shared_ptr<BlockState> headBlockState();
        std::shared_ptr<BlockState> pendingBlockStateHack();
        void clearUnappliedTransaction();
        std::vector<Transaction> getUnappliedTransactions();
        std::list<std::shared_ptr<TransactionMetadata> >* getPendingTransactions();
        void setActionMerkleHack();
        void setTrxMerkleHack();
        std::shared_ptr<TransactionTrace> pushTransaction(const std::shared_ptr<TransactionMetadata>& trx);
        std::shared_ptr<TransactionTrace> pushTransaction(const std::shared_ptr<TransactionMetadata>& trx,
                ultrainio::time_point deadline, uint32_t cpuTimeUs = 0);
        std::shared_ptr<TransactionTrace> pushScheduledTransaction(const TransactionIdType& scheduled);
        std::shared_ptr<TransactionTrace> pushScheduledTransaction(const TransactionIdType& scheduled,
                ultrainio::time_point deadline, uint32_t cpuTimeUs = 0);
        void dropUnappliedTransaction(const std::shared_ptr<TransactionMetadata>& trx);
        void finalizeBlock();
        void commitBlock();
        bool isKnownUnexpiredTransaction(const TransactionIdIype& id);
    private:
        Chain(const Chain& rhs) = delete;
        Chain&operator = (const Chain& rhs) = delete;

        static std::shared_ptr<Chain> s_self;
    };
}