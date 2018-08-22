/**
 *  @file
 *  @copyright defined in ultrain-core-for-open-source/LICENSE.txt
 */
#pragma once

#include <memory>

#include "core/Message.h"

namespace ultrainio {
    class NetManager {
    public:
        static std::shared_ptr<NetManager> getInstance();
        void broadcast(const ProposeMsg& propose);
        void broadcast(const EchoMsg& echo);
        void sendBlock(const std::string& addr, const Block& block);
        bool sendApply(const SyncRequestMessage& msg);
        void sendLastBlockNum(const std::string& addr, const RspLastBlockNumMsg& last_block_num);
        void stopSyncBlock();

        bool handleMessage(const EchoMsg& echo);
        bool handleMessage(const ProposeMsg& propose);
        bool handleMessage(const std::string& peerAddr, const SyncRequestMessage& msg);
        bool handleMessage(const std::string& peerAddr, const ReqLastBlockNumMsg& msg);
        bool handleMessage(const Block& block, bool lastBlock);

    private:
        NetManager(const NetManager& rhs) = delete;
        NetManager&operator = (const NetManager& rhs) = delete;

        static std::shared_ptr<NetManager> s_self;
    };
}