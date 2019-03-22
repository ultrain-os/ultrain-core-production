#pragma once

#include <cstdint>

#include <map>
#include <memory>
#include <string>
#include <vector>

#include <ultrainio/chain/callback.hpp>
#include <ultrainio/chain/transaction_metadata.hpp>
#include <ultrainio/chain/types.hpp>

#include <core/Message.h>
#include <core/types.h>
#include <core/BlsVoterSet.h>
#include <boost/asio/steady_timer.hpp>

#include <rpos/VoterSet.h>

namespace ultrainio {
    class UranusNode;

    struct echo_message_info {
        CommonEchoMsg echoCommonPart;
        std::vector<AccountName> accountPool;
        std::vector<std::string> sigPool;
        std::vector<std::string> blsSignPool;
        std::vector<uint32_t>    timePool;
#ifdef CONSENSUS_VRF
        std::vector<std::string> proofPool;
        int totalVoter;
#endif
        bool hasSend;

        echo_message_info() :
                echoCommonPart(), accountPool(), sigPool(), timePool(),
#ifdef CONSENSUS_VRF
                proofPool(), totalVoter(0),
#endif
                hasSend(false) {}

        bool empty() {
            return accountPool.empty();
        }

        int getTotalVoterWeight() const {
#ifdef CONSENSUS_VRF
            return totalVoter;
#else
            return accountPool.size();
#endif
        }
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

    struct SyncTask {
        uint32_t checkBlock;
        BlsVoterSet bvs;
        fc::sha256 nodeId;
        uint32_t startBlock;
        uint32_t endBlock;
        uint32_t seqNum;

        SyncTask(uint32_t _checkBlock, const BlsVoterSet &_bvs, const fc::sha256 &_nodeId, uint32_t _startBlock, uint32_t _endBlock, uint32_t _seqNum)
                : bvs{_bvs}, nodeId(_nodeId) {
            checkBlock = _checkBlock;
            startBlock = _startBlock;
            endBlock = _endBlock;
            seqNum = _seqNum;
        }
    };

    typedef std::map<chain::block_id_type, echo_message_info> echo_msg_buff;

    class LightClientProducer;

class Scheduler : public std::enable_shared_from_this<Scheduler>, public ultrainio::chain::callback {
    public:
        ~Scheduler();
        Scheduler();

        void reset();

        bool isMinPropose(const ProposeMsg &propose_msg);
        bool isMin2FEcho(int totalVoterWeight, uint32_t phasecnt);
        bool isMinFEcho(const echo_message_info &info);
        bool isMinFEcho(const echo_message_info &info, const echo_msg_buff &msgbuff);
        bool isMinEcho(const echo_message_info &info);
        bool isMinEcho(const echo_message_info &info, const echo_msg_buff &msgbuff);
        Block produceTentativeBlock();
        bool initProposeMsg(ProposeMsg *propose_msg);

        bool isLaterMsg(const EchoMsg &echo);

        bool isLaterMsg(const ProposeMsg &propose);

        bool isLaterMsgAndCache(const EchoMsg &echo, bool &duplicate);

        bool isLaterMsgAndCache(const ProposeMsg &propose, bool &duplicate);

        bool isValid(const EchoMsg &echo);

        bool isValid(const ProposeMsg &propose);

        bool isBroadcast(const EchoMsg &echo);

        bool isBroadcast(const ProposeMsg &propose);

        bool insert(const EchoMsg &echo);

        bool insert(const ProposeMsg &propose);

        bool handleMessage(const EchoMsg &echo);

        bool handleMessage(const ProposeMsg &propose);

        bool handleMessage(const fc::sha256 &nodeId, const ReqSyncMsg &msg);

        bool handleMessage(const fc::sha256 &nodeId, const ReqBlockNumRangeMsg &msg);

        bool handleMessage(const Block &msg);

        bool handleMessage(const fc::sha256 &nodeId, const SyncStopMsg &msg);

        void resetEcho();

        void processCache(const msgkey &msg_key);

        void produceBlock(const chain::signed_block_ptr &block, bool force_push_whole_block = false);
        void clearTrxQueue();

        void init();

        ultrainio::chain::block_id_type getPreviousBlockhash();

        void moveEchoMsg2AllPhaseMap();

        const Block* getBa0Block();

        Block produceBaxBlock();

        bool isBeforeMsg(const EchoMsg &echo);

        bool processBeforeMsg(const EchoMsg &echo);

        bool isNeedSync();

        bool isChangePhase();

        bool fastHandleMessage(const ProposeMsg &propose);

        bool fastHandleMessage(const EchoMsg &echo);

