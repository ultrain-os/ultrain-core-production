#pragma once

#include <core/types.h>
#include <crypto/PrivateKey.h>
#include <crypto/PublicKey.h>

namespace ultrainio {
    class NodeInfo {
    public:
        NodeInfo();

        ~NodeInfo();

        void setCommitteeInfo(const std::string& account, const std::string& sk, const std::string& blsSk, const std::string& accountSk);

        AccountName getMyAccount() const;

        PrivateKey getPrivateKey() const;

        fc::crypto::private_key getAccountPrivateKey() const;

        bool getMyBlsPrivateKey(unsigned char* sk, int skSize) const;

    private:
        PrivateKey m_privateKey;

        unsigned char* m_blsPrivateKey;;

        std::string m_account;

        fc::crypto::private_key m_accountSk;
    };
}