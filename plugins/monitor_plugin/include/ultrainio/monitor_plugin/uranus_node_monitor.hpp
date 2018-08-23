#pragma once
#include <uranus/Node.h>
#include "uranus_controller_monitor.hpp"

namespace ultrainio {

    struct UranusNodeInfo{
        bool ready;
        bool connected;
        bool syncing;
        bool syncFailed;
        bool isNonProducingNode;
        int32_t globalProducingNodeNumber;
        int32_t phase;
        uint32_t baxCount;
        uint32_t proposeMsgNum;
        uint32_t echoMsgnum;
        uint32_t proposeMsgCacheSize;
        uint32_t echoMsgCacheSize;
        uint32_t allPhaseEchoMsgNum;
    };

    class UranusNodeMonitor
    {
    public:
        UranusNodeMonitor(std::weak_ptr<UranusNode> pNode):m_pNode(pNode) {}

        ~UranusNodeMonitor() = default;
        UranusNodeMonitor(const UranusNodeMonitor& ) = delete;
        const UranusNodeMonitor& operator=(const UranusNodeMonitor&) = delete;
       
        UranusNodeInfo getNodeInfo() const{
            std::shared_ptr<UranusNode> pNode = m_pNode.lock(); 
            UranusNodeInfo tempNodeInfo;
            tempNodeInfo.ready = pNode->m_ready;
            tempNodeInfo.connected = pNode->m_connected;
            tempNodeInfo.syncing = pNode->m_syncing;
            tempNodeInfo.syncFailed = pNode->m_syncFailed;
            tempNodeInfo.isNonProducingNode = pNode->m_isNonProducingNode;
            tempNodeInfo.globalProducingNodeNumber = pNode->m_globalProducingNodeNumber;
            tempNodeInfo.phase = static_cast<int32_t>(pNode->m_phase);
            tempNodeInfo.baxCount = pNode->m_baxCount;
            UranusControllerMonitor controllerMonitor(pNode->getController());
            controllerMonitor.getContainersSize(tempNodeInfo.proposeMsgNum, tempNodeInfo.echoMsgnum, tempNodeInfo.proposeMsgCacheSize,
                                                 tempNodeInfo.echoMsgCacheSize, tempNodeInfo.allPhaseEchoMsgNum);
            return tempNodeInfo;
        }

    private:
        std::weak_ptr<UranusNode> m_pNode;
    };
}
