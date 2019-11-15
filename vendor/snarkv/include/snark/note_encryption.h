/*
See the Zcash protocol specification for more information.
https://github.com/zcash/zips/blob/master/protocol/protocol.pdf
*/

#ifndef ZC_NOTE_ENCRYPTION_H_
#define ZC_NOTE_ENCRYPTION_H_

#include "snark/uint256.h"
#include "snark/uint252.h"

#include "snark/zcash.h"
#include "snark/address.h"

#include <array>

using namespace zero;

namespace libzcash {

// Ciphertext for the recipient to decrypt
typedef std::array<unsigned char, ZC_SAPLING_ENCCIPHERTEXT_SIZE> SaplingEncCiphertext;
typedef std::array<unsigned char, ZC_SAPLING_ENCPLAINTEXT_SIZE> SaplingEncPlaintext;

// Ciphertext for outgoing viewing key to decrypt
typedef std::array<unsigned char, ZC_SAPLING_OUTCIPHERTEXT_SIZE> SaplingOutCiphertext;
typedef std::array<unsigned char, ZC_SAPLING_OUTPLAINTEXT_SIZE> SaplingOutPlaintext;

//! This is not a thread-safe API.
class SaplingNoteEncryption {
protected:
    // Ephemeral public key
    zero::uint256 epk;

    // Ephemeral secret key
    zero::uint256 esk;

    bool already_encrypted_enc;
    bool already_encrypted_out;

    SaplingNoteEncryption(zero::uint256 epk, zero::uint256 esk) : epk(epk), esk(esk), already_encrypted_enc(false), already_encrypted_out(false) {

    }

public:

    static boost::optional<SaplingNoteEncryption> FromDiversifier(diversifier_t d);

    boost::optional<SaplingEncCiphertext> encrypt_to_recipient(
        const zero::uint256 &pk_d,
        const SaplingEncPlaintext &message
    );

    SaplingOutCiphertext encrypt_to_ourselves(
        const zero::uint256 &ovk,
        const zero::uint256 &cv,
        const zero::uint256 &cm,
        const SaplingOutPlaintext &message
    );

    zero::uint256 get_epk() const {
        return epk;
    }

    zero::uint256 get_esk() const {
        return esk;
    }
};

// Attempts to decrypt a Sapling note. This will not check that the contents
// of the ciphertext are correct.
boost::optional<SaplingEncPlaintext> AttemptSaplingEncDecryption(
    const SaplingEncCiphertext &ciphertext,
    const zero::uint256 &ivk,
    const zero::uint256 &epk
);

// Attempts to decrypt a Sapling note using outgoing plaintext.
// This will not check that the contents of the ciphertext are correct.
boost::optional<SaplingEncPlaintext> AttemptSaplingEncDecryption (
    const SaplingEncCiphertext &ciphertext,
    const zero::uint256 &epk,
    const zero::uint256 &esk,
    const zero::uint256 &pk_d
);

// Attempts to decrypt a Sapling note. This will not check that the contents
// of the ciphertext are correct.
boost::optional<SaplingOutPlaintext> AttemptSaplingOutDecryption(
    const SaplingOutCiphertext &ciphertext,
    const zero::uint256 &ovk,
    const zero::uint256 &cv,
    const zero::uint256 &cm,
    const zero::uint256 &epk
);

template<size_t MLEN>
class NoteEncryption {
protected:
    enum { CLEN=MLEN+NOTEENCRYPTION_AUTH_BYTES };
    zero::uint256 epk;
    zero::uint256 esk;
    unsigned char nonce;
    zero::uint256 hSig;

public:
    typedef std::array<unsigned char, CLEN> Ciphertext;
    typedef std::array<unsigned char, MLEN> Plaintext;

    NoteEncryption(zero::uint256 hSig);

    // Gets the ephemeral secret key
    zero::uint256 get_esk() {
        return esk;
    }

    // Gets the ephemeral public key
    zero::uint256 get_epk() {
        return epk;
    }

    // Encrypts `message` with `pk_enc` and returns the ciphertext.
    // This is only called ZC_NUM_JS_OUTPUTS times for a given instantiation; 
    // but can be called 255 times before the nonce-space runs out.
    Ciphertext encrypt(const zero::uint256 &pk_enc,
                       const Plaintext &message
                      );

    // Creates a NoteEncryption private key
    static zero::uint256 generate_privkey(const uint252 &a_sk);

    // Creates a NoteEncryption public key from a private key
    static zero::uint256 generate_pubkey(const zero::uint256 &sk_enc);
};

template<size_t MLEN>
class NoteDecryption {
protected:
    enum { CLEN=MLEN+NOTEENCRYPTION_AUTH_BYTES };
    zero::uint256 sk_enc;
    zero::uint256 pk_enc;

public:
    typedef std::array<unsigned char, CLEN> Ciphertext;
    typedef std::array<unsigned char, MLEN> Plaintext;

    NoteDecryption() { }
    NoteDecryption(zero::uint256 sk_enc);

    Plaintext decrypt(const Ciphertext &ciphertext,
                      const zero::uint256 &epk,
                      const zero::uint256 &hSig,
                      unsigned char nonce
                     ) const;

    friend inline bool operator==(const NoteDecryption& a, const NoteDecryption& b) {
        return a.sk_enc == b.sk_enc && a.pk_enc == b.pk_enc;
    }
    friend inline bool operator<(const NoteDecryption& a, const NoteDecryption& b) {
        return (a.sk_enc < b.sk_enc ||
                (a.sk_enc == b.sk_enc && a.pk_enc < b.pk_enc));
    }
};

zero::uint256 random_uint256();
uint252 random_uint252();

class note_decryption_failed : public std::runtime_error {
public:
    note_decryption_failed() : std::runtime_error("Could not decrypt message") { }
};



// Subclass PaymentDisclosureNoteDecryption provides a method to decrypt a note with esk.
template<size_t MLEN>
class PaymentDisclosureNoteDecryption : public NoteDecryption<MLEN> {
protected:
public:
    enum { CLEN=MLEN+NOTEENCRYPTION_AUTH_BYTES };
    typedef std::array<unsigned char, CLEN> Ciphertext;
    typedef std::array<unsigned char, MLEN> Plaintext;

    PaymentDisclosureNoteDecryption() : NoteDecryption<MLEN>() {}
    PaymentDisclosureNoteDecryption(zero::uint256 sk_enc) : NoteDecryption<MLEN>(sk_enc) {}

    Plaintext decryptWithEsk(
        const Ciphertext &ciphertext,
        const zero::uint256 &pk_enc,
        const zero::uint256 &esk,
        const zero::uint256 &hSig,
        unsigned char nonce
        ) const;
};

}

typedef libzcash::NoteEncryption<ZC_NOTEPLAINTEXT_SIZE> ZCNoteEncryption;
typedef libzcash::NoteDecryption<ZC_NOTEPLAINTEXT_SIZE> ZCNoteDecryption;

typedef libzcash::PaymentDisclosureNoteDecryption<ZC_NOTEPLAINTEXT_SIZE> ZCPaymentDisclosureNoteDecryption;

#endif /* ZC_NOTE_ENCRYPTION_H_ */
