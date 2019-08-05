#include <rpos/NodeInfo.h>

#include <base/Hex.h>
#include <crypto/Bls.h>
#include <rpos/Genesis.h>
#include <rpos/Node.h>

namespace ultrainio {

    NodeInfo::NodeInfo() {
        m_blsPrivateKey = (unsigned char*)malloc(Bls::BLS_PRI_KEY_LENGTH);
    }

    NodeInfo::~NodeInfo() {
        if (m_blsPrivateKey) {
            free(m_blsPrivateKey);
            m_blsPrivateKey = nullptr;
        }
    }
    /**
     *
     * @param sk private key for committee member
     * @param account committee member name
     * 1. genesis can be config by itself
     * 2. non-producer can not config private and account
     */
    void NodeInfo::setMyInfoAsCommitteeKey(const std::string& sk, const std::string& blsSk, const std::string& account) {
        m_privateKey = PrivateKey(sk);
        m_account = account;
        if (Genesis::kGenesisAccount == m_account) {
            ULTRAIN_ASSERT(m_privateKey.getPublicKey() == PublicKey(Genesis::s_genesisPk),
                    chain::chain_exception,
                    "genesis key pair invalid");
        }
        if (Node::getInstance()->getNonProducingNode()) {
            ilog("Non Producer Node");
            return;
        }
        ULTRAIN_ASSERT(PrivateKey::verifyKeyPair(m_privateKey.getPublicKey(), m_privateKey),
                       chain::chain_exception,
                       "verify private key error.");
        ULTRAIN_ASSERT(!m_account.empty(), chain::chain_exception, "account is empty");
        ULTRAIN_ASSERT(blsSk.length() == Bls::BLS_PRI_KEY_LENGTH * 2, chain::chain_exception, "bls private key error");
        Hex::fromHex<unsigned char>(blsSk, m_blsPrivateKey, Bls::BLS_PRI_KEY_LENGTH);
    }

    AccountName NodeInfo::getMyAccount() const {
        return AccountName(m_account);
    }

    PrivateKey NodeInfo::getPrivateKey() const {
        return m_privateKey;
    }

    bool NodeInfo::getMyBlsPrivateKey(unsigned char* sk, int skSize) const {
        if (!sk || skSize < Bls::BLS_PRI_KEY_LENGTH) {
            return false;
        }
        memcpy(sk, m_blsPrivateKey, Bls::BLS_PRI_KEY_LENGTH);
        return true;
    }
}
