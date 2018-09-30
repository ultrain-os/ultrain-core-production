#include <rpos/Node.h>

#include <chrono>
#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include <fc/log/logger.hpp>

#include <rpos/Config.h>
#include <rpos/Genesis.h>
#include <rpos/MessageBuilder.h>
#include <rpos/MessageManager.h>
#include <rpos/UranusController.h>
#include <rpos/KeyKeeper.h>

// eos net
#include <appbase/application.hpp>
#include <ultrainio/net_plugin/net_plugin.hpp>

using namespace boost::asio;
using namespace std;

namespace ultrainio {
    char version[]="86ce34";

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
                                                                 m_controllerPtr(std::make_shared<UranusController>()) {
        ilog("Code version is ${s}", ("s", version));
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
        m_controllerPtr->enableEventRegister(v);
    }

    void UranusNode::setMyInfoAsCommitteeKey(const std::string& sk, const std::string& account) {
        VoterSystem::getKeyKeeper()->setMyInfoAsCommitteeKey(sk, account);
    }

    bool UranusNode::getNonProducingNode() const {
        return m_isNonProducingNode;
    }

    void UranusNode::reset() {
        dlog("reset node cache.");
        m_phase = kPhaseInit;
        m_baxCount = 0;
        m_controllerPtr->reset();
    }

    void UranusNode::readyToConnect() {
        m_connected = true;
        readyLoop(6 * Config::s_maxRoundSeconds);
    }

    bool UranusNode::getSyncingStatus() const {
        return m_syncing;
    }

    void UranusNode::init() {
        m_ready = false;
        m_connected = false;
        m_syncing = false;
        m_syncFailed = false;
        m_phase = kPhaseInit;
        m_baxCount = 0;
        m_controllerPtr->init();
    }

    void UranusNode::readyToJoin() {
        boost::chrono::system_clock::time_point current_time = boost::chrono::system_clock::now();
        boost::chrono::seconds pass_time_to_genesis;

        if (!m_connected) {
            //readyToConnect();
            //return;
            m_connected = true;
        }

        std::time_t t = boost::chrono::system_clock::to_time_t(current_time);
        ilog("readyToJoin current_time ${t}", ("t", std::ctime(&t)));

        if (current_time < Genesis::s_time) {
            pass_time_to_genesis = boost::chrono::duration_cast<boost::chrono::seconds>(Genesis::s_time - current_time);

            if (pass_time_to_genesis.count() > Config::s_maxRoundSeconds) {
                readyLoop(Config::s_maxRoundSeconds);
            } else if (pass_time_to_genesis.count() == 0) {
                m_ready = true;
                run();
            } else {
                readyLoop(pass_time_to_genesis.count());
            }
        } else if (Genesis::s_time == current_time) {
            m_ready = true;
            run();
        } else {
            pass_time_to_genesis = boost::chrono::duration_cast<boost::chrono::seconds>(current_time - Genesis::s_time);
            if (pass_time_to_genesis.count() == 0) {
                m_ready = true;
                run();
            } else {
                m_phase = kPhaseInit;
                applyBlock();
            }
        }
    }

    void UranusNode::preRunBa0BlockStep() {
        if (m_phase != kPhaseBA1)
            return;

        bool ret = m_controllerPtr->preRunBa0BlockStep();
        if (ret) {
            preRunBa0BlockLoop(1);
        }
    }

    void UranusNode::preRunBa0BlockLoop(uint32_t timeout) {
        m_preRunTimer.expires_from_now(boost::posix_time::seconds(timeout));
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
        m_timer.async_wait([this](boost::system::error_code ec) {
            if (ec.value() == boost::asio::error::operation_aborted) {
                ilog("ready loop timer cancel");
            } else {
                this->readyToJoin();
            }
        });
    }

