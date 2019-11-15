#include "snark/signature.h"
#include "snark/stransaction.h"
#include "snark/ripemd160.h"
#include "snark/sha256.h"
#include "snark/pubkey.h"
#include "snark/uint256.h"

using namespace std;

static const unsigned char sc_prevouts_personalization[crypto_generichash_blake2b_PERSONALBYTES] =
    {'U','l','t','r','a','i','n','P','r','e','v','o','u','t'};
static const unsigned char sc_sequence_personalization[crypto_generichash_blake2b_PERSONALBYTES] =
    {'U','l','t','r','a','i','n','S','e','q','u','e','n','c'};
static const unsigned char sc_shielded_spends_personalization[crypto_generichash_blake2b_PERSONALBYTES] =
    {'U','l','t','r','a','i','n','S','S','p','e','n','d','s'};
static const unsigned char sc_shielded_outputs_personalization[crypto_generichash_blake2b_PERSONALBYTES] =
    {'U','l','t','r','a','i','n','S','O','u','t','p','u','t','s'};

zero::uint256 GetPrevoutHash(const shielded_transaction& txTo) {
    CBLAKE2bWriter ss(SER_GETHASH, 0, sc_prevouts_personalization);
    for (unsigned int n = 0; n < txTo.vin.size(); n++) {
        ss << txTo.vin[n].prevout;
    }
    return ss.GetHash();
}

zero::uint256 GetSequenceHash(const shielded_transaction& txTo) {
    CBLAKE2bWriter ss(SER_GETHASH, 0, sc_sequence_personalization);
    for (unsigned int n = 0; n < txTo.vin.size(); n++) {
        ss << txTo.vin[n].nSequence;
    }
    return ss.GetHash();
}

zero::uint256 GetShieldedSpendsHash(const shielded_transaction& txTo) {
    CBLAKE2bWriter ss(SER_GETHASH, 0, sc_shielded_spends_personalization);
    for (unsigned int n = 0; n < txTo.vShieldedSpend.size(); n++) {
        ss << txTo.vShieldedSpend[n].cv;
        ss << txTo.vShieldedSpend[n].anchor;
        ss << txTo.vShieldedSpend[n].nullifier;
        ss << txTo.vShieldedSpend[n].rk;
        ss << txTo.vShieldedSpend[n].zkproof;
    }
    return ss.GetHash();
}

zero::uint256 GetShieldedOutputsHash(const shielded_transaction& txTo) {
    CBLAKE2bWriter ss(SER_GETHASH, 0, sc_shielded_outputs_personalization);
    for (unsigned int n = 0; n < txTo.vShieldedOutput.size(); n++) {
        ss << txTo.vShieldedOutput[n];
    }
    return ss.GetHash();
}

PrecomputedTransactionData::PrecomputedTransactionData(const shielded_transaction& txTo)
{
    hashPrevouts = GetPrevoutHash(txTo);
    hashSequence = GetSequenceHash(txTo);
    hashShieldedSpends = GetShieldedSpendsHash(txTo);
    hashShieldedOutputs = GetShieldedOutputsHash(txTo);
}

zero::uint256 SignatureHash(
    const shielded_transaction& txTo,
    unsigned int nIn,
    int nHashType,
    const CAmount& amount,
    const PrecomputedTransactionData* cache)
{
    if (nIn >= txTo.vin.size() && nIn != NOT_AN_INPUT) {
        //  nIn out of range
        throw logic_error("input index is out of range");
    }

    zero::uint256 hashPrevouts;
    zero::uint256 hashSequence;
    zero::uint256 hashShieldedSpends;
    zero::uint256 hashShieldedOutputs;

    if (!(nHashType & SIGHASH_ANYONECANPAY)) {
        hashPrevouts = cache ? cache->hashPrevouts : GetPrevoutHash(txTo);
    }

    if (!(nHashType & SIGHASH_ANYONECANPAY) && (nHashType & 0x1f) != SIGHASH_SINGLE && (nHashType & 0x1f) != SIGHASH_NONE) {
        hashSequence = cache ? cache->hashSequence : GetSequenceHash(txTo);
    }

    if (!txTo.vShieldedSpend.empty()) {
        hashShieldedSpends = cache ? cache->hashShieldedSpends : GetShieldedSpendsHash(txTo);
    }

    if (!txTo.vShieldedOutput.empty()) {
        hashShieldedOutputs = cache ? cache->hashShieldedOutputs : GetShieldedOutputsHash(txTo);
    }

    unsigned char personalization[crypto_generichash_blake2b_PERSONALBYTES] = {};
    memcpy(personalization, "UltrainSigHash", 14);

    CBLAKE2bWriter ss(SER_GETHASH, 0, personalization);
    ss << hashPrevouts;
    ss << hashSequence;
    // Spend descriptions
    ss << hashShieldedSpends;
    // Output descriptions
    ss << hashShieldedOutputs;
    // Locktime
    //ss << txTo.nLockTime;
    // Expiry height
    //ss << txTo.nExpiryHeight;
    // Sapling value balance
    ss << txTo.valueBalance;
    return ss.GetHash();
}
