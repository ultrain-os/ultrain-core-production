#pragma once

#include <vector>
#include <core/types.h>

namespace ultrainio {
    class NodeInfo {
    public:
        ~NodeInfo();

        NodeInfo();

        static std::shared_ptr<NodeInfo> getInstance();

        static AccountName getMainAccount();

        static consensus::PrivateKeyType getMainPrivateKey();

        static bool getMainBlsPriKey(unsigned char* sk, int skSize);

        static chain::private_key_type getMainAccountTrxPriKey();

        bool hasAccount(const std::string& account, size_t& index) const;

        bool hasAccount(const std::string& account) const;

        const std::vector<std::string>& getAccountList() const;

        std::string getAccount(size_t index) const;

        consensus::PrivateKeyType getPrivateKey(size_t index) const;

        bool getBlsPrivateKey(unsigned char* sk, int skSize, size_t index) const;

        void setCommitteeInfo(const std::vector<std::string>& accountV, const std::vector<std::string>& skV,
                              const std::vector<std::string>& blsSkV, const std::vector<std::string>& accTrxPriKeyV);
    private:
        chain::private_key_type realGetMainAccountTrxPriKey() const;

        static std::shared_ptr<NodeInfo> s_instance;

        std::vector<consensus::PrivateKeyType> m_privateKeyV;

        std::vector<unsigned char*> m_blsPrivateKeyV;

        std::vector<std::string> m_accountV;

        std::vector<chain::private_key_type> m_accountTrxPriKeyV;
    };
}