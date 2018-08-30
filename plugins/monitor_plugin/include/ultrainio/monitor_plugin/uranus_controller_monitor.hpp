#pragma once

#include <rpos/UranusController.h>

namespace ultrainio {
    struct BlockHeaderDigest {
        chain::block_timestamp_type      timestamp;
        AccountName                      proposer;
        chain::block_id_type             previous;
        chain::block_id_type             myid;
        uint32_t                         blockNum;

        void digestFromBlockHeader(const BlockHeader& block_header) {
            timestamp  = block_header.timestamp;
            proposer   = block_header.proposer;
            previous   = block_header.previous;
            myid       = block_header.id();
            blockNum   = block_header.block_num();
        }
    };

    struct EchoMsgDigest {
        BlockHeaderDigest     head;
        int32_t               phase;
        uint32_t              baxCount;

        void digestFromeEchoMsg(const EchoMsg& echo_msg) {
            head.digestFromBlockHeader(echo_msg.blockHeader);
            phase    = echo_msg.phase;
            baxCount = echo_msg.baxCount;
        }
    };

    struct EchoMsgInfoDigest {
        EchoMsgDigest             echoMsg;
        bool                      hasSend;
        uint32_t                  accountPoolSize;
        std::vector<AccountName>  account_pool; //public key pool

        void digestFromeEchoMsgInfo(const echo_message_info& echo_msg_info) {
            echoMsg.digestFromeEchoMsg(echo_msg_info.echo);
            hasSend = echo_msg_info.hasSend;
            accountPoolSize = echo_msg_info.accountPool.size();
            account_pool.assign(echo_msg_info.accountPool.begin(), echo_msg_info.accountPool.end());
        }
    };

    typedef std::vector<std::pair<chain::block_id_type, EchoMsgInfoDigest>> echo_msg_digest_vect;

    class UranusControllerMonitor
    {
    public:
        UranusControllerMonitor(std::weak_ptr<UranusController> pController):m_pController(pController) {}
        ~UranusControllerMonitor() = default;
        UranusControllerMonitor(const UranusControllerMonitor&) = delete;
        const UranusControllerMonitor& operator=(const UranusControllerMonitor&) = delete;

        BlockHeaderDigest findProposeMsgByBlockId(const chain::block_id_type& bid) const {
            BlockHeaderDigest tempHeaderDigest;
            std::shared_ptr<UranusController> pController = m_pController.lock();
            if (pController) {
                auto ite = pController->m_proposerMsgMap.find(bid);
                if(ite != pController->m_proposerMsgMap.end()){
                    tempHeaderDigest.digestFromBlockHeader(ite->second.block);
                } else {
                    ULTRAIN_THROW(chain::msg_not_found_exception, "Propose msg not found by id." );
                }
            }
            return tempHeaderDigest;
        }

        EchoMsgInfoDigest findEchoMsgByBlockId(const chain::block_id_type& bid) const {
            EchoMsgInfoDigest tempEchoDigest;
            std::shared_ptr<UranusController> pController = m_pController.lock();
            if (pController) {
                auto ite = pController->m_echoMsgMap.find(bid);
                if(ite != pController->m_echoMsgMap.end()) {
                    tempEchoDigest.digestFromeEchoMsgInfo(ite->second);
                } else {
                    ULTRAIN_THROW(chain::msg_not_found_exception, "Echo msg info not found by id." );
                }
            }
            return tempEchoDigest;
        }

        std::vector<BlockHeaderDigest> findProposeCacheByKey(const msgkey& msg_key) const {
            std::vector<BlockHeaderDigest> tempDigestVect;
            std::shared_ptr<UranusController> pController = m_pController.lock();
            if (pController) {
                auto ite = pController->m_cacheProposeMsgMap.find(msg_key);
                if(ite != pController->m_cacheProposeMsgMap.end()) {
                    for(const auto& proposeMsg : ite->second){
                        BlockHeaderDigest tempHeader;
                        tempHeader.digestFromBlockHeader(proposeMsg.block);
                        tempDigestVect.push_back(tempHeader);
                    }
                } else {
                    ULTRAIN_THROW(chain::msg_not_found_exception, "Propose msg not found by key." );
                }
            }
            return tempDigestVect;
        }

        std::vector<EchoMsgDigest> findEchoCacheByKey(const msgkey& msg_key) const {
            std::vector<EchoMsgDigest> tempEchoDigestVect;
            std::shared_ptr<UranusController> pController = m_pController.lock();
            if (pController) {
                auto ite = pController->m_cacheEchoMsgMap.find(msg_key);
                if(ite != pController->m_cacheEchoMsgMap.end()) {
                    for(const auto& echoMsg : ite->second) {
                        EchoMsgDigest tempEchoMsg;
                        tempEchoMsg.digestFromeEchoMsg(echoMsg);
                        tempEchoDigestVect.push_back(tempEchoMsg);
                    }
                } else {
                    ULTRAIN_THROW(chain::msg_not_found_exception, "Echo msg not found by key." );
                }
            }
            return tempEchoDigestVect;
        }

        echo_msg_digest_vect findEchoApMsgByKey(const msgkey& msg_key) const {
            echo_msg_digest_vect tempEchoMsgInfoDigestVect;
            std::shared_ptr<UranusController> pController = m_pController.lock();
            if (pController) {
                auto ite = pController->m_echoMsgAllPhase.find(msg_key);
                if(ite != pController->m_echoMsgAllPhase.end()) {
                    for(const auto& echoMsgInfoPair : ite->second) {
                        EchoMsgInfoDigest tempEchoMsgInfo;
                        tempEchoMsgInfo.digestFromeEchoMsgInfo(echoMsgInfoPair.second);
                        std::pair<chain::block_id_type, EchoMsgInfoDigest> tempPair(echoMsgInfoPair.first, tempEchoMsgInfo);
                        tempEchoMsgInfoDigestVect.push_back(tempPair);
                    }
                } else{
                    ULTRAIN_THROW(chain::msg_not_found_exception, "Echo msg info not found by key." );
                }
            }
            return tempEchoMsgInfoDigestVect;
        }

        void getContainersSize(uint32_t& proposeNum, uint32_t& echoNum, uint32_t& proposeCacheSize,
                               uint32_t& echoCacheSize, uint32_t& allPhaseEchoNum) const {
            std::shared_ptr<UranusController> pController = m_pController.lock();
            if (pController) {
                proposeNum = pController->m_proposerMsgMap.size();
                echoNum = pController->m_echoMsgMap.size();
                proposeCacheSize = pController->m_cacheProposeMsgMap.size();
                echoCacheSize = pController->m_cacheEchoMsgMap.size();
                allPhaseEchoNum = pController->m_echoMsgAllPhase.size();
            }
            else {
                proposeNum = 0;
                echoNum = 0;
                proposeCacheSize = 0;
                echoCacheSize = 0;
                allPhaseEchoNum = 0;
            }
        }
    private:
        std::weak_ptr<UranusController> m_pController;
    };
}
