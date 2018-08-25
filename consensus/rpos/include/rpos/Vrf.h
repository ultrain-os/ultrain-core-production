#pragma once

namespace ultrainio {
    // forward declare
    class PrivateKey;
    class PublicKey;
    class Seed;
    class Proof;

    class Vrf {
    public:
        static const int kProposer;
        static const int kVoter;
        static Proof vrf(const PrivateKey& privateKey, const Seed& seed, int role);
        static bool verify(const PublicKey& publicKey, const Proof& proof, const Seed& seed, int role);
    };
}