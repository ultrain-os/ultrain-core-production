#include "snark/address.h"
#include "snark/note_encryption.h"
#include "snark/hash.h"
#include "snark/prf.h"
#include "snark/librustzcash.h"

namespace libzcash {

uint256 ReceivingKey::pk_enc() const {
    return ZCNoteEncryption::generate_pubkey(*this);
}

//! Sapling
uint256 SaplingPaymentAddress::GetHash() const {
    char* buffer = new char[d.size() + pk_d.size()];
    size_t pos = 0;
    for (size_t i = 0; i < d.size(); i++) {
        buffer[pos++] = d[i];
    }
    for (const unsigned char* p = pk_d.begin(); p != pk_d.end(); p++) {
        buffer[pos++] = *p;
    }

    uint256 result = Hash(buffer, buffer + pos);
    delete [] buffer;
    return result;
    //CDataStream ss(SER_NETWORK, PROTOCOL_VERSION);
    //ss << *this;
    //return Hash(ss.begin(), ss.end());
}

SaplingFullViewingKey SaplingExpandedSpendingKey::full_viewing_key() const {
    uint256 ak;
    uint256 nk;
    librustzcash_ask_to_ak(ask.begin(), ak.begin());
    librustzcash_nsk_to_nk(nsk.begin(), nk.begin());
    return SaplingFullViewingKey(ak, nk, ovk);
}

SaplingExpandedSpendingKey SaplingSpendingKey::expanded_spending_key() const {
    return SaplingExpandedSpendingKey(PRF_ask(*this), PRF_nsk(*this), PRF_ovk(*this));
}

SaplingFullViewingKey SaplingSpendingKey::full_viewing_key() const {
    return expanded_spending_key().full_viewing_key();
}

SaplingIncomingViewingKey SaplingFullViewingKey::in_viewing_key() const {
    uint256 ivk;
    librustzcash_crh_ivk(ak.begin(), nk.begin(), ivk.begin());
    return SaplingIncomingViewingKey(ivk);
}

bool SaplingFullViewingKey::is_valid() const {
    uint256 ivk;
    librustzcash_crh_ivk(ak.begin(), nk.begin(), ivk.begin());
    return !ivk.IsNull();
}

uint256 SaplingFullViewingKey::GetFingerprint() const {
    char* buffer = new char[ak.size() + nk.size() + ovk.size()];
    size_t pos = 0;
    for (const unsigned char* p = ak.begin(); p != ak.end(); p++) {
        buffer[pos++] = *p;
    }

    for (const unsigned char* p = nk.begin(); p != nk.end(); p++) {
        buffer[pos++] = *p;
    }

    for (const unsigned char* p = ovk.begin(); p != ovk.end(); p++) {
        buffer[pos++] = *p;
    }

    uint256 result = Hash(buffer, buffer + pos);
    delete [] buffer;
    return result;

    //CBLAKE2bWriter ss(SER_GETHASH, 0, ZCASH_SAPLING_FVFP_PERSONALIZATION);
    //ss << *this;
    //return ss.GetHash();
}


SaplingSpendingKey SaplingSpendingKey::random() {
    while (true) {
        auto sk = SaplingSpendingKey(random_uint256());
        if (sk.full_viewing_key().is_valid()) {
            return sk;
        }
    }
}

boost::optional<SaplingPaymentAddress> SaplingIncomingViewingKey::address(diversifier_t d) const {
    uint256 pk_d;
    if (librustzcash_check_diversifier(d.data())) {
        librustzcash_ivk_to_pkd(this->begin(), d.data(), pk_d.begin());
        return SaplingPaymentAddress(d, pk_d);
    } else {
        return boost::none;
    }
}

SaplingPaymentAddress SaplingSpendingKey::default_address() const {
    // Iterates within default_diversifier to ensure a valid address is returned
    auto addrOpt = full_viewing_key().in_viewing_key().address(default_diversifier(*this));
    assert(addrOpt != boost::none);
    return addrOpt.value();
}

}

bool IsValidPaymentAddress(const libzcash::PaymentAddress& zaddr) {
    return zaddr.which() != 0;
}

/*bool IsValidViewingKey(const libzcash::ViewingKey& vk) {
    return vk.which() != 0;
}*/
