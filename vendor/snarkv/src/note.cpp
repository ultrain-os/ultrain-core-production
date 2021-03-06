#include "snark/note.h"
#include "snark/prf.h"
#include "snark/sha256.h"

#include "snark/random.h"
#include <fc/log/logger.hpp>
#include "snark/util.h"
#include "snark/librustzcash.h"

using namespace libzcash;

// Construct and populate Sapling note for a given payment address and value.
SaplingNote::SaplingNote(const SaplingPaymentAddress& address, const uint64_t value) : BaseNote(value) {
    d = address.d;
    pk_d = address.pk_d;
    librustzcash_sapling_generate_r(r.begin());
}

// Call librustzcash to compute the commitment
boost::optional<uint256> SaplingNote::cm() const {
    uint256 result;
    if (!librustzcash_sapling_compute_cm(
            d.data(),
            pk_d.begin(),
            value(),
            r.begin(),
            result.begin()
        ))
    {
        return boost::none;
    }

    return result;
}

// Call librustzcash to compute the nullifier
boost::optional<uint256> SaplingNote::nullifier(const SaplingFullViewingKey& vk, const uint64_t position) const
{
    auto ak = vk.ak;
    auto nk = vk.nk;

    uint256 result;
    if (!librustzcash_sapling_compute_nf(
            d.data(),
            pk_d.begin(),
            value(),
            r.begin(),
            ak.begin(),
            nk.begin(),
            position,
            result.begin()
    ))
    {
        return boost::none;
    }

    return result;
}


// Construct and populate SaplingNotePlaintext for a given note and memo.
SaplingNotePlaintext::SaplingNotePlaintext(
    const SaplingNote& note,
    std::array<unsigned char, ZC_MEMO_SIZE> memo) : BaseNotePlaintext(note, memo)
{
    d = note.d;
    rcm = note.r;
}


boost::optional<SaplingNote> SaplingNotePlaintext::note(const SaplingIncomingViewingKey& ivk) const
{
    auto addr = ivk.address(d);
    if (addr) {
        return SaplingNote(d, addr.get().pk_d, value_, rcm);
    } else {
        return boost::none;
    }
}

boost::optional<SaplingOutgoingPlaintext> SaplingOutgoingPlaintext::decrypt(
    const SaplingOutCiphertext &ciphertext,
    const uint256& ovk,
    const uint256& cv,
    const uint256& cm,
    const uint256& epk
)
{
    auto pt = AttemptSaplingOutDecryption(ciphertext, ovk, cv, cm, epk);
    if (!pt) {
        elog("decrypt SaplingOutCiphertext failed");
        return boost::none;
    }

    if (pt->size() != 64) {
        return boost::none;
    }

    SaplingOutgoingPlaintext ret;
    std::vector<unsigned char> vch;
    for (int i = 0; i < 32; i++) {
        vch.push_back((*pt)[i]);
    }
    ret.pk_d = uint256(vch);

    vch.clear();
    for (int i = 32; i < 64; i++) {
       vch.push_back((*pt)[i]);
    }
    ret.esk = uint256(vch); 
    /*
    // Deserialize from the plaintext
    CDataStream ss(SER_NETWORK, PROTOCOL_VERSION);
    ss << pt.get();

    SaplingOutgoingPlaintext ret;
    ss >> ret;

    assert(ss.size() == 0);
    */

    return ret;
}

boost::optional<SaplingNotePlaintext> SaplingNotePlaintext::decrypt(
    const SaplingEncCiphertext &ciphertext,
    const uint256 &ivk,
    const uint256 &epk,
    const uint256 &cmu
)
{
    auto pt = AttemptSaplingEncDecryption(ciphertext, ivk, epk);
    if (!pt) {
        elog("decrypt SaplingEncCiphertext failed");
        return boost::none;
    }

    if (pt->size() != 1 + ZC_DIVERSIFIER_SIZE + sizeof(uint64_t) + 32 + ZC_MEMO_SIZE || (*pt)[0] != 0x01) {
        elog("lead byte of SaplingNotePlaintext is not recognized");
        return boost::none;
    }

    SaplingNotePlaintext ret;
    for (int i = 1; i < ZC_DIVERSIFIER_SIZE+1; i++) {
        ret.d[i-1] = (*pt)[i];
    }

    memcpy(&ret.value_, pt->data()+ZC_DIVERSIFIER_SIZE+1, sizeof(uint64_t));

    std::vector<unsigned char> vch;
    int start = ZC_DIVERSIFIER_SIZE+1+sizeof(uint64_t);
    for (int i = start; i < start+32; i++) {
        vch.push_back((*pt)[i]);
    }
    ret.rcm = uint256(vch);

    memcpy(ret.memo_.data(), pt->data()+start+32, ZC_MEMO_SIZE);
    /*
    // Deserialize from the plaintext
    CDataStream ss(SER_NETWORK, PROTOCOL_VERSION);
    ss << pt.get();

    SaplingNotePlaintext ret;
    ss >> ret;

    assert(ss.size() == 0);*/

    uint256 pk_d;
    if (!librustzcash_ivk_to_pkd(ivk.begin(), ret.d.data(), pk_d.begin())) {
        return boost::none;
    }

    uint256 cmu_expected;
    if (!librustzcash_sapling_compute_cm(
        ret.d.data(),
        pk_d.begin(),
        ret.value(),
        ret.rcm.begin(),
        cmu_expected.begin()
    ))
    {
        return boost::none;
    }

    if (cmu_expected != cmu) {
        return boost::none;
    }

    return ret;
}

