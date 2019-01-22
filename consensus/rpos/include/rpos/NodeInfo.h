#pragma once

#include <core/Redefined.h>
#include <crypto/PrivateKey.h>
#include <crypto/PublicKey.h>

namespace ultrainio {
    class NodeInfo {
    public:
        NodeInfo();

        ~NodeInfo();

        void setMyInfoAsCommitteeKey(const std::string& sk, const std::string& blsSk, const std::string& account);

        AccountName getMyAccount() const;

        PrivateKey getPrivateKey() const;

        bool getMyBlsPrivateKey(unsigned char* sk, int skSize) const;

        bool getMyBlsPublicKey(unsigned char* blsPk, int pkSize) const;

    private:
        PrivateKey m_privateKey;

        unsigned char* m_blsPrivateKey;;

        std::string m_account;
    };
}