    void UranusNode::applyBlockLoop(uint32_t timeout) {
        m_timer.expires_from_now(boost::posix_time::seconds(timeout));
        m_timer.async_wait([this](boost::system::error_code ec) {
            if (ec.value() == boost::asio::error::operation_aborted) {
                ilog("apply block timer cancel");
            } else {
                this->applyBlock();
            }
        });
    }

    void UranusNode::applyBlockOnce() {
        applyBlock(true);
    }

    void UranusNode::applyBlock() {
        applyBlock(false);
    }

    void UranusNode::applyBlock(bool once) {
        SyncRequestMessage msg;
        dlog("@@@@@@@@@@@ applyBlock syncing:${s}", ("s", m_syncing));
        if (m_syncing) {
            applyBlockLoop(getRoundInterval());
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

        dlog("apply block. start id = ${sid}  end id = ${eid}", ("sid", msg.startBlockNum)("eid", msg.endBlockNum));
        if (!sendMessage(msg)) {
            elog("@applyBlock send sync request msg failed!!!");
            m_syncing = false;
            m_syncFailed = true;
        }

        if (!once) {
            applyBlockLoop(getRoundInterval());
        }
    }

    void UranusNode::ba0Process() {
        if (!m_ready) {
            applyBlock();
            return;
        }

        dlog("ba0Process begin. blockNum = ${id}.", ("id", getBlockNum()));

        //to_do hash error process
        Block ba0Block = m_controllerPtr->produceTentativeBlock();
        //monitor begin, record ba0 block producing time
        if(ba0Callback != nullptr) {
            ba0Callback();
        }
        //monitor end
        m_controllerPtr->setBa0Block(ba0Block);
        if ((!isBlank(ba0Block.id()))
            && (ba0Block.previous != m_controllerPtr->getPreviousBlockhash())) {

            elog("ba0Process error. previous block hash error. hash = ${hash1} local hash = ${hash2}",
                 ("hash1", ba0Block.previous)("hash2", m_controllerPtr->getPreviousBlockhash()));

            ULTRAIN_ASSERT(false, chain::chain_exception, "DB error. please reset with cmd --delete-all-blocks.");
            //m_ready = false;
            //applyBlock();
            return;
        }

        msgkey msg_key;
        ba1Loop(getRoundInterval());

        m_controllerPtr->resetEcho();
        m_phase = kPhaseBA1;
        m_baxCount = 0;

        MessageManager::getInstance()->moveToNewStep(getBlockNum(), kPhaseBA1, 0);

        dlog("############## ba0 finish blockNum = ${id}, host_name = ${host_name}",
             ("id", getBlockNum())("host_name", boost::asio::ip::host_name()));
        dlog("ba0Process voter.hash = ${hash1}",("hash1", ba0Block.id()));
        dlog("ba0Process. prepare ba1. blockNum = ${blockNum}, isVoter = ${isVoter}.", ("blockNum", getBlockNum())
                ("isVoter",MessageManager::getInstance()->isVoter(getBlockNum(), kPhaseBA1, 0)));

        vote(getBlockNum(),kPhaseBA1,0);

        msg_key.blockNum = getBlockNum();
        msg_key.phase = m_phase;
        m_controllerPtr->processCache(msg_key);
        // Only producing node will pre-run proposed block, non-producing node still
        // try to run trx asap.
        if (!MessageManager::getInstance()->isVoter(getBlockNum(), m_phase, m_baxCount) && !m_isNonProducingNode) {
            bool ret = m_controllerPtr->preRunBa0BlockStart();
            if (ret) {
                preRunBa0BlockLoop(1);
            }
        }
    }

    void UranusNode::vote(uint32_t blockNum, ConsensusPhase phase, uint32_t baxCount) {
        const Block* ba0Block = nullptr;

        dlog("vote. blockNum = ${blockNum} phase = ${phase} baxCount = ${cnt}", ("blockNum", blockNum)
                ("phase",uint32_t(phase))("cnt",baxCount));

        if (kPhaseBA0 == phase) {
            if (MessageManager::getInstance()->isProposer(blockNum)) {
                ProposeMsg propose;
                bool ret = m_controllerPtr->initProposeMsg(&propose);
                ULTRAIN_ASSERT(ret, chain::chain_exception, "Init propose msg failed");
                dlog("vote.propose.block_hash : ${block_hash}", ("block_hash", propose.block.id()));
                m_controllerPtr->insert(propose);
                sendMessage(propose);
                if (MessageManager::getInstance()->isVoter(getBlockNum(), kPhaseBA0, 0)) {
                    EchoMsg echo = MessageBuilder::constructMsg(propose);
                    m_controllerPtr->insert(echo);
                    dlog("vote. echo.block_hash : ${block_hash}", ("block_hash", echo.blockId));
                    sendMessage(echo);
                }
            }
            return;
        }

        if (MessageManager::getInstance()->isVoter(blockNum, phase, baxCount)) {
            ba0Block = m_controllerPtr->getBa0Block();
            if (isEmpty(ba0Block->id())) {
                elog("vote ba0Block is empty, and send echo for empty block");
                sendEchoForEmptyBlock();
            } else if (m_controllerPtr->verifyBa0Block()) { // not empty, verify
                EchoMsg echo = MessageBuilder::constructMsg(*ba0Block);
                m_controllerPtr->insert(echo);
                //echo.timestamp = getRoundCount();
                dlog("vote. echo.block_hash : ${block_hash}", ("block_hash", echo.blockId));
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
        m_phase = kPhaseInit;
        Block ba1Block = m_controllerPtr->produceTentativeBlock();
        //monitor begin, record ba1 block producing time
        if(ba1Callback != nullptr) {
            ba1Callback();
        }
        //monitor end

        dlog("ba1Process begin. blockNum = ${id}.", ("id", getBlockNum()));

        if ((!isBlank(ba1Block.id()))
            && (ba1Block.previous != m_controllerPtr->getPreviousBlockhash())) {

            elog("ba1Process error. previous block hash error. hash = ${hash1} local hash = ${hash2}",
                 ("hash1", ba1Block.previous)("hash2", m_controllerPtr->getPreviousBlockhash()));

            ULTRAIN_ASSERT(false, chain::chain_exception, "DB error. please reset with cmd --delete-all-blocks.");
            return;
        }

        std::shared_ptr<Block> blockPtr = std::make_shared<chain::signed_block>();
        if (!isBlank(ba1Block.id())) {
            //UltrainLog::display_block(ba1_block);
            *blockPtr = ba1Block;
        } else {
            elog("ba1Process ba1 finish. block is blank. phase ba2 begin.");
            //init();
            //readyToJoin();
            m_phase = kPhaseBA1;
            baxLoop(getRoundInterval());
            return;
        }

        dlog("##############ba1Process. finish blockNum = ${block_num}, hash = ${hash}, head_hash = ${head_hash}",
             ("block_num", getBlockNum())
                     ("hash", blockPtr->id())
                     ("head_hash", m_controllerPtr->getPreviousBlockhash()));
        m_controllerPtr->produceBlock(blockPtr);

        ULTRAIN_ASSERT(blockPtr->id() == m_controllerPtr->getPreviousBlockhash(),
                       chain::chain_exception, "Produced block hash is not expected");
        run();
    }

    uint32_t UranusNode::isSyncing() {
        return m_controllerPtr->isSyncing();
    }

    void UranusNode::baxProcess() {
        ilog("In baxProcess");
        Block baxBlock = m_controllerPtr->produceTentativeBlock();
        signed_block_ptr uranus_block = std::make_shared<chain::signed_block>();

        if (m_phase == kPhaseInit) {
            dlog("baxProcess finish.apply block ok. blockNum = ${id}, m_syncing = ${m_syncing}.",
                 ("id", getBlockNum() - 1)("m_syncing", (uint32_t) m_syncing));

            applyBlock();
            return;
        }

        //fast into baxcount 20
        if ((m_baxCount < (Config::kMaxBaxCount - m_phase))
            && (m_controllerPtr->isChangePhase())) {
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
                         ("hash", uranus_block->id())
                         ("head_hash", m_controllerPtr->getPreviousBlockhash()));

            m_controllerPtr->produceBlock(uranus_block);

            ULTRAIN_ASSERT(uranus_block->id() == m_controllerPtr->getPreviousBlockhash(),
                           chain::chain_exception, "Produced block hash is not expected");

            fastBlock(getBlockNum());
            //join();
        } else {
            elog("baxProcess.phase bax finish. block is blank.");

            if (INVALID_BLOCK_NUM != isSyncing()) {
                dlog("baxProcess. syncing begin. m_baxCount = ${count}.", ("count", m_baxCount));
                applyBlockOnce();
                //join();
            }
            //init();
            //readyToJoin();
            uint32_t blockNum = getBlockNum();
            if (m_baxCount > 0 && m_baxCount % 5 == 0 && blockNum > 2) {
                uint32_t preBlockNum = blockNum - 1;
                if (MessageManager::getInstance()->isProposer(preBlockNum)) {
                    std::shared_ptr<AggEchoMsg> aggEchoMsg = MessageManager::getInstance()->getMyAggEchoMsg(preBlockNum);
                    if (aggEchoMsg) {
                        sendMessage(*aggEchoMsg);
                    }
                }
            }

            baxLoop(getRoundInterval());
        }
    }

    void UranusNode::runLoop(uint32_t timeout) {
        dlog("start runLoop timeout = ${timeout}", ("timeout", timeout));
        m_timer.expires_from_now(boost::posix_time::seconds(timeout));
        m_timer.async_wait([this](boost::system::error_code ec) {
            if (ec.value() == boost::asio::error::operation_aborted) {
                ilog("run loop timer cancel");
            } else {
                this->run();
            }
        });
    }

    void UranusNode::ba0Loop(uint32_t timeout) {
        dlog("start ba0Loop timeout = ${timeout}", ("timeout", timeout));
        m_timer.expires_from_now(boost::posix_time::seconds(timeout));
        m_timer.async_wait([this](boost::system::error_code ec) {
            if (ec.value() == boost::asio::error::operation_aborted) {
                ilog("ba0 loop timer cancel");
            } else {
                this->ba0Process();
            }
        });
    }

    void UranusNode::ba1Loop(uint32_t timeout) {
        dlog("start ba1Loop timeout = ${timeout}", ("timeout", timeout));
        m_timer.expires_from_now(boost::posix_time::seconds(timeout));
        m_timer.async_wait([this](boost::system::error_code ec) {
            if (ec.value() == boost::asio::error::operation_aborted) {
                ilog("ba1 loop timer cancel");
            } else {
                this->ba1Process();
            }
        });
    }

    void UranusNode::baxLoop(uint32_t timeout) {
        EchoMsg echo_msg;
        msgkey msg_key;

        dlog("start baxLoop timeout = ${timeout}", ("timeout", timeout));
        m_timer.expires_from_now(boost::posix_time::seconds(timeout));
        m_timer.async_wait([this](boost::system::error_code ec) {
            if (ec.value() == boost::asio::error::operation_aborted) {
                dlog("bax loop timer cancel");
            } else {
                this->baxProcess();
            }
        });

        m_controllerPtr->moveEchoMsg2AllPhaseMap();
        m_controllerPtr->resetEcho();
        m_controllerPtr->clearPreRunStatus();
        m_phase = kPhaseBAX;
        m_baxCount++;
        MessageManager::getInstance()->moveToNewStep(getBlockNum(), kPhaseBAX, m_baxCount);
        dlog("bax loop. Voter = ${Voter}, m_baxCount = ${count}.",
             ("Voter", MessageManager::getInstance()->isVoter(getBlockNum(), kPhaseBAX, m_baxCount))
                     ("count",m_baxCount));

        if ((m_baxCount + m_phase) >= Config::kMaxBaxCount) {
            if (MessageManager::getInstance()->isVoter(getBlockNum(), kPhaseBAX, m_baxCount)) {
                sendEchoForEmptyBlock();
            }
        } else {
            vote(getBlockNum(),kPhaseBAX,m_baxCount);
        }

        msg_key.blockNum = getBlockNum();
        msg_key.phase = m_phase;
        msg_key.phase += m_baxCount;
        m_controllerPtr->processCache(msg_key);
    }

    bool UranusNode::handleMessage(const EchoMsg &echo) {
        return m_controllerPtr->handleMessage(echo);
    }

    bool UranusNode::handleMessage(const ProposeMsg &propose) {
        return m_controllerPtr->handleMessage(propose);
    }

    bool UranusNode::handleMessage(const std::string &peer_addr, const SyncRequestMessage &msg) {
        return m_controllerPtr->handleMessage(peer_addr, msg);
    }

    bool UranusNode::handleMessage(const string &peer_addr, const ReqLastBlockNumMsg &msg) {
        return m_controllerPtr->handleMessage(peer_addr, msg);
    }

    uint32_t UranusNode::getLastBlocknum() {
        return m_controllerPtr->getLastBlocknum();
    }

    bool UranusNode::handleMessage(const Block &msg, bool last_block) {
        uint32_t next_blockNum = 0;
        if (!m_syncing) {
            return true;
        }

        bool result = m_controllerPtr->handleMessage(msg);
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
                dlog("handleMessage. close bax, blockNum = ${blockNum}.", ("blockNum", getLastBlocknum()));
                reset();
            }
        }

        return true;
    }

