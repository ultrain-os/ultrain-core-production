#include <rpos/Node.h>

#include <chrono>
#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include <fc/log/logger.hpp>

#include <ultrainio/chain/callback_manager.hpp>
#include <rpos/Config.h>
#include <rpos/Genesis.h>
#include <rpos/MsgBuilder.h>
#include <rpos/MsgMgr.h>
#include <rpos/Scheduler.h>
#include <rpos/NodeInfo.h>
#include <rpos/StakeVoteBase.h>
#include <rpos/Utils.h>

// eos net
#include <appbase/application.hpp>
#include <ultrainio/net_plugin/net_plugin.hpp>

using namespace boost::asio;
using namespace std;

namespace ultrainio {

    char version[]="2e1168";

    std::shared_ptr<UranusNode> UranusNode::s_self(nullptr);

    std::shared_ptr<UranusNode> UranusNode::initAndGetInstance(boost::asio::io_service &ioservice) {
        if (!s_self) {
            s_self = std::shared_ptr<UranusNode>(new UranusNode(ioservice));
        }
        return s_self;
    }

    std::shared_ptr<UranusNode> UranusNode::getInstance() {
        return s_self;
    }

    UranusNode::UranusNode(boost::asio::io_service &ioservice) : m_ready(false), m_connected(false), m_syncing(false),
                                                                 m_syncFailed(false),
                                                                 m_phase(kPhaseInit), m_baxCount(0), m_timer(ioservice),
                                                                 m_preRunTimer(ioservice),
                                                                 m_schedulerPtr(std::make_shared<Scheduler>()) {
        ilog("Code version is ${s}", ("s", version));
        ultrainio::chain::callback_manager::get_self()->register_callback(m_schedulerPtr);
    };

    uint32_t UranusNode::getBlockNum() const {
        const chain::controller &chain = appbase::app().get_plugin<chain_plugin>().chain();
        return chain.head_block_num() + 1;

    }

    uint32_t UranusNode::getBaxCount() const {
        return m_baxCount;
    }

    ConsensusPhase UranusNode::getPhase() const {
        return m_phase;
    }

    void UranusNode::setNonProducingNode(bool v) {
        m_isNonProducingNode = v;
        m_schedulerPtr->enableEventRegister(v);
    }

    void UranusNode::setMyInfoAsCommitteeKey(const std::string& sk, const std::string& blsSk, const std::string& account) {
        StakeVoteBase::getKeyKeeper()->setMyInfoAsCommitteeKey(sk, blsSk, account);
    }

    bool UranusNode::getNonProducingNode() const {
        return m_isNonProducingNode;
    }

    void UranusNode::reset() {
        dlog("reset node cache.");
        m_phase = kPhaseInit;
        m_baxCount = 0;
        m_schedulerPtr->reset();
    }

    void UranusNode::readyToConnect() {
        m_connected = true;
        readyLoop(50);
    }

    bool UranusNode::isSyncing() const {
        return m_syncing;
    }

    void UranusNode::init() {
        m_ready = false;
        m_connected = false;
        m_syncing = false;
        m_syncFailed = false;
        m_phase = kPhaseInit;
        m_baxCount = 0;
        m_schedulerPtr->init();
    }

    void UranusNode::readyToJoin() {
        auto current_time = fc::time_point::now();
        int64_t pass_time_to_genesis;

        if (!m_connected) {
            readyToConnect();
            return;
            //m_connected = true;
        }

        ilog("readyToJoin current_time ${t} genesis time ${s}", ("t",current_time)("s",Genesis::s_time));

        if (current_time < Genesis::s_time) {
            pass_time_to_genesis = (Genesis::s_time - current_time).to_seconds();
            if (pass_time_to_genesis > Config::s_maxRoundSeconds) {
                readyLoop(Config::s_maxRoundSeconds);
            } else if (pass_time_to_genesis == 0) {
                m_ready = true;
                run();
            } else {
                readyLoop(pass_time_to_genesis);
            }
        } else if (Genesis::s_time == current_time) {
            m_ready = true;
            run();
        } else {
            pass_time_to_genesis = (current_time - Genesis::s_time).to_seconds();
            // run when genesis in startup period
            if (pass_time_to_genesis == 0 || StakeVoteBase::isGenesisLeaderAndInGenesisPeriod()) {
                m_ready = true;
                run();
            } else {
                m_phase = kPhaseInit;
                syncBlock();
            }
        }
    }

