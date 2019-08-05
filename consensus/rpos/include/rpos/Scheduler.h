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

#include <rpos/EvilDDosDetector.h>
#include <rpos/EvilMultiSignDetector.h>
#include <rpos/RoundInfo.h>
#include <rpos/VoterSet.h>

namespace ultrainio {

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

    class LightClientProducer;
    class CommitteeSet;
    class UranusNode;

class Scheduler : public std::enable_shared_from_this<Scheduler> {
    public:
        typedef std::map<BlockIdType, VoterSet> BlockIdVoterSetMap;

        ~Scheduler();

        Scheduler();

        void reset();

        Block produceTentativeBlock();

        bool initProposeMsg(ProposeMsg& proposeMsg);

        bool insert(const ProposeMsg& propose);

        bool insert(const EchoMsg& echo);

        bool handleMessage(const EchoMsg &echo);

        bool handleMessage(const ProposeMsg &propose);

        bool handleMessage(const fc::sha256 &nodeId, const ReqSyncMsg &msg);

        bool handleMessage(const fc::sha256 &nodeId, const ReqBlockNumRangeMsg &msg);

        bool handleMessage(const SyncBlockMsg &msg, bool safe);

        bool handleMessage(const fc::sha256 &nodeId, const SyncStopMsg &msg);

        void resetEcho();

        void processCache(const RoundInfo& roundInfo);

        void produceBlock(const chain::signed_block_ptr &block, bool force_push_whole_block = false);
        void clearTrxQueue();

        void init();

        BlockIdType getPreviousBlockhash() const;

        void moveEchoMsg2AllPhaseMap();

        const Block* getBa0Block();

        Block produceBaxBlock();

        bool isNeedSync();

        bool isChangePhase();

        bool fastHandleMessage(const ProposeMsg &propose);

        bool fastHandleMessage(const EchoMsg &echo);

        void fastProcessCache(const RoundInfo& roundInfo);

        bool findEchoCache(const RoundInfo& roundInfo);

        bool isFastba0(const RoundInfo& roundInfo);

        bool findProposeCache(const RoundInfo& roundInfo);

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

        void insertAccount(VoterSet &info, const EchoMsg &echo);

        void enableEventRegister(bool v);

        bool setBlsVoterSet(const std::string& bls);

        bool verifyMyBlsSignature(const EchoMsg& echo) const;

        void invokeDeduceWhenBax();

    private:
        // This function is time consuming, please cache the result empty block.
        std::shared_ptr<Block> generateEmptyBlock();

        bool updateAndMayResponse(VoterSet &info, const EchoMsg &echo, bool response);

        size_t runPendingTrxs(std::list<chain::transaction_metadata_ptr> *trxs,
                              fc::time_point hard_cpu_deadline, fc::time_point block_time);

        size_t runScheduledTrxs(std::vector<chain::transaction_id_type> &trxs,
                                fc::time_point hard_cpu_deadline,
                                fc::time_point block_time);

        size_t runUnappliedTrxs(std::vector<chain::transaction_metadata_ptr> &trxs,
                                fc::time_point hard_cpu_deadline, fc::time_point block_time);

        void startMemleakCheck();

        chain::checksum256_type getCommitteeMroot(uint32_t block_num);

        BlsVoterSet toBlsVoterSetAndFindEvil(const VoterSet& voterSet, const CommitteeSet& committeeSet, bool genesisPeriod, int weight) const;

        // echo relative
        bool sameBlockNumButBeforePhase(const EchoMsg& echo) const;

        bool sameBlockNumAndPhase(const EchoMsg& echo) const;

        bool obsolete(const EchoMsg& echo) const;

        bool isLaterMsg(const EchoMsg &echo) const;

        bool processLaterMsg(const EchoMsg &echo);

        bool processBeforeMsg(const EchoMsg &echo);

        bool loopback(const EchoMsg& echo) const;

        bool duplicated(const EchoMsg& echo) const;

        bool isValid(const EchoMsg &echo) const;

        // propose relative
        bool loopback(const ProposeMsg& propose) const;

        bool obsolete(const ProposeMsg& propose) const;

        bool duplicated(const ProposeMsg& propose) const;

        bool isLaterMsg(const ProposeMsg& propose) const;

        bool processLaterMsg(const ProposeMsg& propose);

        bool sameBlockNumAndPhase(const ProposeMsg& propose) const;

        bool isValid(const ProposeMsg& propose) const;

        bool is2fEcho(const VoterSet& voterSet, uint32_t phaseCount) const;

        bool isMinFEcho(const VoterSet& voterSet, const BlockIdVoterSetMap& blockIdVoterSetMap) const;

        bool isMinFEcho(const VoterSet& voterSet) const;

        bool isMinEcho(const VoterSet& voterSet, const BlockIdVoterSetMap& blockIdVoterSetMap) const;

        bool isMinEcho(const VoterSet& voterSet) const;

        bool isMinPropose(const ProposeMsg& proposeMsg);

        bool theSameOne(const ultrainio::chain::signed_block& lhs, const ultrainio::chain::signed_block_ptr& rhs);

        // data member
        Block m_ba0Block;
        BlockIdType m_ba0VerifiedBlkId = BlockIdType();
        BlockIdType m_ba0FailedBlkId = BlockIdType();
        bool m_voterPreRunBa0InProgress = false;
        int m_currentPreRunBa0TrxIndex = -1;
        int m_initTrxCount = 0;
        std::map<chain::transaction_id_type, uint32_t>  m_blacklistTrx;
        std::map<BlockIdType, ProposeMsg> m_proposerMsgMap;
        BlockIdVoterSetMap m_echoMsgMap;
        std::map<RoundInfo, std::vector<ProposeMsg>> m_cacheProposeMsgMap;
        std::map<RoundInfo, std::vector<EchoMsg>> m_cacheEchoMsgMap;
        std::map<RoundInfo, BlockIdVoterSetMap> m_echoMsgAllPhase;
        const uint32_t m_maxCacheEcho = 200;
        const uint32_t m_maxCachePropose = 100;
        const uint32_t m_maxCommitteeSize = 1000; //This is not strict, just to limit cache size.
        const uint32_t m_maxCachedAllPhaseKeys = 200;
        const uint32_t m_maxSyncClients = 10;
        const uint32_t m_maxPacketsOnce = 80;
        const uint32_t m_maxSyncBlocks = 1000;
        uint32_t m_fastTimestamp = 0;
        boost::asio::steady_timer::duration m_syncTaskPeriod{std::chrono::seconds{2}};
        std::unique_ptr<boost::asio::steady_timer> m_syncTaskTimer;
        boost::asio::steady_timer::duration m_memleakCheckPeriod{std::chrono::seconds{10}};
        std::unique_ptr<boost::asio::steady_timer> m_memleakCheck;
        std::list<SyncTask> m_syncTaskQueue;
        BlsVoterSet m_currentBlsVoterSet;
        std::shared_ptr<LightClientProducer> m_lightClientProducer;
        EvilDDosDetector m_evilDDosDetector;
        EvilMultiSignDetector m_evilMultiSignDetector;
        friend class UranusControllerMonitor;
    };
}