    bool UranusNode::syncFail(const ultrainio::SyncRequestMessage& sync_msg) {
        m_syncFailed = true;
        m_ready = true;
        m_syncing = false;

        if (sync_msg.startBlockNum == sync_msg.endBlockNum && sync_msg.endBlockNum == getLastBlocknum() + 1) {
            ilog("Fail to sync block from ${s} to ${e}, but there has been already ${last} blocks in local.",
                 ("s", sync_msg.startBlockNum)("e", sync_msg.endBlockNum)("last", getLastBlocknum()));
            if (VoterSystem::committeeHasWorked()) {
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
    }

    void UranusNode::sendMessage(const EchoMsg &echo) {
        //echo.timestamp = getRoundCount();
        app().get_plugin<net_plugin>().broadcast(echo);
    }

    void UranusNode::sendMessage(const ProposeMsg &propose) {
        //propose.timestamp = getRoundCount();
        app().get_plugin<net_plugin>().broadcast(propose);
    }

    void UranusNode::sendMessage(const string &peer_addr, const Block &msg) {
        app().get_plugin<net_plugin>().send_block(peer_addr, msg);
    }

    void UranusNode::sendMessage(const AggEchoMsg& aggEchoMsg) {
        app().get_plugin<net_plugin>().broadcast(aggEchoMsg);
    }

    bool UranusNode::sendMessage(const SyncRequestMessage &msg) {
        return app().get_plugin<net_plugin>().send_apply(msg);
    }

    void UranusNode::sendMessage(const std::string &peer_addr, const RspLastBlockNumMsg &msg) {
        app().get_plugin<net_plugin>().send_last_block_num(peer_addr, msg);
    }

    bool UranusNode::isFastBlock() {
        msgkey msg_key;
        msg_key.blockNum = getBlockNum();
        msg_key.phase = kPhaseBA0;

        if ((m_controllerPtr->findProposeCache(msg_key))
            && (m_controllerPtr->findEchoCache(msg_key))) {
            msg_key.phase = kPhaseBA1;
            if (m_controllerPtr->findEchoCache(msg_key)) {
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
        ba0Loop(getRoundInterval());
        MessageManager::getInstance()->moveToNewStep(getBlockNum(), kPhaseBA0, 0);

        dlog("start BA0. blockNum = ${blockNum}. isProposer = ${isProposer} and isVoter = ${isVoter}",
             ("blockNum", getBlockNum())("isProposer", MessageManager::getInstance()->isProposer(getBlockNum()))
                     ("isVoter", MessageManager::getInstance()->isVoter(getBlockNum(), kPhaseBA0, 0)));
        msg_key.blockNum = getBlockNum();
        msg_key.phase = m_phase;
        m_controllerPtr->processCache(msg_key);

        if (voteFlag) {
            vote(getBlockNum(), kPhaseBA0, 0);
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
        m_controllerPtr->fastProcessCache(msg_key);
    }

    void UranusNode::fastBa1() {
        msgkey msg_key;
        signed_block_ptr uranus_block = std::make_shared<chain::signed_block>();

        Block ba0Block = m_controllerPtr->produceTentativeBlock();
        m_controllerPtr->setBa0Block(ba0Block);

        m_phase = kPhaseBA1;
        m_baxCount = 0;
        m_controllerPtr->resetEcho();

        msg_key.blockNum = getBlockNum();
        msg_key.phase = m_phase;
        msg_key.phase += m_baxCount;
        m_controllerPtr->fastProcessCache(msg_key);

        dlog("fastBa1 begin. blockNum = ${id}", ("id", getBlockNum()));

        Block baxBlock = m_controllerPtr->produceTentativeBlock();
        if (!isBlank(baxBlock.id())) {
            msg_key.blockNum = getBlockNum() + 1;
            msg_key.phase = kPhaseBA0;

            if (m_controllerPtr->findEchoCache(msg_key)) {
                *uranus_block = baxBlock;

                dlog("##############fastBa1.finish blockNum = ${id}, hash = ${hash}",
                     ("id", getBlockNum())("hash", uranus_block->id()));
                m_controllerPtr->produceBlock(uranus_block);

                fastBlock(msg_key.blockNum);
            } else {
                if ((getRoundInterval() == Config::s_maxPhaseSeconds) && (isProcessNow())) {
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
            if (m_controllerPtr->findEchoCache(msg_key)) {
                fastBax();
            } else {
                if ((getRoundInterval() == Config::s_maxPhaseSeconds) && (isProcessNow())) {
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

        m_controllerPtr->resetEcho();

        //fast into baxcount 20
        if ((m_baxCount < (Config::kMaxBaxCount - m_phase))
            && (m_controllerPtr->isChangePhase())) {
            m_baxCount = Config::kMaxBaxCount - m_phase;
            dlog("fastBax.ChangePhase to baxcount[20]. blockNum = ${id}, m_baxCount = ${phase}", ("id", getBlockNum())("phase", m_baxCount));
        }

        msg_key.blockNum = getBlockNum();
        msg_key.phase = m_phase;
        msg_key.phase += m_baxCount;
        m_controllerPtr->fastProcessCache(msg_key);
        Block baxBlock = m_controllerPtr->produceTentativeBlock();

        dlog("fastBax begin. blockNum = ${id}, phase = ${phase}", ("id", getBlockNum())("phase", msg_key.phase));

        if (!isBlank(baxBlock.id())) {
            msg_key.blockNum = getBlockNum() + 1;
            msg_key.phase = kPhaseBA0;

            if (m_controllerPtr->findEchoCache(msg_key)) {
                *uranus_block = baxBlock;

                m_controllerPtr->produceBlock(uranus_block);
                dlog("##############finish blockNum = ${id}, hash = ${hash}",
                     ("id", getBlockNum())("hash", uranus_block->id()));

                fastBlock(msg_key.blockNum);
            } else {
                //todo addjust
                baxLoop(getRoundInterval());
            }
        } else {
            msg_key.blockNum = getBlockNum();
            msg_key.phase = m_phase + m_baxCount + 1;
            if (m_controllerPtr->findEchoCache(msg_key)) {
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

        MessageManager::getInstance()->moveToNewStep(getBlockNum(), kPhaseBA0, 0);
        reset();
        m_controllerPtr->resetTimestamp();

        msg_key.blockNum = getBlockNum();
        msg_key.phase = kPhaseBA0;
        if (m_controllerPtr->findEchoCache(msg_key)) {
            fastBa0();
        } else {
            run();
            return;
        }

        msg_key.phase = kPhaseBA1;

        if (m_controllerPtr->findEchoCache(msg_key)) {
            fastBa1();
        } else {
            if ((getRoundInterval() == Config::s_maxPhaseSeconds) && (isProcessNow())) {
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
             ("id1", getRoundCount())("id2",m_controllerPtr->getFastTimestamp()));

        if ((m_controllerPtr->getFastTimestamp() < getRoundCount()) && (m_controllerPtr->getFastTimestamp() != 0)) {
            return true;
        }

        return false;
    }

    uint32_t UranusNode::getRoundCount() {
        boost::chrono::system_clock::time_point current_time = boost::chrono::system_clock::now();
        boost::chrono::seconds pass_time_to_genesis
                = boost::chrono::duration_cast<boost::chrono::seconds>(current_time - Genesis::s_time);

        dlog("getRoundCount. count = ${id}.",
             ("id", pass_time_to_genesis.count() / Config::s_maxPhaseSeconds));

        return pass_time_to_genesis.count() / Config::s_maxPhaseSeconds;
    }

    uint32_t UranusNode::getRoundInterval() {
        boost::chrono::system_clock::time_point current_time = boost::chrono::system_clock::now();
        boost::chrono::seconds pass_time_to_genesis
                = boost::chrono::duration_cast<boost::chrono::seconds>(current_time - Genesis::s_time);

        dlog("getRoundInterval. interval = ${id}.",
             ("id", Config::s_maxPhaseSeconds - (pass_time_to_genesis.count() % Config::s_maxPhaseSeconds)));

        return Config::s_maxPhaseSeconds - (pass_time_to_genesis.count() % Config::s_maxPhaseSeconds);
    }

    BlockIdType UranusNode::getPreviousHash() {
        return m_controllerPtr->getPreviousBlockhash();
    }

    const std::shared_ptr<UranusController> UranusNode::getController() const {
        return std::shared_ptr<UranusController> (m_controllerPtr);
    }

    bool UranusNode::isBlank(const BlockIdType& blockId) {
        return m_controllerPtr->isBlank(blockId);
    }

    bool UranusNode::isEmpty(const BlockIdType& blockId) {
        return m_controllerPtr->isEmpty(blockId);
    }

    void UranusNode::sendEchoForEmptyBlock() {
        Block block = m_controllerPtr->emptyBlock();
        dlog("vote empty block. blockNum = ${blockNum} hash = ${hash}", ("blockNum",getBlockNum())("hash", block.id()));
        EchoMsg echoMsg = MessageBuilder::constructMsg(block);
        m_controllerPtr->insert(echoMsg);
        //echoMsg.timestamp = getRoundCount();
        sendMessage(echoMsg);
    }

    int UranusNode::getCommitteeMemberNumber() {
        std::shared_ptr<VoterSystem> voterSysPtr = MessageManager::getInstance()->getVoterSys(this->getBlockNum());
        return voterSysPtr->getCommitteeMemberNumber();
    }

    void UranusNode::setGenesisTime(const boost::chrono::system_clock::time_point& tp) {
        Genesis::s_time = tp;
        std::time_t t = boost::chrono::system_clock::to_time_t(Genesis::s_time);
        ilog("Genesis time is ${t}", ("t", std::ctime(&t)));
    }

    void UranusNode::setGenesisStartupTime(int32_t minutes) {
        Genesis::s_genesisStartupTime = minutes;
        Genesis::s_genesisStartupBlockNum = Genesis:: s_genesisStartupTime * Config::kAverageBlockPerMinutes;
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
}
