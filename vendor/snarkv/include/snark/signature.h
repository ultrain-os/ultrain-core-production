#ifndef _SIGNATURE_H_
#define _SIGNATURE_H_

#include "snark/stransaction.h"
#include <vector>
#include <stdint.h>
#include <string>
#include <climits>

namespace  zero {
    class uint256;
};

/** Special case nIn for signing JoinSplits. */
const unsigned int NOT_AN_INPUT = UINT_MAX;

/** Signature hash types/flags */
enum
{
    SIGHASH_ALL = 1,
    SIGHASH_NONE = 2,
    SIGHASH_SINGLE = 3,
    SIGHASH_ANYONECANPAY = 0x80,
};


struct PrecomputedTransactionData
{
    zero::uint256 hashPrevouts;
    zero::uint256 hashSequence;
    zero::uint256 hashShieldedSpends;
    zero::uint256 hashShieldedOutputs;

    PrecomputedTransactionData(const shielded_transaction& tx);
};

zero::uint256 SignatureHash(
    const shielded_transaction& tx,
    unsigned int nIn,
    int nHashType,
    const CAmount& amount,
    const PrecomputedTransactionData* cache = NULL);

#endif // _SIGNATURE_H_
