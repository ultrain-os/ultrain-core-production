/*
Zcash uses SHA256Compress as a PRF for various components
within the zkSNARK circuit.
*/

#ifndef ZC_PRF_H_
#define ZC_PRF_H_

#include "uint256.h"
#include "uint252.h"

#include <array>

//! Sprout functions
zero::uint256 PRF_addr_a_pk(const uint252& a_sk);
zero::uint256 PRF_addr_sk_enc(const uint252& a_sk);
zero::uint256 PRF_nf(const uint252& a_sk, const zero::uint256& rho);
zero::uint256 PRF_pk(const uint252& a_sk, size_t i0, const zero::uint256& h_sig);
zero::uint256 PRF_rho(const uint252& phi, size_t i0, const zero::uint256& h_sig);

//! Sapling functions
zero::uint256 PRF_ask(const zero::uint256& sk);
zero::uint256 PRF_nsk(const zero::uint256& sk);
zero::uint256 PRF_ovk(const zero::uint256& sk);

std::array<unsigned char, 11> default_diversifier(const zero::uint256& sk);

#endif // ZC_PRF_H_
