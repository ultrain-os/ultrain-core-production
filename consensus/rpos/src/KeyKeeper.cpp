#include "rpos/KeyKeeper.h"

namespace ultrainio {

    void KeyKeeper::setMyInfoAsCommitteeKey(const std::string& sk, const std::string& account) {
        m_privateKey = PrivateKey(sk);
        m_account = account;
        dlog("My committee key pair. sk : ${sk} account : ${account}", ("sk", sk)("account", account));
        ULTRAIN_ASSERT(PrivateKey::verifyKeyPair(m_privateKey.getPublicKey(), m_privateKey),
                       chain::chain_exception,
                       "verify private key error.");
        ULTRAIN_ASSERT(!m_account.empty(), chain::chain_exception, "account is empty");
    }

    AccountName KeyKeeper::getMyAccount() const {
        return AccountName(m_account);
    }

    PrivateKey KeyKeeper::getPrivateKey() const {
        return m_privateKey;
    }
}