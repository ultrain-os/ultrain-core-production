#pragma once

#include "uranus_controller_monitor.hpp"
#include "runtime_os_info.hpp"
#include <rpos/Genesis.h>
#include <rpos/Node.h>
#include <rpos/NodeInfo.h>
#include <rpos/StakeVote.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <ultrainio/net_plugin/net_plugin.hpp>

namespace ultrainio {

    extern char version[];

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

    struct periodic_report_dynamic_data {
        std::string  nodeIp;
        uint32_t     blockNum;
        std::string  phase;
        uint32_t     baxCount;
        uint32_t     transactionNum;
        std::string  blockProposer;
        bool         syncing;
        bool         syncFailed;
        bool         connected;
        bool         ready;
        std::string  blockHash;
        std::string  previousBlockHash;
        std::string  ba0BlockTime;
        std::string  ba1BlockTime;
        float        cpu;
        uint32_t     memory;          // kB
        uint32_t     virtualMemory;   // kB
        std::vector<string> activePeers;
    };

     struct periodic_report_static_data {
        std::string  nodeIp;
        std::string  version;
        bool         nonProducingNode;
        std::string  genesisLeaderPk;
        std::string  publicKey;
        std::string  privateKey;
        std::string  account;
        std::vector<string> configuredPeers;
    };

    class UranusNodeMonitor
    {
    public:
        UranusNodeMonitor(std::weak_ptr<UranusNode> pNode):m_pNode(pNode) {
            phaseStr[0] = "kPhaseInit";
            phaseStr[1] = "kPhaseBA0";
            phaseStr[2] = "kPhaseBA1";
            phaseStr[3] = "kPhaseBAX";
            setCallbackInNode();
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

        periodic_report_dynamic_data getReortData() {
            periodic_report_dynamic_data reportData;
            std::shared_ptr<UranusNode> pNode = m_pNode.lock();
            if (pNode) {
                reportData.phase             = phaseStr[static_cast<int32_t>(pNode->m_phase)];
                reportData.baxCount          = pNode->m_baxCount;
                reportData.syncing           = pNode->m_syncing;
                reportData.syncFailed        = pNode->m_syncFailed;
                reportData.connected         = pNode->m_connected;
                reportData.ready             = pNode->m_ready;

                const chain::controller &chain = appbase::app().get_plugin<chain_plugin>().chain();
                reportData.blockNum          = chain.head_block_num();
                reportData.blockHash         = chain.head_block_id().str();
                reportData.previousBlockHash = chain.head_block_state()->prev().str();
                reportData.transactionNum    = chain.head_block_state()->block->transactions.size(); //trxs.size();
                reportData.blockProposer     = std::string(chain.head_block_state()->block->proposer);
                reportData.ba0BlockTime      = m_ba0BlockTime;
                reportData.ba1BlockTime      = m_ba1BlockTime;
                reportData.memory            = m_perfMonitor.get_proc_mem();
                reportData.virtualMemory     = m_perfMonitor.get_proc_virtualmem();

                uint64_t currentTotalCpu     = m_perfMonitor.get_cpu_total_occupy();
                uint64_t currentMyCpu        = m_perfMonitor.get_cpu_proc_occupy();
                if(m_lastTotalCpu == 0 && m_lastMyCpu == 0) {
                    reportData.cpu = 0;
                }
                else {
                    if(currentTotalCpu - m_lastTotalCpu > 0) {
                        reportData.cpu = 100.0f * float(currentMyCpu - m_lastMyCpu) / float(currentTotalCpu - m_lastTotalCpu);
                    }
                    else {
                        reportData.cpu = 0;
                    }
                }
                m_lastTotalCpu = currentTotalCpu;
                m_lastMyCpu = currentMyCpu;

                vector<connection_status> connectionsStatus = appbase::app().get_plugin<net_plugin>().connections();
                for (const auto& connectStatus : connectionsStatus) {
                    if(!connectStatus.connecting) {   //only report connected peers
                        reportData.activePeers.push_back(connectStatus.peer);
                    }
                }
            }

            return reportData;
        }

        periodic_report_static_data getStaticConfigInfo() const {
            periodic_report_static_data staticConfig;
            std::shared_ptr<UranusNode> pNode = m_pNode.lock();
            if (pNode) {
                staticConfig.version           = version;
                staticConfig.nonProducingNode  = pNode->getNonProducingNode();
                staticConfig.genesisLeaderPk   = Genesis::s_genesisPk;
                staticConfig.publicKey         = std::string(StakeVote::getMyPrivateKey().getPublicKey());
                staticConfig.privateKey        = std::string(StakeVote::getMyPrivateKey());
                staticConfig.account           = std::string(StakeVote::getMyAccount());
            }
            return staticConfig;
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
        PerformanceMonitor m_perfMonitor;
        uint64_t    m_lastTotalCpu;
        uint64_t    m_lastMyCpu;
    };
}

FC_REFLECT( ultrainio::periodic_report_dynamic_data, (nodeIp)(blockNum)(phase)(baxCount)(transactionNum)(blockProposer)(syncing)
                                                     (syncFailed)(connected)(ready)(blockHash)(previousBlockHash)(ba0BlockTime)
                                                     (ba1BlockTime)(cpu)(memory)(virtualMemory)(activePeers) )
FC_REFLECT( ultrainio::periodic_report_static_data, (nodeIp)(version)(nonProducingNode)(genesisLeaderPk)(publicKey)
                                                    (privateKey)(account)(configuredPeers) )
