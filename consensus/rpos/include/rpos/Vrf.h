#pragma once

#include "core/types.h"

namespace ultrainio {
    // forward declare
    class Seed;
    class Proof;

    class Vrf {
    public:
        static const int kProposer;
        static const int kVoter;
        static Proof vrf(const consensus::PrivateKeyType& privateKey, const Seed& seed, int role);
        static bool verify(const consensus::PublicKeyType& publicKey, const Proof& proof, const Seed& seed, int role);
    };
}