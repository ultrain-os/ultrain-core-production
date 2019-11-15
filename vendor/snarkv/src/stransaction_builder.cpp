#include "snark/stransaction_builder.h"
//#include "pubkey.h"
//#include "rpc/protocol.h"
//#include "script/sign.h"
#include <boost/variant.hpp>
#include "snark/librustzcash.h"
#include "snark/hash.h"
#include "snark/signature.h"

SpendDescriptionInfo::SpendDescriptionInfo(
    libzcash::SaplingExpandedSpendingKey expsk,
    libzcash::SaplingNote note,
    uint256 anchor,
    uint64_t pos,
    const std::vector<unsigned char>& witness_path) : expsk(expsk), note(note), anchor(anchor), pos(pos), witness_path(witness_path)
    //SaplingWitness witness) : expsk(expsk), note(note), anchor(anchor), witness(witness)
{
    librustzcash_sapling_generate_r(alpha.begin());
}

/*
TransactionBuilderResult::TransactionBuilderResult(const CTransaction& tx) : maybeTx(tx) {}

TransactionBuilderResult::TransactionBuilderResult(const std::string& error) : maybeError(error) {}

bool TransactionBuilderResult::IsTx() { return maybeTx != boost::none; }

bool TransactionBuilderResult::IsError() { return maybeError != boost::none; }

CTransaction TransactionBuilderResult::GetTxOrThrow() {
    if (maybeTx) {
        return maybeTx.get();
    } else {
        std::cout << GetError() << std::endl;
        throw JSONRPCError(RPC_WALLET_ERROR, "Failed to build transaction: " + GetError());
    }
}

std::string TransactionBuilderResult::GetError() {
    if (maybeError) {
        return maybeError.get();
    } else {
        // This can only happen if isTx() is true in which case we should not call getError()
        throw std::runtime_error("getError() was called in TransactionBuilderResult, but the result was not initialized as an error.");
    }
}

TransactionBuilder::TransactionBuilder(
    const Consensus::Params& consensusParams,
    int nHeight,
    CKeyStore* keystore) : consensusParams(consensusParams), nHeight(nHeight), keystore(keystore)
{
    mtx = CreateNewContextualCMutableTransaction(consensusParams, nHeight);
}*/

void TransactionBuilder::AddSaplingSpend(
    libzcash::SaplingExpandedSpendingKey expsk,
    libzcash::SaplingNote note,
    uint256 anchor,
    uint64_t pos,
    const std::vector<unsigned char>& witness)
    //SaplingWitness witness)
{
    // Consistency check: all anchors must equal the first one
    if (spends.size() > 0 && spends[0].anchor != anchor) {
        std::cout << " &&&&&&&&&& anchor error" << std::endl;
        //throw JSONRPCError(RPC_WALLET_ERROR, "Anchor does not match previously-added Sapling spends.");
    }

    spends.emplace_back(expsk, note, anchor, pos, witness);
    strx.valueBalance += note.value();
}

void TransactionBuilder::AddSaplingOutput(
    uint256 ovk,
    libzcash::SaplingPaymentAddress to,
    CAmount value,
    std::array<unsigned char, ZC_MEMO_SIZE> memo)
{
    auto note = libzcash::SaplingNote(to, value);
    outputs.emplace_back(ovk, note, memo);
    strx.valueBalance -= value;
}

void TransactionBuilder::AddTransparentInput(COutPoint utxo, /*CScript scriptPubKey,*/ CAmount value)
{
    //strx.vin.emplace_back(utxo);
    strx.vin.emplace_back();
    strx.vin.back().prevout = utxo;
    strx.vin.back().nSequence = 0xffffffff;
    tIns.emplace_back(/*scriptPubKey, */value);
}

/*
void TransactionBuilder::AddTransparentOutput(CTxDestination& to, CAmount value)
{
    if (!IsValidDestination(to)) {
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid output address, not a valid taddr.");
    }

    CScript scriptPubKey = GetScriptForDestination(to);
    CTxOut out(value, scriptPubKey);
    strx.vout.push_back(out);
}*/

void TransactionBuilder::SetFee(CAmount fee)
{
    this->fee = fee;
}

void TransactionBuilder::SendChangeTo(libzcash::SaplingPaymentAddress changeAddr, uint256 ovk)
{
    zChangeAddr = std::make_pair(ovk, changeAddr);
    //tChangeAddr = boost::none;
}

/*
void TransactionBuilder::SendChangeTo(CTxDestination& changeAddr)
{
    if (!IsValidDestination(changeAddr)) {
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid change address, not a valid taddr.");
    }

    tChangeAddr = changeAddr;
    zChangeAddr = boost::none;
}*/