boost::optional<SaplingNotePlaintext> SaplingNotePlaintext::decrypt(
    const SaplingEncCiphertext &ciphertext,
    const uint256 &epk,
    const uint256 &esk,
    const uint256 &pk_d,
    const uint256 &cmu
)
{
    auto pt = AttemptSaplingEncDecryption(ciphertext, epk, esk, pk_d);
    if (!pt) {
        elog("decrypt SaplingEncCiphertext failed");
        return boost::none;
    }

    if (pt->size() != 1 + ZC_DIVERSIFIER_SIZE + sizeof(uint64_t) + 32 + ZC_MEMO_SIZE || (*pt)[0] != 0x01) {
        elog("lead byte of SaplingNotePlaintext is not recognized");
        return boost::none;
    }

    SaplingNotePlaintext ret;
    for (int i = 1; i < ZC_DIVERSIFIER_SIZE+1; i++) {
        ret.d[i-1] = (*pt)[i];
    }

    memcpy(&ret.value_, pt->data()+ZC_DIVERSIFIER_SIZE+1, sizeof(uint64_t));

    std::vector<unsigned char> vch;
    int start = ZC_DIVERSIFIER_SIZE+1+sizeof(uint64_t);
    for (int i = start; i < start+32; i++) {
        vch.push_back((*pt)[i]);
    }
    ret.rcm = uint256(vch);

    memcpy(ret.memo_.data(), pt->data()+start+32, ZC_MEMO_SIZE);

    // Deserialize from the plaintext
    /*CDataStream ss(SER_NETWORK, PROTOCOL_VERSION);
    ss << pt.get();

    SaplingNotePlaintext ret;
    ss >> ret;*/

    uint256 cmu_expected;
    if (!librustzcash_sapling_compute_cm(
        ret.d.data(),
        pk_d.begin(),
        ret.value(),
        ret.rcm.begin(),
        cmu_expected.begin()
    ))
    {
        return boost::none;
    }

    if (cmu_expected != cmu) {
        return boost::none;
    }

    return ret;
}

boost::optional<SaplingNotePlaintextEncryptionResult> SaplingNotePlaintext::encrypt(const uint256& pk_d) const
{
    // Get the encryptor
    auto sne = SaplingNoteEncryption::FromDiversifier(d);
    if (!sne) {
        return boost::none;
    }
    auto enc = sne.get();

    // Create the plaintext
    /*CDataStream ss(SER_NETWORK, PROTOCOL_VERSION);
    ss << (*this);
    SaplingEncPlaintext pt;
    assert(pt.size() == ss.size());
    memcpy(&pt[0], &ss[0], pt.size());*/

    SaplingEncPlaintext pt;
    unsigned char* txt = pt.data();
    *txt = 0x01;
    txt++;

    memcpy(txt, this->d.data(), this->d.size());
    txt += this->d.size();

    memcpy(txt, &(this->value_), sizeof(uint64_t));
    txt += sizeof(uint64_t);

    memcpy(txt, this->rcm.begin(), this->rcm.size());
    txt += this->rcm.size();

    memcpy(txt, this->memo_.data(), this->memo_.size());

    // Encrypt the plaintext
    auto encciphertext = enc.encrypt_to_recipient(pk_d, pt);
    if (!encciphertext) {
        return boost::none;
    }
    return SaplingNotePlaintextEncryptionResult(encciphertext.get(), enc);
}


SaplingOutCiphertext SaplingOutgoingPlaintext::encrypt(
        const uint256& ovk,
        const uint256& cv,
        const uint256& cm,
        SaplingNoteEncryption& enc
    ) const
{
    // Create the plaintext
    /*CDataStream ss(SER_NETWORK, PROTOCOL_VERSION);
    ss << (*this);
    SaplingOutPlaintext pt;
    assert(pt.size() == ss.size());
    memcpy(&pt[0], &ss[0], pt.size());*/

    SaplingOutPlaintext pt;
    unsigned char* txt = pt.data();
    memcpy(txt, this->pk_d.begin(), this->pk_d.size());
    txt += this->pk_d.size();
    memcpy(txt, this->esk.begin(), this->esk.size());

    return enc.encrypt_to_ourselves(ovk, cv, cm, pt);
}