        void fastProcessCache(const msgkey &msg_key);

        bool findEchoCache(const msgkey &msg_key);

        bool isFastba0(const msgkey &msg_key);

        bool findProposeCache(const msgkey &msg_key);

        uint32_t getLastBlocknum();

        void startSyncTaskTimer();

        void processSyncTask();

        template<class T>
        void clearMsgCache(T &cache, uint32_t blockNum);

        bool preRunBa0BlockStart();

        bool preRunBa0BlockStep();

        bool verifyBa0Block();

        void clearPreRunStatus();

        uint32_t getFastTimestamp();

        void resetTimestamp();

        void clearOldCachedProposeMsg();

        void clearOldCachedEchoMsg();

        // is invalid block
        bool isBlank(const BlockIdType& blockId);

        // is block without trxs
        bool isEmpty(const BlockIdType& blockId);

        Block blankBlock();

        Block emptyBlock();

        void setBa0Block(const Block& block);

        void insertAccount(echo_message_info &info, const EchoMsg &echo);

        void enableEventRegister(bool v);

        // implement callback
        bool on_accept_block_header(uint64_t chainName, const BlockHeader& blockHeader, BlockIdType& id);

        bool on_set_master_start_point(uint64_t chain_name, const std::string& committeeSetStr, const BlockIdType& blockId);
    private:

        bool getUnconfirmedHeaderFromDb(const chain::name&, std::vector<BlockHeader>& unconfirmedBlockHeader, BlockIdType& confirmedBlockId);

        // This function is time consuming, please cache the result empty block.
        std::shared_ptr<Block> generateEmptyBlock();

        bool isDuplicate(const ProposeMsg& proposeMsg);

        bool updateAndMayResponse(echo_message_info &info, const EchoMsg &echo, bool response);

        size_t runPendingTrxs(std::list<chain::transaction_metadata_ptr> *trxs,
                              fc::time_point hard_cpu_deadline, fc::time_point block_time);

        size_t runScheduledTrxs(std::vector<chain::transaction_id_type> &trxs,
                                fc::time_point hard_cpu_deadline,
                                fc::time_point block_time);

        size_t runUnappliedTrxs(std::vector<chain::transaction_metadata_ptr> &trxs,
                                fc::time_point hard_cpu_deadline, fc::time_point block_time);

        echo_message_info findEchoMsg(BlockIdType blockId);

        void start_memleak_check();

        chain::checksum256_type getCommitteeMroot(uint32_t block_num);

        bool hasMultiSignPropose(const ProposeMsg& propose);

        bool hasMultiVotePropose(const EchoMsg& echo);

        // data member
        Block m_ba0Block;
        BlockIdType m_ba0VerifiedBlkId = BlockIdType();
        BlockIdType m_ba0FailedBlkId = BlockIdType();
        bool m_voterPreRunBa0InProgress = false;
        int m_currentPreRunBa0TrxIndex = -1;
        int m_initTrxCount = 0;
        std::map<chain::block_id_type, ProposeMsg> m_proposerMsgMap;
        std::map<chain::block_id_type, echo_message_info> m_echoMsgMap;
        std::map<AccountName, chain::block_id_type> m_committeeVoteBlock;
        std::map<msgkey, std::vector<ProposeMsg>> m_cacheProposeMsgMap;
        std::map<msgkey, std::vector<EchoMsg>> m_cacheEchoMsgMap;
        std::map<msgkey, echo_msg_buff> m_echoMsgAllPhase;
        const uint32_t m_maxCacheEcho = 200;
        const uint32_t m_maxCachePropose = 100;
        const uint32_t m_maxCommitteeSize = 1000; //This is not strict, just to limit cache size.
        const uint32_t m_maxCachedAllPhaseKeys = 200;
        const uint32_t m_maxSyncClients = 10;
        const uint32_t m_maxPacketsOnce = 80;
        const uint32_t m_maxSyncBlocks = 1000;
        boost::asio::steady_timer::duration m_syncTaskPeriod{std::chrono::seconds{2}};
        std::unique_ptr<boost::asio::steady_timer> m_syncTaskTimer;
        boost::asio::steady_timer::duration m_memleakCheckPeriod{std::chrono::seconds{10}};
        std::unique_ptr<boost::asio::steady_timer> m_memleakCheck;
        std::list<SyncTask> m_syncTaskQueue;
        uint32_t m_fast_timestamp;
        VoterSet m_currentVoterSet;
        std::shared_ptr<LightClientProducer> m_lightClientProducer;
        friend class UranusControllerMonitor;
    };
}
