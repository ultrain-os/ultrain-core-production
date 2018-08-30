#pragma once

#include <core/Redefined.h>
#include <crypto/PrivateKey.h>
#include <crypto/PublicKey.h>

namespace ultrainio {
    class KeyKeeper {
    public:
        void setGenesisLeaderKeyPair(const std::string& genesisPk, const std::string& genesisSk, const std::string& account);

        void setCommitteeKeyPair(const std::string& pk, const std::string& sk, const std::string& account);

    private:
        PublicKey m_genesisLeaderPk;

        PrivateKey m_genesisLeaderSk;

        std::string m_genesisLeader;

        PublicKey m_publicKey;

        PrivateKey m_privateKey;

        std::string m_account;

        friend class VoterSystem;
        friend class UranusNodeMonitor;
    };
}