#include <rpos/Node.h>

#include <chrono>
#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include <fc/log/logger.hpp>

#include <ultrainio/chain/version.hpp>
#include <rpos/Config.h>
#include <rpos/Genesis.h>
#include <rpos/MsgBuilder.h>
#include <rpos/MsgMgr.h>
#include <rpos/Scheduler.h>
#include <rpos/NodeInfo.h>
#include <rpos/StakeVoteBase.h>
#include <rpos/Utils.h>

#include <appbase/application.hpp>
#include <ultrainio/net_plugin/net_plugin.hpp>
#include <ultrainio/kcp_plugin/kcp_plugin.hpp>
#include <ultrainio/utilities/common.hpp>

using namespace boost::asio;
using namespace std;

namespace ultrainio {
    using net_plugin = net_plugin_n::net_plugin;
    using kcp_plugin = kcp_plugin_n::kcp_plugin;

    // -1 not sure; 1 using net_plugin; 0 using kcp_plugin;
    static int sync_using_net_plugin = -1;

    void set_sync_using_net_plugin() {
        if (sync_using_net_plugin == -1) {
            if (app().find_plugin<net_plugin>()) {
                sync_using_net_plugin = app().get_plugin<net_plugin>().is_netplugin_prime();
            }
        }
    }

    void stop_sync_block() {
        if (app().find_plugin<net_plugin>()) {
            app().get_plugin<net_plugin>().stop_sync_block();
        }
        if (app().find_plugin<kcp_plugin>()) {
            app().get_plugin<kcp_plugin>().stop_sync_block();
        }
    }

    void send_sync_block(const fc::sha256 &nodeId, const SyncBlockMsg &msg) {
        if (app().find_plugin<net_plugin>()) {
            app().get_plugin<net_plugin>().send_block(nodeId, msg);
        }
        if (app().find_plugin<kcp_plugin>()) {
            app().get_plugin<kcp_plugin>().send_block(nodeId, msg);
        }
    }

    bool send_req_sync_msg(const ReqSyncMsg &msg) {
        set_sync_using_net_plugin();
        if (sync_using_net_plugin == 1) {
            return app().get_plugin<net_plugin>().send_req_sync(msg);
        } else {
            return app().get_plugin<kcp_plugin>().send_req_sync(msg);
        }
    }

    void send_rsp_block_num_range(const fc::sha256 &nodeId, const RspBlockNumRangeMsg &msg) {
        if (app().find_plugin<net_plugin>()) {
            app().get_plugin<net_plugin>().send_block_num_range(nodeId, msg);
        }
        if (app().find_plugin<kcp_plugin>()) {
            app().get_plugin<kcp_plugin>().send_block_num_range(nodeId, msg);
        }
    }

    std::shared_ptr<Node> Node::s_self(nullptr);

    std::shared_ptr<Node> Node::initAndGetInstance(boost::asio::io_service& ioservice) {
        if (!s_self) {
            s_self = std::shared_ptr<Node>(new Node(ioservice));
        }
        return s_self;
    }

    std::shared_ptr<Node> Node::getInstance() {
        return s_self;
    }

    Node::Node(boost::asio::io_service& ioservice) : m_ready(false), m_connected(false), m_syncing(false),
                                                                 m_syncFailed(false),
                                                                 m_phase(kPhaseInit), m_baxCount(0), m_timer(ioservice),
                                                                 m_preRunTimer(ioservice),
                                                                 m_schedulerPtr(std::make_shared<Scheduler>()) {

        std::string ver =
            ultrainio::utilities::common::itoh(static_cast<uint32_t>(app().version())) + "+" + chain::get_version_str();
        ilog("Code version is ${s}", ("s", ver));
    };

    uint32_t Node::getBlockNum() const {
        const chain::controller &chain = appbase::app().get_plugin<chain_plugin>().chain();
        return chain.head_block_num() + 1;

    }

    uint32_t Node::getBaxCount() const {
        return m_baxCount;
    }

    ConsensusPhase Node::getPhase() const {
        return m_phase;
    }

    void Node::setNonProducingNode(bool v) {
        m_isNonProducingNode = v;
        m_schedulerPtr->enableEventRegister(v);
    }

    void Node::setCommitteeInfo(const std::string& account, const std::string& sk, const std::string& blsSk, const std::string& accountSk) {
        StakeVoteBase::getNodeInfo()->setCommitteeInfo(account, sk, blsSk, accountSk);
    }

    bool Node::getNonProducingNode() const {
        return m_isNonProducingNode;
    }

    void Node::reset() {
        dlog("reset node cache.");
        m_phase = kPhaseInit;
        m_baxCount = 0;
        m_schedulerPtr->reset();
    }

    void Node::readyToConnect() {
        m_connected = true;
        readyLoop(50);
    }

    bool Node::isSyncing() const {
        return m_syncing;
    }

    void Node::init() {
        m_ready = false;
        m_connected = false;
        m_syncing = false;
        m_syncFailed = false;
        m_phase = kPhaseInit;
        m_baxCount = 0;
        m_schedulerPtr->init();
    }

