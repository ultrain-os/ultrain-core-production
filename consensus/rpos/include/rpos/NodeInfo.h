#pragma once

#include <core/Redefined.h>
#include <crypto/PrivateKey.h>
#include <crypto/PublicKey.h>

namespace ultrainio {
    class NodeInfo {
    public:
        void setMyInfoAsCommitteeKey(const std::string& sk, const std::string& account);

        AccountName getMyAccount() const;

        PrivateKey getPrivateKey() const;

    private:
        PrivateKey m_privateKey;

        std::string m_account;
    };
}