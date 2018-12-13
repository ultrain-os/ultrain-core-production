/* Copyright (c) 2015, Google Inc.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE. */

#ifndef OPENSSL_HEADER_CURVE25519_H
#define OPENSSL_HEADER_CURVE25519_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
// Ed25519.
//
// Ed25519 is a signature scheme using a twisted-Edwards curve that is
// birationally equivalent to curve25519.
//
// Note that, unlike RFC 8032's formulation, our private key representation
// includes a public key suffix to make multiple key signing operations with the
// same key more efficient. The RFC 8032 key private key is referred to in this
// implementation as the "seed" and is the first 32 bytes of our private key.

#define ED25519_PRIVATE_KEY_LEN 64
#define ED25519_PUBLIC_KEY_LEN 32
#define ED25519_SIGNATURE_LEN 64

// ED25519_keypair sets |out_public_key| and |out_private_key| to a freshly
// generated, public–private key pair.
void ED25519_keypair_u(uint8_t out_public_key[32],
                                    uint8_t out_private_key[64]);

// ED25519_sign sets |out_sig| to be a signature of |message_len| bytes from
// |message| using |private_key|. It returns one on success or zero on
// error.
int ED25519_sign_u(uint8_t out_sig[64], const uint8_t *message,
                                size_t message_len,
                                const uint8_t private_key[64]);

// ED25519_verify returns one iff |signature| is a valid signature, by
// |public_key| of |message_len| bytes from |message|. It returns zero
// otherwise.
int ED25519_verify_u(const uint8_t *message, size_t message_len,
                                  const uint8_t signature[64],
                                  const uint8_t public_key[32]);

// ED25519_keypair_from_seed calculates a public and private key from an
// Ed25519 “seed”. Seed values are not exposed by this API (although they
// happen to be the first 32 bytes of a private key) so this function is for
// interoperating with systems that may store just a seed instead of a full
// private key.
void ED25519_keypair_from_seed_u(uint8_t out_public_key[32],
                                              uint8_t out_private_key[64],
                                              const uint8_t seed[32]);

#if defined(__cplusplus)
}  // extern C
#endif

#endif  // OPENSSL_HEADER_CURVE25519_H