    void Node::readyToJoin() {
        if (!m_connected) {
            readyToConnect();
            return;
        }

        auto currentTime = fc::time_point::now();
        int64_t passTimeFromGenesis;
        ilog("readyToJoin currentTime ${t} genesis time ${s}", ("t", currentTime)("s", Genesis::s_time));
        if (currentTime < Genesis::s_time) {
            passTimeFromGenesis = (Genesis::s_time - currentTime).to_seconds();
            if (passTimeFromGenesis > Config::s_maxRoundSeconds) {
                readyLoop(Config::s_maxRoundSeconds);
            } else if (passTimeFromGenesis == 0) {
                m_ready = true;
                run();
            } else {
                readyLoop(passTimeFromGenesis);
            }
        } else if (currentTime == Genesis::s_time) {
            m_ready = true;
            run();
        } else {
            passTimeFromGenesis = (currentTime - Genesis::s_time).to_seconds();
            // run when genesis in startup period
            if (passTimeFromGenesis == 0 || StakeVoteBase::isGenesisLeaderAndInGenesisPeriod()) {
                m_ready = true;
                run();
            } else {
                m_phase = kPhaseInit;
                syncBlock();
            }
        }
    }

    void Node::preRunBa0BlockStep() {
        if (m_phase != kPhaseBA1)
            return;

        bool ret = m_schedulerPtr->preRunBa0BlockStep();
        if (ret) {
            preRunBa0BlockLoop(200 * Config::s_maxPhaseSeconds);
        }
    }

    void Node::preRunBa0BlockLoop(uint32_t timeout) {
        m_preRunTimer.expires_from_now(boost::posix_time::milliseconds(timeout));
        m_preRunTimer.async_wait([this](boost::system::error_code ec) {
            if (ec.value() == boost::asio::error::operation_aborted) {
                ilog("pre run ba0 block timer cancel");
            } else {
                this->preRunBa0BlockStep();
            }
        });
    }

    void Node::readyLoop(uint32_t timeout) {
        m_timer.expires_from_now(boost::posix_time::seconds(timeout));
        m_currentTimerHandlerNo = THN_READY;
        resetTimerCanceled(THN_READY);
        m_timer.async_wait([this](boost::system::error_code ec) {
            if (ec.value() == boost::asio::error::operation_aborted) {
                ilog("ready loop timer cancel");
            } else {
                if (isTimerCanceled(THN_READY)) {
                    ilog("Loop timer has been already canceled.");
                } else {
                    this->readyToJoin();
                }
            }
        });
    }

    void Node::syncBlockLoop(uint32_t timeout) {
        m_timer.expires_from_now(boost::posix_time::milliseconds(timeout));
        m_currentTimerHandlerNo = THN_SYNC_BLOCK;
        resetTimerCanceled(THN_SYNC_BLOCK);
        m_timer.async_wait([this](boost::system::error_code ec) {
            if (ec.value() == boost::asio::error::operation_aborted) {
                ilog("sync block timer cancel");
            } else {
                if (isTimerCanceled(THN_SYNC_BLOCK)) {
                    ilog("Loop timer has been already canceled.");
                } else {
                    this->syncBlock();
                }
            }
        });
    }

    void Node::syncBlock(bool once) {
        dlog("@@@@@@@@@@@syncing : ${s}", ("s", m_syncing));
        if (m_syncing) {
            syncBlockLoop(getLeftTime());
            return;
        }
        m_syncing = true;
        m_syncFailed = false;

        ReqSyncMsg msg;
        msg.startBlockNum = getLastBlocknum();
        if (msg.startBlockNum == INVALID_BLOCK_NUM) {
            msg.startBlockNum = 1;
        } else {
            msg.startBlockNum++;
        }
        msg.endBlockNum = INVALID_BLOCK_NUM;
        dlog("sync block. start id = ${sid}  end id = ${eid}", ("sid", msg.startBlockNum)("eid", msg.endBlockNum));
        if (!sendMessage(msg)) {
            elog("send sync request msg failed!!!");
            m_syncing = false;
            m_syncFailed = true;
        }
        if (!once) {
            syncBlockLoop(getLeftTime());
        }
    }

    void Node::fastProcess() {
        RoundInfo info(getBlockNum(), kPhaseBA1);

        if (m_schedulerPtr->isFastba0(info)) {
            dlog("fastProcess. fastblock begin. blockNum = ${blockNum}.",("blockNum", getBlockNum()));
            ba0Process();
            return;
        }

        ba0Loop(getLeftTime());
    }

