/* Copyright (C) 1995-1998 Eric Young (eay@cryptsoft.com)
 * All rights reserved.
 *
 * This package is an SSL implementation written
 * by Eric Young (eay@cryptsoft.com).
 * The implementation was written so as to conform with Netscapes SSL.
 *
 * This library is free for commercial and non-commercial use as long as
 * the following conditions are aheared to.  The following conditions
 * apply to all code found in this distribution, be it the RC4, RSA,
 * lhash, DES, etc., code; not just the SSL code.  The SSL documentation
 * included with this distribution is covered by the same copyright terms
 * except that the holder is Tim Hudson (tjh@cryptsoft.com).
 *
 * Copyright remains Eric Young's, and as such any Copyright notices in
 * the code are not to be removed.
 * If this package is used in a product, Eric Young should be given attribution
 * as the author of the parts of the library used.
 * This can be in the form of a textual message at program startup or
 * in documentation (online or textual) provided with the package.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    "This product includes cryptographic software written by
 *     Eric Young (eay@cryptsoft.com)"
 *    The word 'cryptographic' can be left out if the rouines from the library
 *    being used are not cryptographic related :-).
 * 4. If you include any Windows specific code (or a derivative thereof) from
 *    the apps directory (application code) you must include an acknowledgement:
 *    "This product includes software written by Tim Hudson (tjh@cryptsoft.com)"
 *
 * THIS SOFTWARE IS PROVIDED BY ERIC YOUNG ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * The licence and distribution terms for any publically available version or
 * derivative of this code cannot be changed.  i.e. this code cannot simply be
 * copied and put under another distribution licence
 * [including the GNU Public Licence.]
 */
/* ====================================================================
 * Copyright (c) 1998-2001 The OpenSSL Project.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. All advertising materials mentioning features or use of this
 *    software must display the following acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit. (http://www.openssl.org/)"
 *
 * 4. The names "OpenSSL Toolkit" and "OpenSSL Project" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. For written permission, please contact
 *    openssl-core@openssl.org.
 *
 * 5. Products derived from this software may not be called "OpenSSL"
 *    nor may "OpenSSL" appear in their names without prior written
 *    permission of the OpenSSL Project.
 *
 * 6. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit (http://www.openssl.org/)"
 *
 * THIS SOFTWARE IS PROVIDED BY THE OpenSSL PROJECT ``AS IS'' AND ANY
 * EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE OpenSSL PROJECT OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 * ====================================================================
 *
 * This product includes cryptographic software written by Eric Young
 * (eay@cryptsoft.com).  This product includes software written by Tim
 * Hudson (tjh@cryptsoft.com). */

#ifndef OPENSSL_HEADER_CRYPTO_INTERNAL_H
#define OPENSSL_HEADER_CRYPTO_INTERNAL_H

//#include <boringssl/base.h>
//#include <boringssl/ex_data.h>
//#include <boringssl/stack.h>
//#include <boringssl/thread.h>
#include "type_check.h"

#include <assert.h>
#include <string.h>

#if defined(__cplusplus)
extern "C" {
#endif

#if defined(__x86_64) || defined(_M_AMD64) || defined(_M_X64)
#define OPENSSL_64_BIT
#define OPENSSL_X86_64
#elif defined(__x86) || defined(__i386) || defined(__i386__) || defined(_M_IX86)
#define OPENSSL_32_BIT
#define OPENSSL_X86
#elif defined(__aarch64__)
#define OPENSSL_64_BIT
#define OPENSSL_AARCH64
#elif defined(__arm) || defined(__arm__) || defined(_M_ARM)
#define OPENSSL_32_BIT
#define OPENSSL_ARM
#elif (defined(__PPC64__) || defined(__powerpc64__)) && defined(_LITTLE_ENDIAN)
#define OPENSSL_64_BIT
#define OPENSSL_PPC64LE
#elif defined(__mips__) && !defined(__LP64__)
#define OPENSSL_32_BIT
#define OPENSSL_MIPS
#elif defined(__mips__) && defined(__LP64__)
#define OPENSSL_64_BIT
#define OPENSSL_MIPS64
#elif defined(__pnacl__)
#define OPENSSL_32_BIT
#define OPENSSL_PNACL
#elif defined(__wasm__)
#define OPENSSL_32_BIT
#elif defined(__asmjs__)
#define OPENSSL_32_BIT
#elif defined(__myriad2__)
#define OPENSSL_32_BIT
#else
// Note BoringSSL only supports standard 32-bit and 64-bit two's-complement,
// little-endian architectures. Functions will not produce the correct answer
// on other systems. Run the crypto_test binary, notably
// crypto/compiler_test.cc, before adding a new architecture.
#error "Unknown target CPU"
#endif

#if defined(OPENSSL_64_BIT)
typedef uint64_t crypto_word_t;
#elif defined(OPENSSL_32_BIT)
typedef uint32_t crypto_word_t;
#else
#error "Must define either OPENSSL_32_BIT or OPENSSL_64_BIT"
#endif

#if (!defined(_MSC_VER) || defined(__clang__)) && defined(OPENSSL_64_BIT)
#define BORINGSSL_HAS_UINT128
typedef __int128_t int128_t;
typedef __uint128_t uint128_t;

// clang-cl supports __uint128_t but modulus and division don't work.
// https://crbug.com/787617.
#if !defined(_MSC_VER) || !defined(__clang__)
#define BORINGSSL_CAN_DIVIDE_UINT128
#endif
#endif

// Language bug workarounds.
//
// Most C standard library functions are undefined if passed NULL, even when the
// corresponding length is zero. This gives them (and, in turn, all functions
// which call them) surprising behavior on empty arrays. Some compilers will
// miscompile code due to this rule. See also
// https://www.imperialviolet.org/2016/06/26/nonnull.html
//
// These wrapper functions behave the same as the corresponding C standard
// functions, but behave as expected when passed NULL if the length is zero.
//
// Note |OPENSSL_memcmp| is a different function from |CRYPTO_memcmp|.

// C++ defines |memchr| as a const-correct overload.
#if defined(__cplusplus)
extern "C++" {

static inline const void *OPENSSL_memchr(const void *s, int c, size_t n) {
  if (n == 0) {
    return NULL;
  }

  return memchr(s, c, n);
}

static inline void *OPENSSL_memchr(void *s, int c, size_t n) {
  if (n == 0) {
    return NULL;
  }

  return memchr(s, c, n);
}

}  // extern "C++"
#else  // __cplusplus

static inline void *OPENSSL_memchr(const void *s, int c, size_t n) {
    if (n == 0) {
        return NULL;
    }

    return memchr(s, c, n);
}

#endif  // __cplusplus

static inline int OPENSSL_memcmp(const void *s1, const void *s2, size_t n) {
    if (n == 0) {
        return 0;
    }

    return memcmp(s1, s2, n);
}

static inline void *OPENSSL_memcpy(void *dst, const void *src, size_t n) {
    if (n == 0) {
        return dst;
    }

    return memcpy(dst, src, n);
}

static inline void *OPENSSL_memmove(void *dst, const void *src, size_t n) {
    if (n == 0) {
        return dst;
    }

    return memmove(dst, src, n);
}

static inline void *OPENSSL_memset(void *dst, int c, size_t n) {
    if (n == 0) {
        return dst;
    }

    return memset(dst, c, n);
}

#if defined(__cplusplus)
}  // extern C
#endif

#endif  // OPENSSL_HEADER_CRYPTO_INTERNAL_H
