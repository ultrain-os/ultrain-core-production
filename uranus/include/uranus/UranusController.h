#pragma once

#include <cstdint>

#include <map>
#include <memory>
#include <string>
#include <vector>

#include <ultrainio/chain/transaction_metadata.hpp>
#include <ultrainio/chain/types.hpp>

#include <core/Message.h>
#include <core/Transaction.h>
#include <crypto/Vrf.h>
#include <boost/asio/steady_timer.hpp>

namespace ultrainio {

    class UranusNode;

    struct echo_message_info {
        EchoMsg echo;
        std::vector<std::string> pk_pool; //public key pool
        int totalVoter;
        bool hasSend;

        echo_message_info() : echo(), pk_pool(), totalVoter(0), hasSend(false) {}
    };

    struct msgkey {
        uint32_t blockNum;
        uint16_t phase;

        bool operator<(const msgkey &other) const {
            if (blockNum < other.blockNum) {
                return true;
            } else if (blockNum == other.blockNum) {
                return phase < other.phase;
            }
            return false;
        }
    };

////////used for monitor Begin/////////////
    struct BlockHeaderDigest {
        chain::block_timestamp_type      timestamp;
        std::string                      proposerPk;
        chain::block_id_type             previous;
        chain::block_id_type             myid;
        uint32_t                         blockNum;

        void digestFromBlockHeader(const chain::signed_block_header& block_header) {
            timestamp  = block_header.timestamp;
            proposerPk = block_header.proposerPk;
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
        uint32_t                  pkPoolSize;
        std::vector<std::string>  pk_pool; //public key pool

        void digestFromeEchoMsgInfo(const echo_message_info& echo_msg_info) {
            echoMsg.digestFromeEchoMsg(echo_msg_info.echo);
            hasSend = echo_msg_info.hasSend;
            pkPoolSize = echo_msg_info.pk_pool.size();
            pk_pool.assign(echo_msg_info.pk_pool.begin(), echo_msg_info.pk_pool.end());
        }
    };

    ////////used for monitor End////////

    struct SyncTask {
        std::string peerAddr;
        uint32_t startBlock;
        uint32_t endBlock;

        SyncTask(const std::string &_peerAddr, size_t _startBlock, uint32_t _endBlock) : peerAddr(_peerAddr) {
            startBlock = _startBlock;
            endBlock = _endBlock;
        }
    };

    typedef std::map<chain::block_id_type, echo_message_info> echo_msg_buff;

    typedef std::vector<std::pair<chain::block_id_type, EchoMsgInfoDigest>> echo_msg_digest_vect;

    class UranusController : public std::enable_shared_from_this<UranusController> {
    public:
        UranusController();

        EchoMsg constructMsg(const Block &block);

        EchoMsg constructMsg(const ProposeMsg &propose);

        EchoMsg constructMsg(const EchoMsg &echo);

        void reset();

        bool isMinPropose(const ProposeMsg &propose_msg);

        bool isMinFEcho(const echo_message_info &info);

        bool isMinEcho(const echo_message_info &info);

        Block produceTentativeBlock();

        bool initProposeMsg(ProposeMsg *propose_msg);

        bool isLaterMsg(const EchoMsg &echo);

        bool isLaterMsg(const ProposeMsg &propose);

        bool isLaterMsgAndCache(const EchoMsg &echo, bool &duplicate);

        bool isLaterMsgAndCache(const ProposeMsg &propose, bool &duplicate);

        bool isValid(const EchoMsg &echo);

        bool isValid(const ProposeMsg &propose);

        bool insert(const EchoMsg &echo);

        bool insert(const ProposeMsg &propose);

        bool handleMessage(const EchoMsg &echo);

        bool handleMessage(const ProposeMsg &propose);

        bool handleMessage(const std::string &peer_addr, const SyncRequestMessage &msg);

        bool handleMessage(const std::string &peer_addr, const ReqLastBlockNumMsg &msg);

        bool handleMessage(const Block &msg);