    void Node::ba0Process() {
        if (!m_ready) {
            syncBlock();
            return;
        }

        dlog("ba0Process begin. blockNum = ${id}.", ("id", getBlockNum()));

        //to_do hash error process
        Block ba0Block = m_schedulerPtr->produceTentativeBlock();
        //monitor begin, record ba0 block producing time
        if(ba0Callback != nullptr) {
            ba0Callback();
        }
        //monitor end
        m_schedulerPtr->setBa0Block(ba0Block);
        if ((!isBlank(ba0Block.id()))
            && (ba0Block.previous != m_schedulerPtr->getPreviousBlockhash())) {
            elog("ba0Process error. previous block hash error. hash = ${hash1} local hash = ${hash2}",
                 ("hash1", ba0Block.previous)("hash2", m_schedulerPtr->getPreviousBlockhash()));

            ULTRAIN_ASSERT(false, chain::chain_exception, "DB error. please reset with cmd --delete-all-blocks.");
        }

        ba1Loop(getLeftTime());

        m_schedulerPtr->resetEcho();
        m_phase = kPhaseBA1;
        m_baxCount = 0;

        MsgMgr::getInstance()->moveToNewStep(getBlockNum(), kPhaseBA1, 0);

        dlog("############## ba0 finish blockNum = ${id}, voter.hash = ${hash1}, prepare ba1. isVoter = ${isVoter}",
             ("id", getBlockNum())("hash1", short_hash(ba0Block.id()))("isVoter",MsgMgr::getInstance()->isVoter(getBlockNum(), kPhaseBA1, 0)));

        vote(getBlockNum(), kPhaseBA1, 0);

        RoundInfo info(getBlockNum(), m_phase);
        m_schedulerPtr->processCache(info);
        // Only producing node will pre-run proposed block, non-producing node still
        // try to run trx asap.
        if (isListener(getBlockNum(), m_phase, m_baxCount)) {
            bool ret = m_schedulerPtr->preRunBa0BlockStart();
            if (ret) {
                preRunBa0BlockLoop(200 * Config::s_maxPhaseSeconds);
            }
        }
    }

    void Node::vote(uint32_t blockNum, ConsensusPhase phase, uint32_t baxCount) {
        dlog("vote. blockNum = ${blockNum} phase = ${phase} baxCount = ${cnt}", ("blockNum", blockNum)
                ("phase",uint32_t(phase))("cnt",baxCount));

        if (kPhaseBA0 == phase) {
            if (MsgMgr::getInstance()->isProposer(blockNum)) {
                ProposeMsg propose;
                bool ret = m_schedulerPtr->initProposeMsg(propose);
                ULTRAIN_ASSERT(ret, chain::chain_exception, "Init propose msg failed");
                m_schedulerPtr->insert(propose);
                sendMessage(propose);
                if (MsgMgr::getInstance()->isVoter(getBlockNum(), kPhaseBA0, 0)) {
                    EchoMsg echo = MsgBuilder::constructMsg(propose);
                    ULTRAIN_ASSERT(m_schedulerPtr->verifyMyBlsSignature(echo), chain::chain_exception, "bls signature error, check bls private key pls");
                    m_schedulerPtr->insert(echo);
                    dlog("vote. echo.block_hash : ${block_hash}", ("block_hash", short_hash(echo.blockId)));
                    sendMessage(echo);
                }
            }
            return;
        }

        if (MsgMgr::getInstance()->isVoter(blockNum, phase, baxCount)) {
            const Block* ba0Block = m_schedulerPtr->getBa0Block();
            if (isEmpty(ba0Block->id())) {
                elog("vote ba0Block is empty, and send echo for empty block");
                sendEchoForEmptyBlock();
            } else if (m_schedulerPtr->verifyBa0Block()) { // not empty, verify
                EchoMsg echo = MsgBuilder::constructMsg(*ba0Block);
                ULTRAIN_ASSERT(m_schedulerPtr->verifyMyBlsSignature(echo), chain::chain_exception, "bls signature error, check bls private key pls");
                m_schedulerPtr->insert(echo);
                dlog("vote. echo.block_hash : ${block_hash}", ("block_hash", short_hash(echo.blockId)));
                sendMessage(echo);
            } else {
                elog("vote. verify ba0Block failed. And send echo for empty block");
                sendEchoForEmptyBlock();
            }
        }
    }

    void Node::ba1Process() {
        // produce block
        Block ba1Block = m_schedulerPtr->produceTentativeBlock();
        m_phase = kPhaseInit; // why reset to kPhaseInit
        //monitor begin, record ba1 block producing time
        if(ba1Callback != nullptr) {
            ba1Callback();
        }
        //monitor end

        dlog("ba1Process begin. blockNum = ${id}.", ("id", getBlockNum()));

        if ((!isBlank(ba1Block.id()))
            && (ba1Block.previous != m_schedulerPtr->getPreviousBlockhash())) {

            elog("ba1Process error. previous block hash error. hash = ${hash1} local hash = ${hash2}",
                 ("hash1", short_hash(ba1Block.previous))("hash2", short_hash(m_schedulerPtr->getPreviousBlockhash())));

            ULTRAIN_ASSERT(false, chain::chain_exception, "DB error. please reset with cmd --delete-all-blocks.");
        }

        std::shared_ptr<Block> blockPtr = std::make_shared<chain::signed_block>();
        if (!isBlank(ba1Block.id())) {
            *blockPtr = ba1Block;
        } else {
            elog("ba1Process ba1 finish. block is blank. phase ba2 begin.");
            m_phase = kPhaseBA1;
            baxLoop(getLeftTime());
            return;
        }

        m_schedulerPtr->reportEmptyBlockReason(blockPtr->id(), m_syncing);
        dlog("##############ba1 finish blockNum = ${block_num}, hash = ${hash}, head_hash = ${head_hash}",
             ("block_num", getBlockNum())
             ("hash", short_hash(blockPtr->id()))
             ("head_hash", short_hash(m_schedulerPtr->getPreviousBlockhash())));
        m_schedulerPtr->produceBlock(blockPtr);

        ULTRAIN_ASSERT(blockPtr->id() == m_schedulerPtr->getPreviousBlockhash(), chain::chain_exception,
                "Produced block hash is not expected id : ${id} while previous : ${previous}",
                ("id", blockPtr->id())("previous", m_schedulerPtr->getPreviousBlockhash()));
        run();
    }

