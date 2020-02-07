#pragma once

#include <vector>
#include <core/types.h>
#include <crypto/PrivateKey.h>
#include <crypto/PublicKey.h>

namespace ultrainio {
    class NodeInfo {
    public:
        ~NodeInfo();

        NodeInfo();

        static std::shared_ptr<NodeInfo> getInstance();

        static AccountName getMainAccount();

        static PrivateKey getMainPrivateKey();

        static bool getMainBlsPriKey(unsigned char* sk, int skSize);

        static fc::crypto::private_key getMainAccountTrxPriKey();

        bool hasAccount(const std::string& account, size_t& index) const;

        bool hasAccount(const std::string& account) const;

        const std::vector<std::string>& getAccountList() const;

        std::string getAccount(size_t index) const;

        PrivateKey getPrivateKey(size_t index) const;

        bool getBlsPrivateKey(unsigned char* sk, int skSize, size_t index) const;

        void setCommitteeInfo(const std::vector<std::string>& accountV, const std::vector<std::string>& skV,
                              const std::vector<std::string>& blsSkV, const std::vector<std::string>& accTrxPriKeyV);
    private:
        fc::crypto::private_key realGetMainAccountTrxPriKey() const;

        static std::shared_ptr<NodeInfo> s_instance;

        std::vector<PrivateKey> m_privateKeyV;

        std::vector<unsigned char*> m_blsPrivateKeyV;

        std::vector<std::string> m_accountV;

        std::vector<fc::crypto::private_key> m_accountTrxPriKeyV;
    };
}