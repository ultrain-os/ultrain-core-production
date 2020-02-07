#include <rpos/NodeInfo.h>

#include <base/Hex.h>
#include <crypto/Bls.h>
#include <rpos/Genesis.h>
#include <rpos/Node.h>

namespace ultrainio {
    std::shared_ptr<NodeInfo> NodeInfo::s_instance = nullptr;

    AccountName NodeInfo::getMainAccount() {
        return AccountName(NodeInfo::getInstance()->getAccount(0));
    }

    PrivateKey NodeInfo::getMainPrivateKey() {
        return NodeInfo::getInstance()->getPrivateKey(0);
    }

    bool NodeInfo::getMainBlsPriKey(unsigned char* sk, int skSize) {
        return NodeInfo::getInstance()->getBlsPrivateKey(sk, skSize, 0);
    }

    fc::crypto::private_key NodeInfo::getMainAccountTrxPriKey() {
        return NodeInfo::getInstance()->realGetMainAccountTrxPriKey();
    }

    NodeInfo::~NodeInfo() {
        for (size_t i = 0; i < m_blsPrivateKeyV.size(); i++) {
            free(m_blsPrivateKeyV[i]);
            m_blsPrivateKeyV[i] = nullptr;
        }
    }

    std::shared_ptr<NodeInfo> NodeInfo::getInstance() {
        if (!s_instance) {
            s_instance = std::make_shared<NodeInfo>();
        }
        return s_instance;
    }

    NodeInfo::NodeInfo() {
    }

    void NodeInfo::setCommitteeInfo(const std::vector<std::string>& accountV, const std::vector<std::string>& skV,
            const std::vector<std::string>& blsSkV, const std::vector<std::string>& accTrxPriKeyV) {
        ULTRAIN_ASSERT(accountV.size() != 0, chain::chain_exception, "account is empty");
        ULTRAIN_ASSERT(accountV.size() == skV.size() && skV.size() == blsSkV.size() && blsSkV.size() == accTrxPriKeyV.size(),
                       chain::chain_exception,
                       "account key not match");
        m_accountV = accountV;
        for (size_t i = 0; i < accountV.size(); i++) {
            m_privateKeyV.push_back(PrivateKey(skV[i]));
            m_accountTrxPriKeyV.push_back(fc::crypto::private_key(accTrxPriKeyV[i]));
            ULTRAIN_ASSERT(blsSkV[i].length() == Bls::BLS_PRI_KEY_LENGTH * 2, chain::chain_exception, "bls private key error");
            unsigned char* blsPrivateKey = (unsigned char*)malloc(Bls::BLS_PRI_KEY_LENGTH);
            Hex::fromHex<unsigned char>(blsSkV[i], blsPrivateKey, Bls::BLS_PRI_KEY_LENGTH);
            m_blsPrivateKeyV.push_back(blsPrivateKey);
        }
        if (m_accountV.size() == 1 && Genesis::kGenesisAccount == m_accountV[0]) {
            ULTRAIN_ASSERT(m_privateKeyV[0].getPublicKey() == PublicKey(Genesis::s_genesisPk),
                           chain::chain_exception,
                           "genesis key pair invalid");
        }
    }

    const std::vector<std::string>& NodeInfo::getAccountList() const {
        return m_accountV;
    }

    fc::crypto::private_key NodeInfo::realGetMainAccountTrxPriKey() const {
        if (m_accountTrxPriKeyV.size() > 0) {
            return m_accountTrxPriKeyV[0];
        }
        return fc::crypto::private_key();
    }

    bool NodeInfo::hasAccount(const std::string& account, size_t& index) const {
        for (size_t i = 0; i < m_accountV.size(); i++) {
            if (account == m_accountV[i]) {
                index = i;
                return true;
            }
        }
        return false;
    }

    bool NodeInfo::hasAccount(const std::string& account) const {
        size_t index;
        return hasAccount(account, index);
    }

    PrivateKey NodeInfo::getPrivateKey(size_t index) const {
        return m_privateKeyV[index];
    }

    std::string NodeInfo::getAccount(size_t index) const {
        return m_accountV[index];
    }

    bool NodeInfo::getBlsPrivateKey(unsigned char* sk, int skSize, size_t index) const {
        if (!sk || skSize < Bls::BLS_PRI_KEY_LENGTH) {
            return false;
        }
        memcpy(sk, m_blsPrivateKeyV[index], Bls::BLS_PRI_KEY_LENGTH);
        return true;
    }
}