    bool Node::isNeedSync() {
        return m_schedulerPtr->isNeedSync();
    }

    void Node::baxProcess() {
        ilog("In baxProcess");
        m_schedulerPtr->invokeDeduceWhenBax();

        if (m_phase == kPhaseInit) {
            dlog("baxProcess finish. Sync block ok. blockNum = ${id}, m_syncing = ${m_syncing}.",
                 ("id", getBlockNum() - 1)("m_syncing", (uint32_t) m_syncing));
            syncBlock();
            return;
        }

        //fast into baxcount 20
        if ((m_baxCount < (Config::kMaxBaxCount - m_phase))
            && (m_schedulerPtr->isChangePhase())) {
            m_baxCount = Config::kMaxBaxCount - m_phase - 1;
            dlog("baxProcess.ChangePhase to baxcount[20]. blockNum = ${id}, m_baxCount = ${phase}", ("id", getBlockNum())("phase", m_baxCount));
            baxLoop(getLeftTime());
            return;
        }

        Block baxBlock = m_schedulerPtr->produceTentativeBlock();
        if (!isBlank(baxBlock.id())) {
            m_ready = true;
            m_syncing = false;
            m_syncFailed = false;
            stop_sync_block();
            signed_block_ptr blockPtr = std::make_shared<chain::signed_block>();
            *blockPtr = baxBlock;
            dlog("##############baxProcess. finish blockNum = ${block_num}, hash = ${hash}, head_hash = ${head_hash}",
                 ("block_num", getBlockNum())
                 ("hash", short_hash(blockPtr->id()))
                 ("head_hash", short_hash(m_schedulerPtr->getPreviousBlockhash())));

            m_schedulerPtr->reportEmptyBlockReason(blockPtr->id(), m_syncing);
            m_schedulerPtr->reportMaxBaxCountStatistics(blockPtr->id(), m_syncing);
            m_schedulerPtr->produceBlock(blockPtr);

            ULTRAIN_ASSERT(blockPtr->id() == m_schedulerPtr->getPreviousBlockhash(),
                           chain::chain_exception, "Produced block hash is not expected");

            doFastBlock();
        } else {
            elog("baxProcess.phase bax finish. block is blank.");
            if (isNeedSync()) {
                dlog("baxProcess. syncing begin. m_baxCount = ${count}.", ("count", m_baxCount));
                syncBlock(true);
            }
            baxLoop(getLeftTime());
        }
    }

    void Node::runLoop(uint32_t timeout) {
        dlog("start runLoop timeout = ${timeout}", ("timeout", timeout));
        m_timer.expires_from_now(boost::posix_time::milliseconds(timeout));
        m_currentTimerHandlerNo = THN_RUN;
        resetTimerCanceled(THN_RUN);
        m_timer.async_wait([this](boost::system::error_code ec) {
            if (ec.value() == boost::asio::error::operation_aborted) {
                ilog("run loop timer cancel");
            } else {
                if (isTimerCanceled(THN_RUN)) {
                    ilog("Loop timer has been already canceled.");
                } else {
                    this->run();
                }
            }
        });
    }

    void Node::ba0Loop(uint32_t timeout) {
        dlog("start ba0Loop timeout = ${timeout}", ("timeout", timeout));
        m_timer.expires_from_now(boost::posix_time::milliseconds(timeout));
        m_currentTimerHandlerNo = THN_BA0;
        resetTimerCanceled(THN_BA0);
        m_timer.async_wait([this](boost::system::error_code ec) {
            if (ec.value() == boost::asio::error::operation_aborted) {
                ilog("ba0 loop timer cancel");
            } else {
                if (isTimerCanceled(THN_BA0)) {
                    ilog("Loop timer has been already canceled.");
                } else {
                    this->ba0Process();
                }
            }
        });
    }