    void UranusNode::preRunBa0BlockStep() {
        if (m_phase != kPhaseBA1)
            return;

        bool ret = m_schedulerPtr->preRunBa0BlockStep();
        if (ret) {
            preRunBa0BlockLoop(200 * Config::s_maxPhaseSeconds);
        }
    }

    void UranusNode::preRunBa0BlockLoop(uint32_t timeout) {
        m_preRunTimer.expires_from_now(boost::posix_time::milliseconds(timeout));
        m_preRunTimer.async_wait([this](boost::system::error_code ec) {
            if (ec.value() == boost::asio::error::operation_aborted) {
                ilog("pre run ba0 block timer cancel");
            } else {
                this->preRunBa0BlockStep();
            }
        });
    }

    void UranusNode::readyLoop(uint32_t timeout) {
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

    void UranusNode::syncBlockLoop(uint32_t timeout) {
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

    void UranusNode::syncBlock(bool once) {
        ReqSyncMsg msg;
        dlog("@@@@@@@@@@@ syncBlock syncing:${s}", ("s", m_syncing));
        if (m_syncing) {
            syncBlockLoop(getRoundInterval());
            return;
        }

        m_syncing = true;
        m_syncFailed = false;

        msg.startBlockNum = getLastBlocknum();
        if (msg.startBlockNum == INVALID_BLOCK_NUM) {
            msg.startBlockNum = 0;
        } else {
            msg.startBlockNum++;
        }

        msg.endBlockNum = INVALID_BLOCK_NUM;

        dlog("sync block. start id = ${sid}  end id = ${eid}", ("sid", msg.startBlockNum)("eid", msg.endBlockNum));
        if (!sendMessage(msg)) {
            elog("@syncBlock send sync request msg failed!!!");
            m_syncing = false;
            m_syncFailed = true;
        }

        if (!once) {
            syncBlockLoop(getRoundInterval());
        }
    }

    void UranusNode::fastProcess() {
        msgkey msg_key;
        msg_key.blockNum = getBlockNum();
        msg_key.phase = kPhaseBA1;

        if (m_schedulerPtr->isFastba0(msg_key)) {
            dlog("fastProcess. fastblock begin. blockNum = ${blockNum}.",("blockNum", getBlockNum()));
            ba0Process();
            return;
        }

        ba0Loop(getRoundInterval());
    }

    void UranusNode::ba0Process() {
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

        msgkey msg_key;
        ba1Loop(getRoundInterval());

        m_schedulerPtr->resetEcho();
        m_phase = kPhaseBA1;
        m_baxCount = 0;

        MsgMgr::getInstance()->moveToNewStep(getBlockNum(), kPhaseBA1, 0);

        dlog("############## ba0 finish blockNum = ${id}, host_name = ${host_name}",
             ("id", getBlockNum())("host_name", boost::asio::ip::host_name()));
        dlog("ba0Process voter.hash = ${hash1}",("hash1", short_hash(ba0Block.id())));
        dlog("ba0Process. prepare ba1. blockNum = ${blockNum}, isVoter = ${isVoter}.", ("blockNum", getBlockNum())
                ("isVoter",MsgMgr::getInstance()->isVoter(getBlockNum(), kPhaseBA1, 0)));

        vote(getBlockNum(),kPhaseBA1,0);

        msg_key.blockNum = getBlockNum();
        msg_key.phase = m_phase;
        m_schedulerPtr->processCache(msg_key);
        // Only producing node will pre-run proposed block, non-producing node still
        // try to run trx asap.
        if (!MsgMgr::getInstance()->isVoter(getBlockNum(), m_phase, m_baxCount) && !m_isNonProducingNode) {
            bool ret = m_schedulerPtr->preRunBa0BlockStart();
            if (ret) {
                preRunBa0BlockLoop(200 * Config::s_maxPhaseSeconds);
            }
        }
    }

    void UranusNode::vote(uint32_t blockNum, ConsensusPhase phase, uint32_t baxCount) {
        const Block* ba0Block = nullptr;

        dlog("vote. blockNum = ${blockNum} phase = ${phase} baxCount = ${cnt}", ("blockNum", blockNum)
                ("phase",uint32_t(phase))("cnt",baxCount));

        if (kPhaseBA0 == phase) {
            if (MsgMgr::getInstance()->isProposer(blockNum)) {
                ProposeMsg propose;
                bool ret = m_schedulerPtr->initProposeMsg(&propose);
                ULTRAIN_ASSERT(ret, chain::chain_exception, "Init propose msg failed");
                dlog("vote.propose.block_hash : ${block_hash}", ("block_hash", short_hash(propose.block.id())));
                m_schedulerPtr->insert(propose);
                sendMessage(propose);
                if (MsgMgr::getInstance()->isVoter(getBlockNum(), kPhaseBA0, 0)) {
                    EchoMsg echo = MsgBuilder::constructMsg(propose);
                    m_schedulerPtr->insert(echo);
                    dlog("vote. echo.block_hash : ${block_hash}", ("block_hash", short_hash(echo.blockId)));
                    sendMessage(echo);
                }
            }
            return;
        }

        if (MsgMgr::getInstance()->isVoter(blockNum, phase, baxCount)) {
            ba0Block = m_schedulerPtr->getBa0Block();
            if (isEmpty(ba0Block->id())) {
                elog("vote ba0Block is empty, and send echo for empty block");
                sendEchoForEmptyBlock();
            } else if (m_schedulerPtr->verifyBa0Block()) { // not empty, verify
                EchoMsg echo = MsgBuilder::constructMsg(*ba0Block);
                m_schedulerPtr->insert(echo);
                //echo.timestamp = getRoundCount();
                dlog("vote. echo.block_hash : ${block_hash}", ("block_hash", short_hash(echo.blockId)));
                sendMessage(echo);
            } else {
                elog("vote. verify ba0Block failed. And send echo for empty block");
                sendEchoForEmptyBlock();
            }
        }
        return;
    }

    void UranusNode::ba1Process() {
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
            baxLoop(getRoundInterval());
            return;
        }

        dlog("##############ba1Process. finish blockNum = ${block_num}, hash = ${hash}, head_hash = ${head_hash}",
             ("block_num", getBlockNum())
             ("hash", short_hash(blockPtr->id()))
             ("head_hash", short_hash(m_schedulerPtr->getPreviousBlockhash())));
        m_schedulerPtr->produceBlock(blockPtr);

        ULTRAIN_ASSERT(blockPtr->id() == m_schedulerPtr->getPreviousBlockhash(), chain::chain_exception,
                "Produced block hash is not expected id : ${id} while previous : ${previous}",
                ("id", blockPtr->id())("previous", m_schedulerPtr->getPreviousBlockhash()));
        run();
    }

    bool UranusNode::isNeedSync() {
        return m_schedulerPtr->isNeedSync();
    }

    void UranusNode::baxProcess() {
        ilog("In baxProcess");
        Block baxBlock = m_schedulerPtr->produceTentativeBlock();
        signed_block_ptr uranus_block = std::make_shared<chain::signed_block>();

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
            baxLoop(getRoundInterval());
            return;
        }

        if (!isBlank(baxBlock.id())) {
            m_ready = true;
            m_syncing = false;
            m_syncFailed = false;
            app().get_plugin<net_plugin>().stop_sync_block();
            *uranus_block = baxBlock;
            dlog("##############baxProcess. finish blockNum = ${block_num}, hash = ${hash}, head_hash = ${head_hash}",
                 ("block_num", getBlockNum())
                 ("hash", short_hash(uranus_block->id()))
                 ("head_hash", short_hash(m_schedulerPtr->getPreviousBlockhash())));

            m_schedulerPtr->produceBlock(uranus_block);

            ULTRAIN_ASSERT(uranus_block->id() == m_schedulerPtr->getPreviousBlockhash(),
                           chain::chain_exception, "Produced block hash is not expected");

            fastBlock(getBlockNum());
            //join();
        } else {
            elog("baxProcess.phase bax finish. block is blank.");

            if (isNeedSync()) {
                dlog("baxProcess. syncing begin. m_baxCount = ${count}.", ("count", m_baxCount));
                syncBlock(true);
            }

            baxLoop(getRoundInterval());
        }
    }

