// Copyright (c) 2018 The Zcash developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef _STRANSACTION_BUILDER_H
#define _STRANSACTION_BUILDER_H

#include "uint256.h"
#include "address.h"
#include "incremental_merkle_tree.h"
#include "note.h"
#include "note_encryption.h"
#include "stransaction.h"
#include <boost/optional.hpp>

struct SpendDescriptionInfo {
    libzcash::SaplingExpandedSpendingKey expsk;
    libzcash::SaplingNote note;
    uint256 alpha;
    uint256 anchor;
    //SaplingWitness witness;
    uint64_t pos;
    std::vector<unsigned char> witness_path;

    SpendDescriptionInfo(
        libzcash::SaplingExpandedSpendingKey expsk,
        libzcash::SaplingNote note,
        uint256 anchor,
        uint64_t pos,
        const std::vector<unsigned char>& witness_path);
        //SaplingWitness witness);
};

struct OutputDescriptionInfo {
    uint256 ovk;
    libzcash::SaplingNote note;
    std::array<unsigned char, ZC_MEMO_SIZE> memo;

    OutputDescriptionInfo(
        uint256 ovk,
        libzcash::SaplingNote note,
        std::array<unsigned char, ZC_MEMO_SIZE> memo) : ovk(ovk), note(note), memo(memo) {}
};

struct TransparentInputInfo {
    //CScript scriptPubKey;
    int64_t value;

    TransparentInputInfo(
        /*CScript scriptPubKey,*/
        int64_t value) : /*scriptPubKey(scriptPubKey),*/ value(value) {}
};

class TransactionBuilder
{
private:
    shielded_transaction strx;
    int64_t fee = 0;

    std::vector<SpendDescriptionInfo> spends;
    std::vector<OutputDescriptionInfo> outputs;
    std::vector<TransparentInputInfo> tIns;

    boost::optional<std::pair<uint256, libzcash::SaplingPaymentAddress>> zChangeAddr;

public:
    TransactionBuilder() { strx.valueBalance = 0; }

    void SetFee(int64_t fee);

    // Throws if the anchor does not match the anchor used by
    // previously-added Sapling spends.
    void AddSaplingSpend(
        libzcash::SaplingExpandedSpendingKey expsk,
        libzcash::SaplingNote note,
        uint256 anchor,
        uint64_t pos,
        const std::vector<unsigned char>& witness);
        //SaplingWitness witness);

    void AddSaplingOutput(
        uint256 ovk,
        libzcash::SaplingPaymentAddress to,
        int64_t value,
        std::array<unsigned char, ZC_MEMO_SIZE> memo = {{0xF6}});

    // Assumes that the value correctly corresponds to the provided UTXO.
    void AddTransparentInput(COutPoint utxo, /*CScript scriptPubKey,*/ int64_t value);

    void SendChangeTo(libzcash::SaplingPaymentAddress changeAddr, uint256 ovk);

    bool Build();

    shielded_transaction GetStransaction() {return strx;};
};

#endif /* _STRANSACTION_BUILDER_H */