    void Node::fastLoop(uint32_t timeout) {
        dlog("start fastLoop timeout = ${timeout}", ("timeout", timeout));
        m_timer.expires_from_now(boost::posix_time::milliseconds(timeout));
        m_currentTimerHandlerNo = THN_FAST_CHECK;
        resetTimerCanceled(THN_FAST_CHECK);
        m_timer.async_wait([this](boost::system::error_code ec) {
            if (ec.value() == boost::asio::error::operation_aborted) {
                ilog("fast check timer cancel");
            } else {
                if (isTimerCanceled(THN_FAST_CHECK)) {
                    ilog("fast check timer has been already canceled.");
                } else {
                    this->fastProcess();
                }
            }
        });
    }

    void Node::ba1Loop(uint32_t timeout) {
        dlog("start ba1Loop timeout = ${timeout}", ("timeout", timeout));
        m_timer.expires_from_now(boost::posix_time::milliseconds(timeout));
        m_currentTimerHandlerNo = THN_BA1;
        resetTimerCanceled(THN_BA1);
        m_timer.async_wait([this](boost::system::error_code ec) {
            if (ec.value() == boost::asio::error::operation_aborted) {
                ilog("ba1 loop timer cancel");
            } else {
                if (isTimerCanceled(THN_BA1)) {
                    ilog("Loop timer has been already canceled.");
                } else {
                    this->ba1Process();
                }
            }
        });
    }

    void Node::baxLoop(uint32_t timeout) {
        dlog("start baxLoop timeout = ${timeout}", ("timeout", timeout));
        m_timer.expires_from_now(boost::posix_time::milliseconds(timeout));
        m_currentTimerHandlerNo = THN_BAX;
        resetTimerCanceled(THN_BAX);
        m_timer.async_wait([this](boost::system::error_code ec) {
            if (ec.value() == boost::asio::error::operation_aborted) {
                dlog("bax loop timer cancel");
            } else {
                if (isTimerCanceled(THN_BAX)) {
                    ilog("Loop timer has been already canceled.");
                } else {
                    this->baxProcess();
                }
            }
        });

        m_schedulerPtr->moveEchoMsg2AllPhaseMap();
        m_schedulerPtr->resetEcho();
        m_schedulerPtr->clearPreRunStatus();
        m_phase = kPhaseBAX;
        m_baxCount++;
        MsgMgr::getInstance()->moveToNewStep(getBlockNum(), kPhaseBAX, m_baxCount);
        dlog("bax loop. Voter = ${Voter}, m_baxCount = ${count}.",
             ("Voter", MsgMgr::getInstance()->isVoter(getBlockNum(), kPhaseBAX, m_baxCount))
                     ("count",m_baxCount));

        if ((m_baxCount + m_phase) >= Config::kMaxBaxCount) {
            if (MsgMgr::getInstance()->isVoter(getBlockNum(), kPhaseBAX, m_baxCount)) {
                sendEchoForEmptyBlock();
            }
        } else {
            vote(getBlockNum(), kPhaseBAX, m_baxCount);
        }

        RoundInfo info(getBlockNum(), m_phase + m_baxCount);
        m_schedulerPtr->processCache(info);
    }

    bool Node::handleMessage(const EchoMsg &echo) {
        bool rtn =  m_schedulerPtr->handleMessage(echo);
        if (app().find_plugin<net_plugin>()) {
            app().get_plugin<net_plugin>().partial_broadcast(echo,rtn);
        }
        if (app().find_plugin<kcp_plugin>()) {
            app().get_plugin<kcp_plugin>().partial_broadcast(echo,rtn);
        }
        return rtn ;
    }

    bool Node::handleMessage(const ProposeMsg &propose) {
        bool rtn = m_schedulerPtr->handleMessage(propose);
        if (app().find_plugin<net_plugin>()) {
            app().get_plugin<net_plugin>().partial_broadcast(propose,rtn);
        }
        if (app().find_plugin<kcp_plugin>()) {
            app().get_plugin<kcp_plugin>().partial_broadcast(propose,rtn);
        }
        return rtn ;
    }

    bool Node::handleMessage(const fc::sha256 &nodeId, const ReqSyncMsg &msg) {
        return m_schedulerPtr->handleMessage(nodeId, msg);
    }

    bool Node::handleMessage(const fc::sha256 &nodeId, const ReqBlockNumRangeMsg &msg) {
        return m_schedulerPtr->handleMessage(nodeId, msg);
    }

    uint32_t Node::getLastBlocknum() {
        return m_schedulerPtr->getLastBlocknum();
    }

    bool Node::handleMessage(const SyncBlockMsg& msg, bool last_block, bool safe) {
        if (!m_syncing) {
            return true;
        }

        bool result = m_schedulerPtr->handleMessage(msg, safe);
        if (!result) {
            m_ready = true;
            m_syncing = false;
            m_syncFailed = true;
            stop_sync_block();
            return false;
        } else if (last_block) {
            m_ready = true;
            m_syncing = false;
            m_syncFailed = false;

            cancelTimer();
            doFastBlock();
            return true;
        } else {
            if ((m_phase == kPhaseBAX) && (msg.block.block_num() == getLastBlocknum())) {
                dlog("handleMessage blockmsg. close bax, blockNum = ${blockNum}.", ("blockNum", getLastBlocknum()));
                reset();
            }
        }
        return true;
    }

