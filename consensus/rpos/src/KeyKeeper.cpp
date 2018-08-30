#include "rpos/KeyKeeper.h"

namespace ultrainio {

    void KeyKeeper::setGenesisLeaderKeyPair(const std::string& pk, const std::string& sk, const std::string& account) {
        m_genesisLeaderPk = PublicKey(pk);
        m_genesisLeaderSk = PrivateKey(sk, m_genesisLeaderPk);
        m_genesisLeader = account;
        ULTRAIN_ASSERT(m_genesisLeaderPk.isValid(),
                       chain::chain_exception,
                       "should set correct genesis public key");
        if (!m_genesisLeaderSk.isValid() || account.empty()) {
            dlog("not genesis leader. pk : ${pk}, sk : ${sk} account : ${account}", ("pk", pk)("sk", sk)("account", account));
        } else {
            dlog("genesis leader. pk : ${pk}, sk : ${sk} account : ${account}", ("pk", pk)("sk", sk)("account", account));
            ULTRAIN_ASSERT(!account.empty(), chain::chain_exception, "genesis leader account is empty");
            ULTRAIN_ASSERT(PrivateKey::verifyKeyPair(m_genesisLeaderPk, m_genesisLeaderSk),
                           chain::chain_exception, "verify genesis leader key pair failed");
        }
    }

    void KeyKeeper::setCommitteeKeyPair(const std::string& pk, const std::string& sk, const std::string& account) {
        m_publicKey = PublicKey(pk);
        m_privateKey = PrivateKey(sk, m_publicKey);
        m_account = account;
        dlog("My committee key pair. pk : ${pk}, sk : ${sk} account : ${account}", ("pk", pk)("sk", sk)("account", account));
        ULTRAIN_ASSERT(PrivateKey::verifyKeyPair(m_publicKey, m_privateKey),
                       chain::chain_exception,
                       "should set correct committee key pair.");
        ULTRAIN_ASSERT(!account.empty(), chain::chain_exception, "account is empty");
    }
}