bool TransactionBuilder::Build()
{
    //
    // Consistency checks
    //

    // Valid change
    CAmount change = strx.valueBalance - fee;
    for (auto tIn : tIns) {
        change += tIn.value;
    }
    for (auto tOut : strx.vout) {
        change -= tOut.nValue;
    }
    if (change < 0) {
        elog("Change cannot be negative");
        return false;
    }

    ilog("change: ${c}", ("c", change));
    //
    // Change output
    //

    if (change > 0) {
        // Send change to the specified change address. If no change address
        // was set, send change to the first Sapling address given as input.
        if (zChangeAddr) {
            AddSaplingOutput(zChangeAddr->first, zChangeAddr->second, change);
        /*} else if (tChangeAddr) {
            // tChangeAddr has already been validated.
            AddTransparentOutput(tChangeAddr.value(), change);*/
        } else if (!spends.empty()) {
            auto fvk = spends[0].expsk.full_viewing_key();
            auto note = spends[0].note;
            libzcash::SaplingPaymentAddress changeAddr(note.d, note.pk_d);
            AddSaplingOutput(fvk.ovk, changeAddr, change);
        } else {
            elog("Could not determine change address");
            return false;
        }
    }

    //
    // Sapling spends and outputs
    //

    auto ctx = librustzcash_sapling_proving_ctx_init();

    // Create Sapling SpendDescriptions
    for (auto spend : spends) {
        auto cm = spend.note.cm();
        auto nf = spend.note.nullifier(
            spend.expsk.full_viewing_key(), spend.pos);
        if (!cm || !nf) {
            librustzcash_sapling_proving_ctx_free(ctx);
            elog("Spend is invalid");
            return false;
        }

        //CDataStream ss(SER_NETWORK, PROTOCOL_VERSION);
        //ss << spend.witness.path();
        //std::vector<unsigned char> witness(ss.begin(), ss.end());

        SpendDescription sdesc;
        if (!librustzcash_sapling_spend_proof(
                ctx,
                spend.expsk.full_viewing_key().ak.begin(),
                spend.expsk.nsk.begin(),
                spend.note.d.data(),
                spend.note.r.begin(),
                spend.alpha.begin(),
                spend.note.value(),
                spend.anchor.begin(),
                spend.witness_path.data(),
                sdesc.cv.begin(),
                sdesc.rk.begin(),
                sdesc.zkproof.data())) {
            librustzcash_sapling_proving_ctx_free(ctx);
            elog("Spend proof failed");
            return false;
        }

        sdesc.anchor = spend.anchor;
        sdesc.nullifier = *nf;
        strx.vShieldedSpend.push_back(sdesc);
    }

    // Create Sapling OutputDescriptions
    for (auto output : outputs) {
        auto cm = output.note.cm();
        if (!cm) {
            librustzcash_sapling_proving_ctx_free(ctx);
            elog("Output is invalid");
            return false;
        }

        libzcash::SaplingNotePlaintext notePlaintext(output.note, output.memo);

        auto res = notePlaintext.encrypt(output.note.pk_d);
        if (!res) {
            librustzcash_sapling_proving_ctx_free(ctx);
            elog("Failed to encrypt note");
            return false;
        }
        auto enc = res.get();
        auto encryptor = enc.second;

        OutputDescription odesc;
        if (!librustzcash_sapling_output_proof(
                ctx,
                encryptor.get_esk().begin(),
                output.note.d.data(),
                output.note.pk_d.begin(),
                output.note.r.begin(),
                output.note.value(),
                odesc.cv.begin(),
                odesc.zkproof.begin())) {
            librustzcash_sapling_proving_ctx_free(ctx);
            elog("Output proof failed");
            return false;
        }

        odesc.cm = *cm;
        odesc.ephemeralKey = encryptor.get_epk();
        odesc.encCiphertext = enc.first;

        libzcash::SaplingOutgoingPlaintext outPlaintext(output.note.pk_d, encryptor.get_esk());
        odesc.outCiphertext = outPlaintext.encrypt(
            output.ovk,
            odesc.cv,
            odesc.cm,
            encryptor);
        strx.vShieldedOutput.push_back(odesc);
    }

    //
    // Signatures
    //

    // Empty output script.
    uint256 dataToBeSigned;
    try {
        dataToBeSigned = SignatureHash(strx, NOT_AN_INPUT, SIGHASH_ALL, 0, nullptr);
    } catch (std::logic_error ex) {
        librustzcash_sapling_proving_ctx_free(ctx);
        elog("Could not construct signature hash: ${h}", ("h", std::string(ex.what())));
        return false;
    }

    // Create Sapling spendAuth and binding signatures
    for (size_t i = 0; i < spends.size(); i++) {
        librustzcash_sapling_spend_sig(
            spends[i].expsk.ask.begin(),
            spends[i].alpha.begin(),
            dataToBeSigned.begin(),
            strx.vShieldedSpend[i].spendAuthSig.data());
    }
    librustzcash_sapling_binding_sig(
        ctx,
        strx.valueBalance,
        dataToBeSigned.begin(),
        strx.bindingSig.data());

    librustzcash_sapling_proving_ctx_free(ctx);

    // Transparent signatures
   /* CTransaction txNewConst(mtx);
    for (int nIn = 0; nIn < mtx.vin.size(); nIn++) {
        auto tIn = tIns[nIn];
        SignatureData sigdata;
        bool signSuccess = ProduceSignature(
            TransactionSignatureCreator(
                keystore, &txNewConst, nIn, tIn.value, SIGHASH_ALL),
            tIn.scriptPubKey, sigdata, consensusBranchId);

        if (!signSuccess) {
            return TransactionBuilderResult("Failed to sign transaction");
        } else {
            UpdateTransaction(mtx, nIn, sigdata);
        }
    }

    return TransactionBuilderResult(CTransaction(mtx));*/
    return true;
}