    bool Node::handleMessage(const fc::sha256 &nodeId, const SyncStopMsg& msg) {
        return m_schedulerPtr->handleMessage(nodeId, msg);
    }

    bool Node::syncFail(const ultrainio::ReqSyncMsg& sync_msg) {
        m_syncFailed = true;
        m_ready = true;
        m_syncing = false;
        if ((sync_msg.startBlockNum + syncFailed_blockheight >= sync_msg.endBlockNum) // Only the latest blocks are missing, so we can try to produce them.
            && (sync_msg.endBlockNum == getLastBlocknum() + 1)
            && (m_phase == kPhaseInit)) {
            ilog("Fail to sync block from ${s} to ${e}, but there has been already ${last} blocks in local, let me try to produce them.",
                 ("s", sync_msg.startBlockNum)("e", sync_msg.endBlockNum)("last", getLastBlocknum()));
            if (StakeVoteBase::committeeHasWorked()) {
                runLoop(getLeftTime());
            } else {
                ilog("Committee has not worked.");
            }
        }
        return true;
    }

    bool Node::syncCancel() {
        m_syncing = false;
        return true;
    }

    void Node::cancelTimer() {
        m_timer.cancel();
        setTimerCanceled(m_currentTimerHandlerNo);
    }

    void Node::sendMessage(const EchoMsg &echo) {
        if (app().find_plugin<net_plugin>()) {
            app().get_plugin<net_plugin>().broadcast(echo);
        }
        if (app().find_plugin<kcp_plugin>()) {
            app().get_plugin<kcp_plugin>().broadcast(echo);
        }
    }

    void Node::sendMessage(const ProposeMsg &propose) {
        if (app().find_plugin<net_plugin>()) {
            app().get_plugin<net_plugin>().broadcast(propose);
        }
        if (app().find_plugin<kcp_plugin>()) {
            app().get_plugin<kcp_plugin>().broadcast(propose);
        }
    }

    void Node::sendMessage(const fc::sha256 &nodeId, const SyncBlockMsg &msg) {
        send_sync_block(nodeId, msg);
    }

    bool Node::sendMessage(const ReqSyncMsg &msg) {
        return send_req_sync_msg(msg);
    }

    void Node::sendMessage(const fc::sha256 &nodeId, const RspBlockNumRangeMsg &msg) {
        send_rsp_block_num_range(nodeId, msg);
    }

    bool Node::canEnterFastBlockMode() {
        RoundInfo info(getBlockNum(), kPhaseBA0);

        if ((m_schedulerPtr->findProposeCache(info))
            && (m_schedulerPtr->findEchoCache(info))) {
            info.phase = kPhaseBA1;
            if (m_schedulerPtr->isFastba0(info)) {
                return true;
            }
        }
        return false;
    }

    void Node::run() {
        reset();

        m_phase = kPhaseBA0;
        m_baxCount = 0;
        if (canEnterFastBlockMode()) {
            dlog("start BA0. fastblock begin. blockNum = ${blockNum}", ("blockNum", getBlockNum()));
            doFastBlock();
            return;
        }

        // BA0=======
        MsgMgr::getInstance()->moveToNewStep(getBlockNum(), kPhaseBA0, 0);

        bool isProposer = MsgMgr::getInstance()->isProposer(getBlockNum());
        dlog("start BA0. blockNum = ${blockNum}. isProposer = ${isProposer} and isVoter = ${isVoter}",
             ("blockNum", getBlockNum())("isProposer", isProposer)
                     ("isVoter", MsgMgr::getInstance()->isVoter(getBlockNum(), kPhaseBA0, 0)));
        //monitor: report if this node is a proposer of this block at phase ba0.
        if(setIsProposer != nullptr) {
            setIsProposer(isProposer);
        }
        RoundInfo info(getBlockNum(), m_phase);
        m_schedulerPtr->processCache(info);

        vote(getBlockNum(), kPhaseBA0, 0);

        if ((getLeftTime() > (Config::s_maxTrxMicroSeconds/1000 + 300)) && (!MsgMgr::getInstance()->isProposer(getBlockNum()))) {
            fastLoop(Config::s_maxTrxMicroSeconds/1000 + 300);
        } else {
            ba0Loop(getLeftTime());
        }
    }

    void Node::join() {
        m_syncing = true;
    }

    void Node::fastBa0() {
        m_phase = kPhaseBA0;
        m_baxCount = 0;
        dlog("fastBa0 begin. blockNum = ${id}", ("id", getBlockNum()));
        RoundInfo info(getBlockNum(), m_phase + m_baxCount);
        m_schedulerPtr->fastProcessCache(info);
    }

