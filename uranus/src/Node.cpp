#include <uranus/Node.h>

#include <chrono>
#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include <fc/log/logger.hpp>

#include <log/Log.h>
#include <uranus/MessageManager.h>
#include <uranus/UranusController.h>

// eos net
#include <appbase/application.hpp>
#include <ultrainio/net_plugin/net_plugin.hpp>

using namespace boost::asio;
using namespace std;

namespace {
    bool EmptyBlock(const ultrainio::chain::block_header &block) {
        if (!block.proposerPk.empty() || !block.proposerProof.empty())
            return false;
        return true;
    }
}

namespace ultrainio {
    const int UranusNode::MAX_ROUND_SECONDS = 10;
    const int UranusNode::MAX_PHASE_SECONDS = 5;
    uint8_t UranusNode::URANUS_PUBLIC_KEY[VRF_PUBLIC_KEY_LEN] = {0};
    uint8_t UranusNode::URANUS_PRIVATE_KEY[VRF_PRIVATE_KEY_LEN] = {0};

    boost::chrono::system_clock::time_point UranusNode::GENESIS;

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
    }

    bool UranusNode::verifyRole(uint32_t blockNum, uint16_t phase, const std::string &role_proof,
                                const std::string &pk) {
        if (role_proof.empty()) {
            return false;
        }
        char msg[64] = {0};
        snprintf(msg, 64, "%d%d", blockNum, phase);
        if (Vrf::verify((const uint8_t *) msg, strlen(msg), (const uint8_t *) role_proof.data(),
                                 (const uint8_t *) pk.data())) {
            return true;
        }
        return false;
    }

    bool UranusNode::startup() {
        ilog("UranusNode starts as producing node: ${b}", ("b", !m_isNonProducingNode));
        if (!Vrf::keypair(UranusNode::URANUS_PUBLIC_KEY, UranusNode::URANUS_PRIVATE_KEY)) {
            //elog("generate key error.");
            return false;
        }
        LOG_INFO << "node startup pk : "
                 << UltrainLog::convert2Hex(std::string((char *) UranusNode::URANUS_PUBLIC_KEY, VRF_PUBLIC_KEY_LEN))
                 << std::endl;
        return true;
    }

    void UranusNode::reset() {
        m_phase = kPhaseInit;
        m_baxCount = 0;
        m_controllerPtr->reset();
    }

    void UranusNode::readyToConnect() {
        m_connected = true;
        LOG_INFO << "ready_to_connect time = " << 6 * MAX_ROUND_SECONDS << std::endl;
        readyLoop(6 * MAX_ROUND_SECONDS);
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
        uint32_t timeval;
        boost::chrono::seconds pass_time_to_genesis;

        if (!m_connected) {
            readyToConnect();
            return;
        }

        if (current_time < GENESIS) {
            pass_time_to_genesis = boost::chrono::duration_cast<boost::chrono::seconds>(GENESIS - current_time);

            if (pass_time_to_genesis.count() > MAX_ROUND_SECONDS) {
                LOG_INFO << "ready_loop timer = 10. interval = " << pass_time_to_genesis.count() << std::endl;
                readyLoop(MAX_ROUND_SECONDS);
            } else if (pass_time_to_genesis.count() == 0) {
                m_ready = true;
                LOG_INFO << "genesis block " << std::endl;
                run();
            } else {
                LOG_INFO << "ready_loop timer =  " << pass_time_to_genesis.count() << std::endl;
                readyLoop(pass_time_to_genesis.count());
            }
        } else if (GENESIS == current_time) {
            m_ready = true;
            LOG_INFO << "genesis block " << std::endl;
            run();
        } else {
            pass_time_to_genesis = boost::chrono::duration_cast<boost::chrono::seconds>(current_time - GENESIS);
            if (pass_time_to_genesis.count() == 0) {
                m_ready = true;
                LOG_INFO << "genesis block " << std::endl;
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
        BlockRecord ba0_block = m_controllerPtr->produceTentativeBlock();
        if ((!EmptyBlock(ba0_block.block))
            && (ba0_block.block.previous != m_controllerPtr->getPreviousBlockhash())) {

            elog("error. previous block hash error. re-join.hash = ${hash1} local hash = ${hash2}",
                 ("hash1", ba0_block.block.previous)
                         ("hash2", m_controllerPtr->getPreviousBlockhash()));
            LOG_ERROR << "error. previous block hash error. re-join." << std::endl;
            m_ready = false;
            applyBlock();
            return;
        }

        msgkey msg_key;
        ba1Loop(getRoundInterval());

        m_controllerPtr->resetEcho();
        m_phase = kPhaseBA1;
        m_baxCount = 0;

        dlog("start BA1. blockNum = ${blockNum}.", ("blockNum", getBlockNum()));
        MessageManager::getInstance()->moveToNewStep(getBlockNum(), kPhaseBA1, 0);

        LOG_INFO << "start BA1. isVoter = " << MessageManager::getInstance()->isVoter(getBlockNum(), kPhaseBA1, 0)
                 << std::endl;
        if (MessageManager::getInstance()->isVoter(getBlockNum(), kPhaseBA1, 0)) {
            if (EmptyBlock(ba0_block.block)) {
                elog("error. tentative block is empty.");
                LOG_ERROR << "error. tentative block is empty" << std::endl;
            } else {
                if (m_controllerPtr->verifyBa0Block()) {
                    EchoMsg echo = m_controllerPtr->constructMsg(ba0_block.block);
                    m_controllerPtr->insert(echo);
                    sendMessage(echo);
                } else {
                    elog("#################Can't verify ba0 block");
                }
            }
        }

        dlog("############## ba0 finish blockNum = ${id}, host_name = ${host_name}",
             ("id", getBlockNum())("host_name", boost::asio::ip::host_name()));

        LOG_INFO << "checkpoint: blockNum:" << getBlockNum() << ";phase:" << m_phase << ";host_name:"
                 << boost::asio::ip::host_name() << std::endl;
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

    void UranusNode::ba1Process() {
        // produce block
        m_phase = kPhaseInit;
        BlockRecord ba1_block = m_controllerPtr->produceTentativeBlock();
        signed_block_ptr uranus_block = std::make_shared<chain::signed_block>();
        msgkey msg_key;

        dlog("ba1Process begin. blockNum = ${id}.", ("id", getBlockNum()));

        if (!EmptyBlock(ba1_block.block)) {
            //UltrainLog::display_block(ba1_block);
            *uranus_block = ba1_block.block;
        } else {
            elog("ba1Process ba1 finish. block is empty. phase ba2 begin.");
            //init();
            //readyToJoin();
            baxLoop(getRoundInterval());
            return;
        }

        m_controllerPtr->produceBlock(uranus_block);
        dlog("##############finish blockNum = ${block_num}, hash = ${hash}, head_hash = ${head_hash}",
             ("block_num", getLastBlocknum())
                     ("hash", uranus_block->id())
                     ("head_hash", m_controllerPtr->getPreviousBlockhash()));
        ULTRAIN_ASSERT(uranus_block->id() == m_controllerPtr->getPreviousBlockhash(),
                       chain::chain_exception, "Produced block hash is not expected");

        LOG_INFO << "checkpoint: blockNum:" << getBlockNum() << ";phase:" << m_phase << ";host_name:"
                 << boost::asio::ip::host_name() << ";block_hash_previous: " << uranus_block->previous << ";txs_hash: "
                 << uranus_block->id() << std::endl;
        run();
    }

    uint32_t UranusNode::isSyncing() {
        return m_controllerPtr->isSyncing();
    }

    void UranusNode::baxProcess() {
        BlockRecord bax_block = m_controllerPtr->produceTentativeBlock();
        signed_block_ptr uranus_block = std::make_shared<chain::signed_block>();
        msgkey msg_key;

        if (m_phase == kPhaseInit) {
            dlog("baxProcess finish.apply block ok. blockNum = ${id}, m_syncing = ${m_syncing}.",
                 ("id", getBlockNum() - 1)("m_syncing", (uint32_t) m_syncing));

            applyBlock();
            return;
        }

        if (!EmptyBlock(bax_block.block)) {
            m_ready = true;
            m_syncing = false;
            m_syncFailed = false;
            app().get_plugin<net_plugin>().stop_sync_block();
            //UltrainLog::display_block(bax_block);
            *uranus_block = bax_block.block;

            m_controllerPtr->produceBlock(uranus_block);
            dlog("##############finish blockNum = ${block_num}, hash = ${hash}, head_hash = ${head_hash}",
                 ("block_num", getLastBlocknum())
                         ("hash", uranus_block->id())
                         ("head_hash", m_controllerPtr->getPreviousBlockhash()));
            ULTRAIN_ASSERT(uranus_block->id() == m_controllerPtr->getPreviousBlockhash(),
                           chain::chain_exception, "Produced block hash is not expected");

            LOG_INFO << "checkpoint: blockNum:" << getBlockNum() << ";phase:" << m_phase << ";host_name:"
                     << boost::asio::ip::host_name() << ";block_hash_previous: " << uranus_block->previous
                     << ";txs_hash: " << uranus_block->id() << std::endl;
            fastBlock(getBlockNum());
            //join();
        } else {
            elog("baxProcess.phase bax finish. block is empty.");

            if (INVALID_BLOCK_NUM != isSyncing()) {
                dlog("baxProcess. syncing begin. count = ${count}.", ("count", m_baxCount));
                applyBlockOnce();
                //join();
            }
            //init();
            //readyToJoin();
            baxLoop(getRoundInterval());
        }
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
        const BlockRecord *ba0_block = nullptr;
        EchoMsg echo_msg;

        dlog("start baxLoop timeout = ${timeout}", ("timeout", timeout));
        m_timer.expires_from_now(boost::posix_time::seconds(timeout));
        m_timer.async_wait([this](boost::system::error_code ec) {
            if (ec.value() == boost::asio::error::operation_aborted) {
                ilog("bax loop timer cancel");
            } else {
                this->baxProcess();
            }
        });

        m_controllerPtr->saveEchoMsg();
        m_controllerPtr->resetEcho();
        m_controllerPtr->clearPreRunStatus();
        m_phase = kPhaseBAX;
        m_baxCount++;
        MessageManager::getInstance()->moveToNewStep(getBlockNum(), kPhaseBAX, m_baxCount);
        dlog("bax loop. Voter = ${Voter}, count = ${count}.",
             ("Voter", MessageManager::getInstance()->isVoter(getBlockNum(), kPhaseBAX, m_baxCount))("count",
                                                                                                     m_baxCount));

        if (MessageManager::getInstance()->isVoter(getBlockNum(), kPhaseBAX, m_baxCount)) {
            ba0_block = m_controllerPtr->getBa0Block();
            if (EmptyBlock(ba0_block->block)) {
                elog("error.kVoter tentative block is empty.");
            } else {
                if (m_controllerPtr->verifyBa0Block()) {
                    echo_msg = m_controllerPtr->constructMsg(ba0_block->block);
                    sendMessage(echo_msg);
                } else {
                    elog("kPhaseBAX  voter Can't verify ba0 block");
                }
            }
        }
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

    bool UranusNode::syncFail() {
        m_syncFailed = true;
        m_ready = true;
        m_syncing = false;
        return true;
    }

    void UranusNode::cancelTimer() {
        m_timer.cancel();
    }

    void UranusNode::sendMessage(const EchoMsg &echo) {
        app().get_plugin<net_plugin>().broadcast(echo);
    }

    void UranusNode::sendMessage(const ProposeMsg &propose) {
        app().get_plugin<net_plugin>().broadcast(propose);
    }

    void UranusNode::sendMessage(const string &peer_addr, const Block &msg) {
        app().get_plugin<net_plugin>().send_block(peer_addr, msg);
    }

    bool UranusNode::sendMessage(const SyncRequestMessage &msg) {
        return app().get_plugin<net_plugin>().send_apply(msg);
    }

    void UranusNode::sendMessage(const std::string &peer_addr, const RspLastBlockNumMsg &msg) {
        app().get_plugin<net_plugin>().send_last_block_num(peer_addr, msg);
    }

    void UranusNode::run() {
        msgkey msg_key;

        reset();

        // BA0=======
        m_phase = kPhaseBA0;
        m_baxCount = 0;
        ba0Loop(getRoundInterval());
        MessageManager::getInstance()->moveToNewStep(getBlockNum(), kPhaseBA0, 0);

        LOG_INFO << "start BA0. " << std::endl;
        dlog("start BA0. blockNum = ${blockNum}. isProposer = ${isProposer} and isVoter = ${isVoter}",
             ("blockNum", getBlockNum())("isProposer", MessageManager::getInstance()->isProposer(getBlockNum()))
                     ("isVoter", MessageManager::getInstance()->isVoter(getBlockNum(), kPhaseBA0, 0)));
        msg_key.blockNum = getBlockNum();
        msg_key.phase = m_phase;
        m_controllerPtr->processCache(msg_key);

        if (MessageManager::getInstance()->isProposer(getBlockNum())) {
            ProposeMsg propose;
            bool ret = m_controllerPtr->initProposeMsg(&propose);
            ULTRAIN_ASSERT(ret, chain::chain_exception, "Init propose msg failed");
            ilog("txs_hash : ${txs_hash}", ("txs_hash", propose.block.id()));
            m_controllerPtr->insert(propose);
            sendMessage(propose);
            if (MessageManager::getInstance()->isVoter(getBlockNum(), kPhaseBA0, 0)) {
                EchoMsg echo = m_controllerPtr->constructMsg(propose);
                m_controllerPtr->insert(echo);
                sendMessage(echo);
            }
            LOG_INFO << "checkpoint: blockNum:" << getBlockNum() << ";phase:" << m_phase << ";role:" << kProposer
                     << ";host_name:" << boost::asio::ip::host_name() << ";txs_hash: " << propose.block.id()
                     << std::endl;
        } else {
            LOG_INFO << "checkpoint: blockNum:" << getBlockNum() << ";phase:" << m_phase << ";host_name:"
                     << boost::asio::ip::host_name() << std::endl;
        }
    }

    void UranusNode::join() {
        m_syncing = true;
#if 0
        msgkey msg_key;

        reset();

        // BA0
        m_phase = kPhaseBA0;

        ba0Loop(5);
        m_role = LISTENER;

        dlog("start BA0. role = ${role}",("role",(uint32_t)m_role));

        msg_key.blockNum = getBlockNum();
        msg_key.phase = m_phase;
        m_controllerPtr->processCache(msg_key);
#endif
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
        BlockRecord bax_block;

        m_phase = kPhaseBA1;
        m_baxCount = 0;

        m_controllerPtr->resetEcho();

        msg_key.blockNum = getBlockNum();
        msg_key.phase = m_phase;
        msg_key.phase += m_baxCount;
        m_controllerPtr->fastProcessCache(msg_key);

        dlog("fastBa1 begin. blockNum = ${id}", ("id", getBlockNum()));

        bax_block = m_controllerPtr->produceTentativeBlock();
        if (!EmptyBlock(bax_block.block)) {
            msg_key.blockNum = getBlockNum() + 1;
            msg_key.phase = kPhaseBA0;

            if (m_controllerPtr->findEchoCache(msg_key)) {
                *uranus_block = bax_block.block;

                dlog("##############fastBa1.finish blockNum = ${id}, hash = ${hash}",
                     ("id", getBlockNum())("hash", uranus_block->id()));
                m_controllerPtr->produceBlock(uranus_block);

                LOG_INFO << "checkpoint: blockNum:" << getBlockNum() << ";phase:" << m_phase << ";host_name:"
                         << boost::asio::ip::host_name() << ";block_hash_previous: " << uranus_block->previous
                         << ";txs_hash: " << uranus_block->id() << std::endl;

                fastBlock(msg_key.blockNum);
            } else {
                if ((getRoundInterval() == MAX_PHASE_SECONDS) && (isProcessNow())) {
                    //todo process two phase
                    ba1Process();
                } else {
                    ba1Loop(getRoundInterval());
                }
            }
        } else {
            msg_key.blockNum = getBlockNum();
            msg_key.phase = kPhaseBAX + 1;
            if (m_controllerPtr->findEchoCache(msg_key)) {
                fastBax();
            } else {
                if ((getRoundInterval() == MAX_PHASE_SECONDS) && (isProcessNow())) {
                    //todo process two phase
                    ba1Process();
                } else {
                    ba1Loop(getRoundInterval());
                }
            }
        }
    }

    void UranusNode::fastBax() {
        msgkey msg_key;
        signed_block_ptr uranus_block = std::make_shared<chain::signed_block>();
        BlockRecord bax_block;

        m_phase = kPhaseBAX;
        m_baxCount++;

        m_controllerPtr->resetEcho();

        msg_key.blockNum = getBlockNum();
        msg_key.phase = m_phase;
        msg_key.phase += m_baxCount;
        m_controllerPtr->fastProcessCache(msg_key);
        bax_block = m_controllerPtr->produceTentativeBlock();

        dlog("fastBax begin. blockNum = ${id}, phase = ${phase}", ("id", getBlockNum())("phase", msg_key.phase));

        if (!EmptyBlock(bax_block.block)) {
            msg_key.blockNum = getBlockNum() + 1;
            msg_key.phase = kPhaseBA0;

            if (m_controllerPtr->findEchoCache(msg_key)) {
                *uranus_block = bax_block.block;

                m_controllerPtr->produceBlock(uranus_block);
                dlog("##############finish blockNum = ${id}, hash = ${hash}",
                     ("id", getBlockNum())("hash", uranus_block->id()));
                LOG_INFO << "checkpoint: blockNum:" << getBlockNum() << ";phase:" << m_phase << ";host_name:"
                         << boost::asio::ip::host_name() << ";block_hash_previous: " << uranus_block->previous
                         << ";txs_hash: " << uranus_block->id() << std::endl;

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

        reset();

        m_controllerPtr->resetTimestamp();
        fastBa0();

        msg_key.blockNum = getBlockNum();
        msg_key.phase = kPhaseBA1;

        if (m_controllerPtr->findEchoCache(msg_key)) {
            fastBa1();
        } else {
            if ((getRoundInterval() == MAX_PHASE_SECONDS) && (isProcessNow())) {
                //todo process two phase
                ba0Process();
            } else {
                ba0Loop(getRoundInterval());
            }
        }
    }

    bool UranusNode::isProcessNow() {
        fc::time_point time_now = fc::time_point::now();

        dlog("isProcessNow: timenow = ${time} and block_timestamp = ${timestamp} time2 = ${interval} time2 = ${interval2}",
             ("time", time_now)("timestamp", m_controllerPtr->getFastTimestamp())("interval", fc::seconds(1))
                     ("interval2", time_now - m_controllerPtr->getFastTimestamp()));

        if (time_now > m_controllerPtr->getFastTimestamp()) {
            if ((time_now - m_controllerPtr->getFastTimestamp()) > fc::seconds(1)) {
                return true;
            }
        }

        return false;
    }

    uint32_t UranusNode::getRoundInterval() {
        boost::chrono::system_clock::time_point current_time = boost::chrono::system_clock::now();
        boost::chrono::seconds pass_time_to_genesis
                = boost::chrono::duration_cast<boost::chrono::seconds>(current_time - GENESIS);

        dlog("getRoundInterval. interval = ${id}.",
             ("id", MAX_PHASE_SECONDS - (pass_time_to_genesis.count() % MAX_PHASE_SECONDS)));

        return MAX_PHASE_SECONDS - (pass_time_to_genesis.count() % MAX_PHASE_SECONDS);
    }

    int UranusNode::getStakes(const std::string &pk) {
        // TODO(yufengshen): hack for now, when get stakes for self return 0 so that the role
        // will always be listener (non-producing), but for stakes for others, we still return
        // the expected value.
        if (m_isNonProducingNode && pk == std::string((char *) UranusNode::URANUS_PUBLIC_KEY))
            return 0;
        return VoterSystem::TOTAL_STAKES / GLOBAL_NODE_NUMBER;
    }

    ultrainio::chain::block_id_type UranusNode::getPreviousHash() {
        return m_controllerPtr->getPreviousBlockhash();
    }
}