        void resetEcho();

        void processCache(const msgkey &msg_key);

        void produceBlock(const chain::signed_block_ptr &block, bool force_push_whole_block = false);

        void init();

        ultrainio::chain::block_id_type getPreviousBlockhash();

        void saveEchoMsg();

        const Block* getBa0Block();

        Block produceBaxBlock();

        bool isBeforeMsg(const EchoMsg &echo);

        bool processEchoMsg(const EchoMsg &echo);

        bool isBeforeMsgAndProcess(const EchoMsg &echo);

        uint32_t isSyncing();

        bool fastHandleMessage(const ProposeMsg &propose);

        bool fastHandleMessage(const EchoMsg &echo);

        void fastProcessCache(const msgkey &msg_key);

        bool findEchoCache(const msgkey &msg_key);

        uint32_t getLastBlocknum();

        void startSyncTaskTimer();

        void processSyncTask();

        template<class T>
        void clearMsgCache(T cache, uint32_t blockNum);

        bool preRunBa0BlockStart();

        bool preRunBa0BlockStep();

        bool verifyBa0Block();

        void clearPreRunStatus();

        fc::time_point getFastTimestamp();

        void resetTimestamp();

        void clearOldCachedProposeMsg();

        void clearOldCachedEchoMsg();

        void clearOldCachedAllPhaseMsg();

        void getContainersSize(uint32_t& proposeNum, uint32_t& echoNum, uint32_t& proposeCacheSize, uint32_t& echoCacheSize, uint32_t& allPhaseEchoNum) const;

        BlockHeaderDigest findProposeMsgByBlockId(const chain::block_id_type& bid) const;

        EchoMsgInfoDigest findEchoMsgByBlockId(const chain::block_id_type& bid) const;

        std::vector<BlockHeaderDigest> findProposeCacheByKey(const msgkey& msg_key) const;

        std::vector<EchoMsgDigest> findEchoCacheByKey(const msgkey& msg_key) const;

        echo_msg_digest_vect findEchoApMsgByKey(const msgkey& msg_key) const;

        bool isBlank(const BlockHeader& blockHeader);

        bool isEmpty(const BlockHeader& blockHeader);

        Block blankBlock();

        Block emptyBlock();

        void setBa0Block(const Block& block);
    private:
        // This function is time consuming, please cache the result empty block.
        std::shared_ptr<Block> generateEmptyBlock();

        bool updateAndMayResponse(echo_message_info &info, const EchoMsg &echo, bool response);

        size_t runPendingTrxs(std::list<chain::transaction_metadata_ptr> *trxs,
                              fc::time_point start_timesamp);

        size_t runUnappliedTrxs(const std::vector<chain::transaction_metadata_ptr> &trxs,
                                fc::time_point start_timesamp);

        std::string signature(const EchoMsg &echo);

        Block m_ba0Block;
        bool m_voterPreRunBa0InProgress = false;
        int m_currentPreRunBa0TrxIndex = -1;
        int m_initTrxCount = 0;
        std::map<chain::block_id_type, ProposeMsg> m_proposerMsgMap;
        std::map<chain::block_id_type, echo_message_info> m_echoMsgMap;
        std::map<msgkey, std::vector<ProposeMsg>> m_cacheProposeMsgMap;
        std::map<msgkey, std::vector<EchoMsg>> m_cacheEchoMsgMap;
        std::map<msgkey, echo_msg_buff> m_echoMsgAllPhase;
        const uint32_t m_maxCachedKeys = 360;
        const uint32_t m_maxCachedAllPhaseKeys = 100;
        const uint32_t m_maxSyncClients = 10;
        const uint32_t m_maxPacketsOnce = 80;
        boost::asio::steady_timer::duration m_syncTaskPeriod{std::chrono::seconds{1}};
        std::unique_ptr<boost::asio::steady_timer> m_syncTaskTimer;
        std::list<SyncTask> m_syncTaskQueue;
        fc::time_point m_fast_timestamp;
    };
}