    void Node::fastBa1() {
        Block ba0Block = m_schedulerPtr->produceTentativeBlock();
        m_schedulerPtr->setBa0Block(ba0Block);

        m_phase = kPhaseBA1;
        m_baxCount = 0;
        m_schedulerPtr->resetEcho();

        RoundInfo info(getBlockNum(), m_phase + m_baxCount);
        m_schedulerPtr->fastProcessCache(info);

        dlog("fastBa1 begin. blockNum = ${id}", ("id", getBlockNum()));

        Block finalBlock = m_schedulerPtr->produceTentativeBlock();
        if (!isBlank(finalBlock.id())) {
            info.blockNum = getBlockNum() + 1;
            info.phase = kPhaseBA0;

            if (m_schedulerPtr->findEchoCache(info)) {
                dlog("##############fastBa1.finish blockNum = ${id}, hash = ${hash}",
                     ("id", getBlockNum())("hash", short_hash(finalBlock.id())));
                m_schedulerPtr->produceBlock(std::make_shared<chain::signed_block>(finalBlock));

                doFastBlock();
            } else {
                if ((getLeftTime() == (1000 * Config::s_maxPhaseSeconds)) && (isProcessNow())) {
                    //todo process two phase
                    ba1Process();
                } else {
                    vote(getBlockNum(),kPhaseBA1,0);
                    ba1Loop(getLeftTime());
                }
            }
        } else {
            info.blockNum = getBlockNum();
            info.phase = kPhaseBAX + 1;
            if (m_schedulerPtr->findEchoCache(info)) {
                fastBax();
            } else {
                if ((getLeftTime() == (1000 * Config::s_maxPhaseSeconds)) && (isProcessNow())) {
                    //todo process two phase
                    ba1Process();
                } else {
                    vote(getBlockNum(),kPhaseBA1,0);
                    ba1Loop(getLeftTime());
                }
            }
        }
    }

    void Node::fastBax() {
        m_phase = kPhaseBAX;
        m_baxCount++;

        m_schedulerPtr->resetEcho();

        //fast into baxcount 20
        if ((m_baxCount < (Config::kMaxBaxCount - m_phase))
            && (m_schedulerPtr->isChangePhase())) {
            m_baxCount = Config::kMaxBaxCount - m_phase;
            dlog("fastBax.ChangePhase to baxcount[20]. blockNum = ${id}, m_baxCount = ${phase}", ("id", getBlockNum())("phase", m_baxCount));
        }

        RoundInfo info(getBlockNum(), m_phase + m_baxCount);
        m_schedulerPtr->fastProcessCache(info);
        dlog("fastBax begin. blockNum = ${id}, phase = ${phase}", ("id", getBlockNum())("phase", info.phase));

        Block finalBlock = m_schedulerPtr->produceTentativeBlock();
        if (!isBlank(finalBlock.id())) {
            info.blockNum = getBlockNum() + 1;
            info.phase = kPhaseBA0;

            if (m_schedulerPtr->findEchoCache(info)) {
                m_schedulerPtr->produceBlock(std::make_shared<chain::signed_block>(finalBlock));
                dlog("##############finish blockNum = ${id}, hash = ${hash}",
                     ("id", getBlockNum())("hash", short_hash(finalBlock.id())));

                doFastBlock();
            } else {
                //todo addjust
                baxLoop(getLeftTime());
            }
        } else {
            info.blockNum = getBlockNum();
            info.phase = m_phase + m_baxCount + 1;
            if (m_schedulerPtr->findEchoCache(info)) {
                fastBax();
            } else {
                //todo addjust
                baxLoop(getLeftTime());
            }
        }
    }

    void Node::doFastBlock() {
        dlog("begin. blockNum = ${id}", ("id", getBlockNum()));
        MsgMgr::getInstance()->moveToNewStep(getBlockNum(), kPhaseBA0, 0);
        reset();
        m_schedulerPtr->resetTimestamp();

        RoundInfo info(getBlockNum(), kPhaseBA0);
        if (m_schedulerPtr->findEchoCache(info)) {
            fastBa0();
        } else {
            run();
            return;
        }

        info.phase = kPhaseBA1;

        if (m_schedulerPtr->findEchoCache(info)) {
            fastBa1();
        } else {
            if ((getLeftTime() == (1000 * Config::s_maxPhaseSeconds)) && (isProcessNow())) {
                //todo process two phase
                ba0Process();
            } else {
                ba0Loop(getLeftTime());
            }
        }
    }

    bool Node::isProcessNow() {
        dlog("isProcessNow. current timestamp = ${id1}, fast timestamp = ${id2}",
             ("id1", getRoundCount())("id2",m_schedulerPtr->getFastTimestamp()));
        if ((m_schedulerPtr->getFastTimestamp() < getRoundCount()) && (m_schedulerPtr->getFastTimestamp() != 0)) {
            return true;
        }

        return false;
    }

    uint32_t Node::getRoundCount() {
        fc::time_point currentTime = fc::time_point::now();
        int64_t passTimeFromGenesis = (currentTime - Genesis::s_time).to_seconds();
        return passTimeFromGenesis / Config::s_maxPhaseSeconds;
    }

    uint32_t Node::getLeftTime() {
        fc::time_point currentTime = fc::time_point::now();
        int64_t passTimeFromGenesis = (currentTime - Genesis::s_time).count()/1000;

        uint32_t round = 1000 * Config::s_maxPhaseSeconds - (passTimeFromGenesis % (1000 * Config::s_maxPhaseSeconds));
        dlog("interval = ${id}", ("id", round));
        return round;
    }

