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
#include <core/Transaction.h>
#include <crypto/PrivateKey.h>
#include <crypto/PublicKey.h>
#include <crypto/Vrf.h>

#include <uranus/VoterSystem.h>

namespace ultrainio {

#define MOST_ATTACK_NUMBER_F       ((VoterSystem::VOTER_STAKES - 50) / 3)
#define THRESHOLD_SEND_ECHO        (MOST_ATTACK_NUMBER_F + 1)
#define THRESHOLD_NEXT_ROUND       (2 * MOST_ATTACK_NUMBER_F + 1)
#define THRESHOLD_SYNCING          (2 * MOST_ATTACK_NUMBER_F + 1)
#define INVALID_BLOCK_NUM          0xFFFFFFFF

#define CODE_EXEC_MAX_TIME_S       3
#define MAX_PROPOSE_TRX_COUNT      5000

    class UranusController;

    class UranusNode : public std::enable_shared_from_this<UranusNode> {
    public:
        static const int MAX_ROUND_SECONDS;
        static const int MAX_PHASE_SECONDS;
        static boost::chrono::system_clock::time_point GENESIS;

        // TODO(qinxiaofen) should be remove
        static uint8_t URANUS_PUBLIC_KEY[VRF_PUBLIC_KEY_LEN];
        static uint8_t URANUS_PRIVATE_KEY[VRF_PRIVATE_KEY_LEN];

        static std::shared_ptr<UranusNode> initAndGetInstance(boost::asio::io_service &ioservice);

        static std::shared_ptr<UranusNode> getInstance();

        void setNonProducingNode(bool);
        void setGlobalProducingNodeNumber(int32_t);

        uint32_t getBlockNum() const;

        uint32_t getBaxCount() const;

        bool getSyncingStatus() const;

        void init();

        bool initKeyPair(const std::string& privateKeyHexStr, const std::string& publicKeyHexStr);

        void readyToJoin();

        void readyToConnect();

        void readyLoop(uint32_t timeout);

        bool startup();

        void run();

        void join();

        void reset();

        void sendMessage(const EchoMsg &echo);

        void sendMessage(const ProposeMsg &propose);

        void sendMessage(const std::string &peer_addr, const Block &msg);

        bool sendMessage(const SyncRequestMessage &msg);

        void sendMessage(const std::string &peer_addr, const RspLastBlockNumMsg &msg);

        void sendMessage(const AggEchoMsg& aggEchoMsg);

        ConsensusPhase getPhase() const;

        void ba0Process();

        uint32_t isSyncing();

        void ba1Process();

        void baxProcess();

        void ba0Loop(uint32_t timeout);

        void ba1Loop(uint32_t timeout);

        void baxLoop(uint32_t timeout);

        bool handleMessage(const EchoMsg &echo);

        bool handleMessage(const ProposeMsg &propose);

        bool handleMessage(const std::string &peer_addr, const SyncRequestMessage &msg);

        bool handleMessage(const std::string &peer_addr, const ReqLastBlockNumMsg &msg);

        bool handleMessage(const Block &block, bool last_block);

        uint32_t getLastBlocknum();

        bool syncFail();

        void cancelTimer();

        void applyBlockLoop(uint32_t timeout);

        int getStakes(const std::string &pk);

        ultrainio::chain::block_id_type getPreviousHash();

        bool isProcessNow();

        const std::shared_ptr<UranusController> getController() const;

        // TODO(qinxiaofen) may be not secure
        PrivateKey getPrivateKey() const;

        PublicKey getPublicKey() const;

    private:
        explicit UranusNode(boost::asio::io_service &ioservice);

        bool isBlank(const BlockHeader& blockHeader);

        bool isEmpty(const BlockHeader& blockHeader);

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
        // 6 seems to be a good number :)
        int32_t m_globalProducingNodeNumber = 6;
        ConsensusPhase m_phase;
        uint32_t m_baxCount;
        boost::asio::deadline_timer m_timer;
        boost::asio::deadline_timer m_preRunTimer;
        std::shared_ptr<UranusController> m_controllerPtr;
        PrivateKey m_privateKey;
        friend class UranusNodeMonitor;
    };
}
