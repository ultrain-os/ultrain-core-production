#pragma once

#include "uranus_controller_monitor.hpp"

#include <rpos/Node.h>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace ultrainio {

    struct UranusNodeInfo {
        bool     ready;
        bool     connected;
        bool     syncing;
        bool     syncFailed;
        bool     isNonProducingNode;
        int32_t  globalProducingNodeNumber;
        int32_t  phase;
        uint32_t baxCount;
        uint32_t proposeMsgNum;
        uint32_t echoMsgnum;
        uint32_t proposeMsgCacheSize;
        uint32_t echoMsgCacheSize;
        uint32_t allPhaseEchoMsgNum;
    };

    struct periodic_reort_data {
        std::string nodeIp;
        uint32_t    blockNum;
        std::string phase;
        uint32_t    baxCount;
        uint32_t    transactionNum;
        bool        syncing;
        bool        syncFailed;
        bool        connected;
        bool        ready;
        bool        nonProducingNode;
        std::string blockHash;
        std::string previousBlockHash;
        std::string ba0BlockTime;
        std::string ba1BlockTime;
        std::string genesisLeaderPk;
        std::string genesisLeaderSk;
        std::string publicKey;
        std::string privateKey;
    };

    class UranusNodeMonitor
    {
    public:
        UranusNodeMonitor(std::weak_ptr<UranusNode> pNode):m_pNode(pNode) {
            phaseStr[0] = "kPhaseInit";
            phaseStr[1] = "kPhaseBA0";
            phaseStr[2] = "kPhaseBA1";
            phaseStr[3] = "kPhaseBAX";
        }

        ~UranusNodeMonitor() = default;
        UranusNodeMonitor(const UranusNodeMonitor& ) = delete;
        const UranusNodeMonitor& operator=(const UranusNodeMonitor&) = delete;

        UranusNodeInfo getNodeInfo() const {
            UranusNodeInfo tempNodeInfo;
            std::shared_ptr<UranusNode> pNode = m_pNode.lock();
            if (pNode) {
                tempNodeInfo.ready = pNode->m_ready;
                tempNodeInfo.connected = pNode->m_connected;
                tempNodeInfo.syncing = pNode->m_syncing;
                tempNodeInfo.syncFailed = pNode->m_syncFailed;
                tempNodeInfo.isNonProducingNode = pNode->m_isNonProducingNode;
                tempNodeInfo.globalProducingNodeNumber = pNode->getCommitteeMemberNumber();
                tempNodeInfo.phase = static_cast<int32_t>(pNode->m_phase);
                tempNodeInfo.baxCount = pNode->m_baxCount;
                UranusControllerMonitor controllerMonitor(pNode->getController());
                controllerMonitor.getContainersSize(tempNodeInfo.proposeMsgNum, tempNodeInfo.echoMsgnum, tempNodeInfo.proposeMsgCacheSize,
                                                    tempNodeInfo.echoMsgCacheSize, tempNodeInfo.allPhaseEchoMsgNum);
            }
            return tempNodeInfo;
        }

        periodic_reort_data getReortData() const {
            periodic_reort_data reportData;
            std::shared_ptr<UranusNode> pNode = m_pNode.lock();
            if (pNode) {
                reportData.phase             = phaseStr[static_cast<int32_t>(pNode->m_phase)];
                reportData.baxCount          = pNode->m_baxCount;
                reportData.syncing           = pNode->m_syncing;
                reportData.syncFailed        = pNode->m_syncFailed;
                reportData.connected         = pNode->m_connected;
                reportData.ready             = pNode->m_ready;
                reportData.nonProducingNode  = pNode->getNonProducingNode();
                reportData.genesisLeaderPk   = std::string(pNode->m_genesisLeaderPk);
                reportData.genesisLeaderSk   = std::string(pNode->m_genesisLeaderSk);
                reportData.publicKey         = std::string(pNode->m_publicKey);
                reportData.privateKey        = std::string(pNode->m_privateKey);

                const chain::controller &chain = appbase::app().get_plugin<chain_plugin>().chain();
                reportData.blockNum          = chain.head_block_num();
                reportData.blockHash         = chain.head_block_id().str();
                reportData.previousBlockHash = chain.head_block_state()->prev().str();
                reportData.transactionNum    = chain.head_block_state()->trxs.size();
                reportData.ba0BlockTime      = m_ba0BlockTime;
                reportData.ba1BlockTime      = m_ba1BlockTime;
            }

            return reportData;
        }

        void setCallbackInNode() {
            std::shared_ptr<UranusNode> pNode = m_pNode.lock();
            if (pNode) {
                pNode->ba0Callback = std::bind(&UranusNodeMonitor::ba0BlockProducingTime, this);
                pNode->ba1Callback = std::bind(&UranusNodeMonitor::ba1BlockProducingTime, this);
            }
        }

        void ba0BlockProducingTime() {
            m_ba0BlockTime = boost::posix_time::to_iso_string(boost::posix_time::second_clock::local_time());

            auto pos = m_ba0BlockTime.find('T');
            m_ba0BlockTime.replace(pos, 1, std::string(" "));
            m_ba0BlockTime.replace(pos + 3, 0, std::string(":"));
            m_ba0BlockTime.replace(pos + 6, 0, std::string(":"));
            m_ba0BlockTime.replace(pos - 2, 0, std::string("-"));
            m_ba0BlockTime.replace(pos - 4, 0, std::string("-"));
        }

        void ba1BlockProducingTime() {
            m_ba1BlockTime = boost::posix_time::to_iso_string(boost::posix_time::second_clock::local_time());

            auto pos = m_ba1BlockTime.find('T');  
            m_ba1BlockTime.replace(pos, 1, std::string(" "));
            m_ba1BlockTime.replace(pos + 3, 0, std::string(":"));
            m_ba1BlockTime.replace(pos + 6, 0, std::string(":"));
            m_ba1BlockTime.replace(pos - 2, 0, std::string("-"));
            m_ba1BlockTime.replace(pos - 4, 0, std::string("-"));
        }

    private:
        std::weak_ptr<UranusNode> m_pNode;
        std::string phaseStr[4];
        std::string m_ba0BlockTime;
        std::string m_ba1BlockTime;
    };
}

FC_REFLECT( ultrainio::periodic_reort_data, (nodeIp)(blockNum)(phase)(baxCount)(transactionNum)(syncing)(syncFailed)(connected)
                                            (ready)(nonProducingNode)(blockHash)(previousBlockHash)(ba0BlockTime)(ba1BlockTime)
                                            (genesisLeaderPk)(genesisLeaderSk)(publicKey)(privateKey))