    BlockIdType Node::getPreviousHash() {
        return m_schedulerPtr->getPreviousBlockhash();
    }

    const std::shared_ptr<Scheduler> Node::getScheduler() const {
        return std::shared_ptr<Scheduler> (m_schedulerPtr);
    }

    bool Node::isBlank(const BlockIdType& blockId) {
        return m_schedulerPtr->isBlank(blockId);
    }

    bool Node::isEmpty(const BlockIdType& blockId) {
        return m_schedulerPtr->isEmpty(blockId);
    }

    void Node::sendEchoForEmptyBlock() {
        Block block = m_schedulerPtr->emptyBlock();
        dlog("vote empty block. blockNum = ${blockNum} hash = ${hash}", ("blockNum",getBlockNum())("hash", short_hash(block.id())));
        EchoMsg echoMsg = MsgBuilder::constructMsg(block);
        ULTRAIN_ASSERT(m_schedulerPtr->verifyMyBlsSignature(echoMsg), chain::chain_exception, "bls signature error, check bls private key pls");
        m_schedulerPtr->insert(echoMsg);
        sendMessage(echoMsg);
    }

    int Node::getCommitteeMemberNumber() {
        std::shared_ptr<StakeVoteBase> voterSysPtr = MsgMgr::getInstance()->getStakeVote(this->getBlockNum());
        return voterSysPtr->getCommitteeMemberNumber();
    }

    void Node::setGenesisTime(const fc::time_point& tp) {
        Genesis::s_time = tp;
        ilog("Set Genesis time is ${t}", ("t", tp));
    }

    void Node::setGenesisStartupTime(int32_t minutes) {
        Genesis::s_genesisStartupTime = minutes;
        Genesis::s_genesisStartupBlockNum = Genesis:: s_genesisStartupTime * 60/Config::s_maxRoundSeconds;
        ilog("Genesis startup time : ${minutes} minutes, startup block num : ${number}",
                ("minutes",Genesis::s_genesisStartupTime)("number", Genesis::s_genesisStartupBlockNum));
    }

    void Node::setGenesisPk(const std::string& pk) {
        Genesis::s_genesisPk = pk;
        ilog("Genesis pk : ${pk}", ("pk", pk));
    }

    void Node::setRoundAndPhaseSecond(int32_t roundSecond, int32_t phaseSecond) {
        Config::s_maxRoundSeconds = roundSecond;
        Config::s_maxPhaseSeconds = phaseSecond;
        ilog("maxRoundSecond : ${maxRoundSecond}, maxPhaseSecond : ${maxPhaseSecond}",
                ("maxRoundSecond", Config::s_maxRoundSeconds)("maxPhaseSecond", Config::s_maxPhaseSeconds));
        setSyncFailBlockHeight();
    }
    void Node::setTrxsSecond(int32_t trxssecond) {
        Config::s_maxTrxMicroSeconds = trxssecond;
        ilog("s_maxTrxMicroSeconds : ${s_maxTrxMicroSeconds}",
             ("s_maxTrxMicroSeconds", Config::s_maxTrxMicroSeconds));
    }

    void Node::setAllowReportEvil(bool v) {
        Config::s_allowReportEvil = v;
        ilog("s_maxTrxMicroSeconds : ${v}", ("v", Config::s_allowReportEvil));
    }
    void Node::setSyncFailBlockHeight(){
        int sync_waitblock_interval = app().get_plugin<net_plugin>().get_sync_waitblock_interval();
        int sync_waitblocknum_interval = app().get_plugin<net_plugin>().get_sync_waitblocknum_interval();
        int block_height = (sync_waitblocknum_interval+sync_waitblock_interval*2)/Config::s_maxRoundSeconds + 1;
        syncFailed_blockheight = block_height*2;
        ilog("syncFailed_blockheight ${syncFailed_blockheight}",("syncFailed_blockheight",syncFailed_blockheight));
    }
    ReportHandler& Node::getEvilReportHandler() {
        return m_evilReportHandler;
    }

    ReportHandler& Node::getEmptyBlockReportHandler() {
        return m_emptyBlockReportHandler;
    }

    ReportHandler& Node::getMaxBaxCountReportHandler() {
        return m_maxBaxCountReportHandler;
    }

    void Node::setTimerCanceled(TimerHandlerNumber thn) {
        m_timerCanceledBits |= (1 << thn);
    }

    void Node::resetTimerCanceled(TimerHandlerNumber thn) {
        m_timerCanceledBits &= (~(1 << thn));
    }

    bool Node::isTimerCanceled(TimerHandlerNumber thn) const {
        return m_timerCanceledBits & (1 << thn);
    }

    bool Node::isListener(uint32_t blockNum, ConsensusPhase phase, uint32_t baxCount) {
        return !MsgMgr::getInstance()->isVoter(blockNum, phase, baxCount) && !m_isNonProducingNode;
    }
}
