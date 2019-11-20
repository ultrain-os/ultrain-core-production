#pragma once

#include <chrono>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

#include <boost/chrono/include.hpp>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/signals2/signal.hpp>

#include <core/Message.h>
#include <core/types.h>
#include <crypto/Ed25519.h>
#include <crypto/PrivateKey.h>
#include <crypto/PublicKey.h>

namespace ultrainio {
    class Scheduler;
    class NodeInfo;
    class StakeVoteBase;

    enum TimerHandlerNumber {
        THN_READY = 0,
        THN_RUN = 1,
        THN_BA0 = 2,
        THN_BA1 = 3,
        THN_BAX = 4,
        THN_SYNC_BLOCK = 5,
        THN_FAST_CHECK = 6,
        THN_MAX = 31
    };

    // used for monitor to record block producing time
    typedef std::function<void ()> monitorCallback;
    typedef std::function<void (bool)> monitorSetCallback;

    typedef boost::signals2::signal<void(const std::string&, uint32_t, const std::string&, const std::string&)> ReportHandler;

    class Node : public std::enable_shared_from_this<Node> {
    public:
        static std::shared_ptr<Node> initAndGetInstance(boost::asio::io_service& ioservice);

        static std::shared_ptr<Node> getInstance();

        void setCommitteeInfo(const std::string& account, const std::string& sk, const std::string& blsSk, const std::string& accountSk);

        void setNonProducingNode(bool);

        bool getNonProducingNode() const;

        uint32_t getBlockNum() const;

        uint32_t getBaxCount() const;

        bool isSyncing() const;

        void init();

        void readyToJoin();

        bool syncFail(const ultrainio::ReqSyncMsg& sync_msg);

        bool syncCancel();

        void run();

        void reset();

        void sendMessage(const EchoMsg& echo);

        void sendMessage(const ProposeMsg& propose);

        void sendMessage(const fc::sha256& nodeId, const SyncBlockMsg& msg);

        bool sendMessage(const ReqSyncMsg& msg);

        void sendMessage(const fc::sha256& nodeId, const RspBlockNumRangeMsg& msg);

        ConsensusPhase getPhase() const;

        bool handleMessage(const EchoMsg& echo);

        bool handleMessage(const ProposeMsg& propose);

        bool handleMessage(const fc::sha256& nodeId, const ReqSyncMsg& msg);

        bool handleMessage(const fc::sha256& nodeId, const ReqBlockNumRangeMsg& msg);

        bool handleMessage(const SyncBlockMsg& msg, bool last_block, bool safe);

        bool handleMessage(const fc::sha256& nodeId, const SyncStopMsg& msg);

        BlockIdType getPreviousHash();

        const std::shared_ptr<Scheduler> getScheduler() const;

        uint32_t getRoundCount();

        int getCommitteeMemberNumber();

        void setGenesisTime(const fc::time_point& tp);

        void setGenesisStartupTime(int32_t minutes);

        void setGenesisPk(const std::string& pk);

        void setRoundAndPhaseSecond(int32_t roundSecond, int32_t phaseSecond);

        void setTrxsSecond(int32_t trxssecond);

        void setSyncFailBlockHeight();

        void setAllowReportEvil(bool v);

        ReportHandler& getEvilReportHandler();

        ReportHandler& getEmptyBlockReportHandler();

        ReportHandler& getMaxBaxCountReportHandler();

    private:
        explicit Node(boost::asio::io_service& ioservice);

        void runLoop(uint32_t timeout);

        void ba0Loop(uint32_t timeout);

        void ba1Loop(uint32_t timeout);

        void fastLoop(uint32_t timeout);

        void baxLoop(uint32_t timeout);

        void fastProcess();

        bool isNeedSync();

        void ba0Process();

        void ba1Process();

        void baxProcess();

        bool isProcessNow();

        bool canEnterFastBlockMode();

        void doFastBlock();

        void syncBlockLoop(uint32_t timeout);

        uint32_t getLastBlocknum();

        void vote(uint32_t blockNum, ConsensusPhase phase, uint32_t baxCount);

        void join();

        bool isBlank(const BlockIdType& blockId);

        bool isEmpty(const BlockIdType& blockId);

        void sendEchoForEmptyBlock();

        void syncBlock(bool once = false);

        void fastBa0();

        void fastBa1();

        void fastBax();

        void cancelTimer();

        void readyToConnect();

        void readyLoop(uint32_t timeout);

        void preRunBa0BlockLoop(uint32_t timeout);

        void preRunBa0BlockStep();

        uint32_t getLeftTime();

        void setTimerCanceled(TimerHandlerNumber thn);

        void resetTimerCanceled(TimerHandlerNumber thn);

        bool isTimerCanceled(TimerHandlerNumber thn) const;

        bool isListener(uint32_t blockNum, ConsensusPhase phase, uint32_t baxCount);

        static std::shared_ptr<Node> s_self;

        bool m_ready;
        bool m_connected;
        bool m_syncing;
        // used by monitor
        bool m_syncFailed;
        int syncFailed_blockheight = 0;
        bool m_isNonProducingNode = false;
        TimerHandlerNumber m_currentTimerHandlerNo = THN_MAX;
        uint32_t m_timerCanceledBits = 0;
        ConsensusPhase m_phase;
        uint32_t m_baxCount;
        boost::asio::deadline_timer m_timer;
        boost::asio::deadline_timer m_preRunTimer;
        std::shared_ptr<Scheduler> m_schedulerPtr;
        ReportHandler m_evilReportHandler;
        ReportHandler m_emptyBlockReportHandler;
        ReportHandler m_maxBaxCountReportHandler;
        friend class UranusNodeMonitor;
        monitorCallback ba0Callback = nullptr;
        monitorCallback ba1Callback = nullptr;
        monitorSetCallback setIsProposer = nullptr;
    };
}
