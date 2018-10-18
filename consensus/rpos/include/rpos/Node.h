#pragma once

#include <chrono>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

#include <boost/chrono/include.hpp>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <core/Message.h>
#include <core/Redefined.h>
#include <crypto/Ed25519.h>
#include <crypto/PrivateKey.h>
#include <crypto/PublicKey.h>
#include <rpos/StakeVote.h>

namespace ultrainio {
    class Scheduler;
    class NodeInfo;

    // used for monitor to record block producing time
    typedef std::function<void ()> monitorCallback;

    class UranusNode : public std::enable_shared_from_this<UranusNode> {
    public:
        static std::shared_ptr<UranusNode> initAndGetInstance(boost::asio::io_service &ioservice);

        static std::shared_ptr<UranusNode> getInstance();

        void setMyInfoAsCommitteeKey(const std::string& sk, const std::string& account);

        void setNonProducingNode(bool);

        bool getNonProducingNode() const;

        uint32_t getBlockNum() const;

        uint32_t getBaxCount() const;

        bool getSyncingStatus() const;

        void init();

        void readyToJoin();

        void readyToConnect();

        void readyLoop(uint32_t timeout);

        void run(bool voteFlag = true);

        void join();

        void reset();

        void sendMessage(const EchoMsg &echo);

        void sendMessage(const ProposeMsg &propose);

        void sendMessage(const std::string &peer_addr, const SyncBlockMsg &msg);

        bool sendMessage(const ReqSyncMsg &msg);

        void sendMessage(const std::string &peer_addr, const RspLastBlockNumMsg &msg);

        void sendMessage(const AggEchoMsg& aggEchoMsg);

        bool isFastBlock();

        ConsensusPhase getPhase() const;

        void ba0Process();

        uint32_t isSyncing();

        bool isReady() {return m_ready;}

        void ba1Process();

        void baxProcess();

        void runLoop(uint32_t timeout);

        void ba0Loop(uint32_t timeout);

        void ba1Loop(uint32_t timeout);

        void baxLoop(uint32_t timeout);

        bool handleMessage(const EchoMsg &echo);

        bool handleMessage(const ProposeMsg &propose);

        bool handleMessage(const std::string &peer_addr, const ReqSyncMsg &msg);

        bool handleMessage(const std::string &peer_addr, const ReqLastBlockNumMsg &msg);

        bool handleMessage(const Block &block, bool last_block);

        bool handleMessage(const std::string &peer_addr, const SyncStopMsg &msg);

        uint32_t getLastBlocknum();

        bool syncFail(const ultrainio::ReqSyncMsg& sync_msg);

        bool syncCancel();

        void cancelTimer();

        void applyBlockLoop(uint32_t timeout);

        ultrainio::chain::block_id_type getPreviousHash();

        bool isProcessNow();

        const std::shared_ptr<Scheduler> getScheduler() const;

        uint32_t getRoundCount();

        int getCommitteeMemberNumber();

        void vote(uint32_t blockNum, ConsensusPhase phase, uint32_t baxCount);

        void setGenesisTime(const boost::chrono::system_clock::time_point& tp);

        void setGenesisStartupTime(int32_t minutes);

        void setGenesisPk(const std::string& pk);

        void setRoundAndPhaseSecond(int32_t roundSecond, int32_t phaseSecond);

    private:
        explicit UranusNode(boost::asio::io_service &ioservice);

        bool isBlank(const BlockIdType& blockId);

        bool isEmpty(const BlockIdType& blockId);

        void sendEchoForEmptyBlock();

        void applyBlock();

        void applyBlockOnce();

        void applyBlock(bool once);

        void fastBlock(uint32_t blockNum);

        void fastBa0();

        void fastBa1();

        void fastBax();

        void preRunBa0BlockLoop(uint32_t timeout);

        void preRunBa0BlockStep();

        uint32_t getRoundInterval();

        static std::shared_ptr<UranusNode> s_self;
        bool m_ready;
        bool m_connected;
        bool m_syncing;
        bool m_syncFailed;
        bool m_isNonProducingNode = false;
        bool m_timerCanceled = false;
        ConsensusPhase m_phase;
        uint32_t m_baxCount;
        boost::asio::deadline_timer m_timer;
        boost::asio::deadline_timer m_preRunTimer;
        std::shared_ptr<Scheduler> m_schedulerPtr;
        friend class UranusNodeMonitor;
        monitorCallback ba0Callback = nullptr;
        monitorCallback ba1Callback = nullptr;
    };
}