    void UranusNode::runLoop(uint32_t timeout) {
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

    void UranusNode::ba0Loop(uint32_t timeout) {
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

    void UranusNode::fastLoop(uint32_t timeout) {
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

    void UranusNode::ba1Loop(uint32_t timeout) {
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

    void UranusNode::baxLoop(uint32_t timeout) {
        msgkey msg_key;

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
            vote(getBlockNum(),kPhaseBAX,m_baxCount);
        }

        msg_key.blockNum = getBlockNum();
        msg_key.phase = m_phase;
        msg_key.phase += m_baxCount;
        m_schedulerPtr->processCache(msg_key);
    }

    bool UranusNode::handleMessage(const EchoMsg &echo) {
        return m_schedulerPtr->handleMessage(echo);
    }

    bool UranusNode::handleMessage(const ProposeMsg &propose) {
        return m_schedulerPtr->handleMessage(propose);
    }

    bool UranusNode::handleMessage(const fc::sha256 &nodeId, const ReqSyncMsg &msg) {
        return m_schedulerPtr->handleMessage(nodeId, msg);
    }

    bool UranusNode::handleMessage(const fc::sha256 &nodeId, const ReqLastBlockNumMsg &msg) {
        return m_schedulerPtr->handleMessage(nodeId, msg);
    }

    uint32_t UranusNode::getLastBlocknum() {
        return m_schedulerPtr->getLastBlocknum();
    }

    bool UranusNode::handleMessage(const Block &msg, bool last_block) {
        uint32_t next_blockNum = 0;
        if (!m_syncing) {
            return true;
        }

        bool result = m_schedulerPtr->handleMessage(msg);
        if (!result) {
            m_ready = true;
            m_syncing = false;
            m_syncFailed = true;
            app().get_plugin<net_plugin>().stop_sync_block();
            return false;
        } else if (last_block) {
            m_ready = true;
            m_syncing = false;
            m_syncFailed = false;

            cancelTimer();
            next_blockNum = getLastBlocknum();
            if (next_blockNum == INVALID_BLOCK_NUM) {
                next_blockNum = 0;
            } else {
                next_blockNum++;
            }
            fastBlock(next_blockNum);
            return true;
        } else {
            if ((m_phase == kPhaseBAX) && (msg.block_num() == getLastBlocknum())) {
                dlog("handleMessage blockmsg. close bax, blockNum = ${blockNum}.", ("blockNum", getLastBlocknum()));
                reset();
            }
        }

        return true;
    }

    bool UranusNode::handleMessage(const fc::sha256 &nodeId, const SyncStopMsg& msg) {
        return m_schedulerPtr->handleMessage(nodeId, msg);
    }

    bool UranusNode::syncFail(const ultrainio::ReqSyncMsg& sync_msg) {
        m_syncFailed = true;
        m_ready = true;
        m_syncing = false;

        if ((sync_msg.startBlockNum == sync_msg.endBlockNum)
            && (sync_msg.endBlockNum == getLastBlocknum() + 1)
            && (m_phase == kPhaseInit)) {
            ilog("Fail to sync block from ${s} to ${e}, but there has been already ${last} blocks in local.",
                 ("s", sync_msg.startBlockNum)("e", sync_msg.endBlockNum)("last", getLastBlocknum()));
            if (StakeVoteBase::committeeHasWorked()) {
                runLoop(getRoundInterval());
            } else {
                ilog("Committee has not worked.");
            }
        }

        return true;
    }

    bool UranusNode::syncCancel() {
        m_syncing = false;
        return true;
    }

    void UranusNode::cancelTimer() {
        m_timer.cancel();
        setTimerCanceled(m_currentTimerHandlerNo);
    }

    void UranusNode::sendMessage(const EchoMsg &echo) {
        app().get_plugin<net_plugin>().broadcast(echo);
    }

    void UranusNode::sendMessage(const ProposeMsg &propose) {
        app().get_plugin<net_plugin>().broadcast(propose);
    }

    void UranusNode::sendMessage(const fc::sha256 &nodeId, const SyncBlockMsg &msg) {
        app().get_plugin<net_plugin>().send_block(nodeId, msg);
    }

    bool UranusNode::sendMessage(const ReqSyncMsg &msg) {
        return app().get_plugin<net_plugin>().send_req_sync(msg);
    }

    void UranusNode::sendMessage(const fc::sha256 &nodeId, const RspLastBlockNumMsg &msg) {
        app().get_plugin<net_plugin>().send_last_block_num(nodeId, msg);
    }

    bool UranusNode::isFastBlock() {
        msgkey msg_key;
        msg_key.blockNum = getBlockNum();
        msg_key.phase = kPhaseBA0;

        if ((m_schedulerPtr->findProposeCache(msg_key))
            && (m_schedulerPtr->findEchoCache(msg_key))) {
            msg_key.phase = kPhaseBA1;
            if (m_schedulerPtr->isFastba0(msg_key)) {
                return true;
            }
        }

        return false;
    }

    void UranusNode::run(bool voteFlag) {
        msgkey msg_key;

        reset();

        m_phase = kPhaseBA0;
        m_baxCount = 0;

        if (isFastBlock()) {
            dlog("start BA0. fastblock begin. blockNum = ${blockNum}.",("blockNum", getBlockNum()));
            fastBlock(getBlockNum());
            return;
        }

        // BA0=======
        //ba0Loop(getRoundInterval());
        MsgMgr::getInstance()->moveToNewStep(getBlockNum(), kPhaseBA0, 0);

        bool isProposer = MsgMgr::getInstance()->isProposer(getBlockNum());
        dlog("start BA0. blockNum = ${blockNum}. isProposer = ${isProposer} and isVoter = ${isVoter}",
             ("blockNum", getBlockNum())("isProposer", isProposer)
                     ("isVoter", MsgMgr::getInstance()->isVoter(getBlockNum(), kPhaseBA0, 0)));
        //monitor: report if this node is a proposer of this block at phase ba0.
        if(setIsProposer != nullptr) {
            setIsProposer(isProposer);
        }
        msg_key.blockNum = getBlockNum();
        msg_key.phase = m_phase;
        m_schedulerPtr->processCache(msg_key);

        if (voteFlag) {
            vote(getBlockNum(), kPhaseBA0, 0);
        }

        if ((getRoundInterval() > (Config::s_maxTrxMicroSeconds/1000 + 300)) && (!MsgMgr::getInstance()->isProposer(getBlockNum()))) {
            fastLoop(Config::s_maxTrxMicroSeconds/1000 + 300);
        } else {
            ba0Loop(getRoundInterval());
        }

        return;
    }

    void UranusNode::join() {
        m_syncing = true;
    }

    void UranusNode::fastBa0() {
        msgkey msg_key;
        m_phase = kPhaseBA0;
        m_baxCount = 0;

        dlog("fastBa0 begin. blockNum = ${id}", ("id", getBlockNum()));

        msg_key.blockNum = getBlockNum();
        msg_key.phase = m_phase;
        msg_key.phase += m_baxCount;
        m_schedulerPtr->fastProcessCache(msg_key);
    }

    void UranusNode::fastBa1() {
        msgkey msg_key;
        signed_block_ptr uranus_block = std::make_shared<chain::signed_block>();

        Block ba0Block = m_schedulerPtr->produceTentativeBlock();
        m_schedulerPtr->setBa0Block(ba0Block);

        m_phase = kPhaseBA1;
        m_baxCount = 0;
        m_schedulerPtr->resetEcho();

        msg_key.blockNum = getBlockNum();
        msg_key.phase = m_phase;
        msg_key.phase += m_baxCount;
        m_schedulerPtr->fastProcessCache(msg_key);

        dlog("fastBa1 begin. blockNum = ${id}", ("id", getBlockNum()));

        Block baxBlock = m_schedulerPtr->produceTentativeBlock();
        if (!isBlank(baxBlock.id())) {
            msg_key.blockNum = getBlockNum() + 1;
            msg_key.phase = kPhaseBA0;

            if (m_schedulerPtr->findEchoCache(msg_key)) {
                *uranus_block = baxBlock;

                dlog("##############fastBa1.finish blockNum = ${id}, hash = ${hash}",
                     ("id", getBlockNum())("hash", short_hash(uranus_block->id())));
                m_schedulerPtr->produceBlock(uranus_block);

                fastBlock(msg_key.blockNum);
            } else {
                if ((getRoundInterval() == (1000 * Config::s_maxPhaseSeconds)) && (isProcessNow())) {
                    //todo process two phase
                    ba1Process();
                } else {
                    vote(getBlockNum(),kPhaseBA1,0);
                    ba1Loop(getRoundInterval());
                }
            }
        } else {
            msg_key.blockNum = getBlockNum();
            msg_key.phase = kPhaseBAX + 1;
            if (m_schedulerPtr->findEchoCache(msg_key)) {
                fastBax();
            } else {
                if ((getRoundInterval() == (1000 * Config::s_maxPhaseSeconds)) && (isProcessNow())) {
                    //todo process two phase
                    ba1Process();
                } else {
                    vote(getBlockNum(),kPhaseBA1,0);
                    ba1Loop(getRoundInterval());
                }
            }
        }
    }

    void UranusNode::fastBax() {
        msgkey msg_key;
        signed_block_ptr uranus_block = std::make_shared<chain::signed_block>();

        m_phase = kPhaseBAX;
        m_baxCount++;

        m_schedulerPtr->resetEcho();

        //fast into baxcount 20
        if ((m_baxCount < (Config::kMaxBaxCount - m_phase))
            && (m_schedulerPtr->isChangePhase())) {
            m_baxCount = Config::kMaxBaxCount - m_phase;
            dlog("fastBax.ChangePhase to baxcount[20]. blockNum = ${id}, m_baxCount = ${phase}", ("id", getBlockNum())("phase", m_baxCount));
        }

        msg_key.blockNum = getBlockNum();
        msg_key.phase = m_phase;
        msg_key.phase += m_baxCount;
        m_schedulerPtr->fastProcessCache(msg_key);
        Block baxBlock = m_schedulerPtr->produceTentativeBlock();

        dlog("fastBax begin. blockNum = ${id}, phase = ${phase}", ("id", getBlockNum())("phase", msg_key.phase));

        if (!isBlank(baxBlock.id())) {
            msg_key.blockNum = getBlockNum() + 1;
            msg_key.phase = kPhaseBA0;

            if (m_schedulerPtr->findEchoCache(msg_key)) {
                *uranus_block = baxBlock;

                m_schedulerPtr->produceBlock(uranus_block);
                dlog("##############finish blockNum = ${id}, hash = ${hash}",
                     ("id", getBlockNum())("hash", short_hash(uranus_block->id())));

                fastBlock(msg_key.blockNum);
            } else {
                //todo addjust
                baxLoop(getRoundInterval());
            }
        } else {
            msg_key.blockNum = getBlockNum();
            msg_key.phase = m_phase + m_baxCount + 1;
            if (m_schedulerPtr->findEchoCache(msg_key)) {
                fastBax();
            } else {
                //todo addjust
                baxLoop(getRoundInterval());
            }
        }
    }

    void UranusNode::fastBlock(uint32_t blockNum) {
        msgkey msg_key;

        dlog("fastBlock begin. blockNum = ${id}", ("id", getBlockNum()));

        MsgMgr::getInstance()->moveToNewStep(getBlockNum(), kPhaseBA0, 0);
        reset();
        m_schedulerPtr->resetTimestamp();

        msg_key.blockNum = getBlockNum();
        msg_key.phase = kPhaseBA0;
        if (m_schedulerPtr->findEchoCache(msg_key)) {
            fastBa0();
        } else {
            run();
            return;
        }

        msg_key.phase = kPhaseBA1;

        if (m_schedulerPtr->findEchoCache(msg_key)) {
            fastBa1();
        } else {
            if ((getRoundInterval() == (1000 * Config::s_maxPhaseSeconds)) && (isProcessNow())) {
                //todo process two phase
                ba0Process();
            } else {
                //vote(getBlockNum(),kPhaseBA0,0);
                ba0Loop(getRoundInterval());
            }
        }
    }

    bool UranusNode::isProcessNow() {
        dlog("isProcessNow. current timestamp = ${id1}, fast timestamp = ${id2}",
             ("id1", getRoundCount())("id2",m_schedulerPtr->getFastTimestamp()));

        if ((m_schedulerPtr->getFastTimestamp() < getRoundCount()) && (m_schedulerPtr->getFastTimestamp() != 0)) {
            return true;
        }

        return false;
    }

    uint32_t UranusNode::getRoundCount() {
        fc::time_point current_time = fc::time_point::now();
        int64_t pass_time_to_genesis = (current_time - Genesis::s_time).to_seconds();

        dlog("getRoundCount. count = ${id}.",
             ("id", pass_time_to_genesis / Config::s_maxPhaseSeconds));

        return pass_time_to_genesis / Config::s_maxPhaseSeconds;
    }

    uint32_t UranusNode::getRoundInterval() {
        fc::time_point current_time = fc::time_point::now();
        int64_t pass_time_to_genesis_m = (current_time - Genesis::s_time).to_seconds() * 1000;

        dlog("getRoundInterval. interval = ${id}.",
             ("id", 1000 * Config::s_maxPhaseSeconds - (pass_time_to_genesis_m % (1000 * Config::s_maxPhaseSeconds))));

        return 1000 * Config::s_maxPhaseSeconds - (pass_time_to_genesis_m % (1000 * Config::s_maxPhaseSeconds));
    }

    BlockIdType UranusNode::getPreviousHash() {
        return m_schedulerPtr->getPreviousBlockhash();
    }

    const std::shared_ptr<Scheduler> UranusNode::getScheduler() const {
        return std::shared_ptr<Scheduler> (m_schedulerPtr);
    }

    bool UranusNode::isBlank(const BlockIdType& blockId) {
        return m_schedulerPtr->isBlank(blockId);
    }

    bool UranusNode::isEmpty(const BlockIdType& blockId) {
        return m_schedulerPtr->isEmpty(blockId);
    }

    void UranusNode::sendEchoForEmptyBlock() {
        Block block = m_schedulerPtr->emptyBlock();
        dlog("vote empty block. blockNum = ${blockNum} hash = ${hash}", ("blockNum",getBlockNum())("hash", short_hash(block.id())));
        EchoMsg echoMsg = MsgBuilder::constructMsg(block);
        m_schedulerPtr->insert(echoMsg);
        //echoMsg.timestamp = getRoundCount();
        sendMessage(echoMsg);
    }

    int UranusNode::getCommitteeMemberNumber() {
        std::shared_ptr<StakeVoteBase> voterSysPtr = MsgMgr::getInstance()->getVoterSys(this->getBlockNum());
        return voterSysPtr->getCommitteeMemberNumber();
    }

    void UranusNode::setGenesisTime(const fc::time_point& tp) {
        Genesis::s_time = tp;
        ilog("Set Genesis time is ${t}", ("t", tp));
    }

    void UranusNode::setGenesisStartupTime(int32_t minutes) {
        Genesis::s_genesisStartupTime = minutes;
        Genesis::s_genesisStartupBlockNum = Genesis:: s_genesisStartupTime * 60/Config::s_maxRoundSeconds;
        ilog("Genesis startup time : ${minutes} minutes, startup block num : ${number}",
                ("minutes",Genesis::s_genesisStartupTime)("number", Genesis::s_genesisStartupBlockNum));
    }

    void UranusNode::setGenesisPk(const std::string& pk) {
        Genesis::s_genesisPk = pk;
        ilog("Genesis pk : ${pk}", ("pk", pk));
    }

    void UranusNode::setRoundAndPhaseSecond(int32_t roundSecond, int32_t phaseSecond) {
        Config::s_maxRoundSeconds = roundSecond;
        Config::s_maxPhaseSeconds = phaseSecond;
        ilog("maxRoundSecond : ${maxRoundSecond}, maxPhaseSecond : ${maxPhaseSecond}",
                ("maxRoundSecond", Config::s_maxRoundSeconds)("maxPhaseSecond", Config::s_maxPhaseSeconds));
    }
    void UranusNode::setTrxsSecond(int32_t trxssecond) {
        Config::s_maxTrxMicroSeconds = trxssecond;

        ilog("s_maxTrxMicroSeconds : ${s_maxTrxMicroSeconds}",
             ("s_maxTrxMicroSeconds", Config::s_maxTrxMicroSeconds));
    }

    void UranusNode::setTimerCanceled(TimerHandlerNumber thn) {
        m_timerCanceledBits |= (1 << thn);
    }

    void UranusNode::resetTimerCanceled(TimerHandlerNumber thn) {
        m_timerCanceledBits &= (~(1 << thn));
    }

    bool UranusNode::isTimerCanceled(TimerHandlerNumber thn) const {
        return m_timerCanceledBits & (1 << thn);
    }
}
