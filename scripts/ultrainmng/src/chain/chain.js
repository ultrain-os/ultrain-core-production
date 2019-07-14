const {U3} = require('u3.js');
const fs = require('fs');
var logger = require("../config/logConfig").getLogger("Chain");
var loggerChainChanging = require("../config/logConfig").getLogger("ChainChanging");
var chainConfig = require("./chainConfig")
var chainApi = require("./chainApi")
var constants = require("../common/constant/constants")
var chainNameConstants = require("../common/constant/constants").chainNameConstants
var contractConstants = require("../common/constant/constants").contractConstants
var tableConstants = require("../common/constant/constants").tableConstants
var scopeConstants = require("../common/constant/constants").scopeConstants
var actionConstants = require("../common/constant/constants").actionConstants
var chainIdConstants = require("../common/constant/constants").chainIdConstants
var pathConstants = require("../common/constant/constants").pathConstants
var iniConstants = require("../common/constant/constants").iniConstants
var sleep = require("sleep")
var utils = require("../common/util/utils")
var committeeUtil = require("./util/committeeUtil");
var blockUtil = require("./util/blockUtil");
var voteUtil = require("./util/voteUtil");
var NodUltrain = require("../nodultrain/nodultrain")
var WorldState = require("../worldstate/worldstate")
var chainUtil = require("./util/chainUtil");
var monitor = require("./monitor")
var mongoUtil = require("./util/mongoUtil");
var process = require('child_process');
var filenameConstants = require("../common/constant/constants").filenameConstants;



/**
 * 同步子链内数据标志（当该字段为false时，不向主链同步块，资源，用户等信息
 * 1. 当自己是主链是为false
 * 2. 当自己是子链，但不是委员会成员是为false
 * @type {boolean}
 */
var syncChainData = false;

//链切换flag-表示正在做链切换
var syncChainChanging = false;


//本地委员会成员列表
var localProducers = []


//存储用户同步的时候已成功的账户信息
var successAccountCacheList = new Set();

//存储同步时失败的
var failedAccountPramList = [];

//nod请求失败次数
var nodFailedTimes=0;
var maxNodFailedTimes=5;

//交易缓存
var trxCacheSet = new Set();

var seedCheckCount = 0;

//重启次数
var restartCount = 0;

/**
 * 进程正在跑
 * @type {boolean}
 */
var processRuning = false;


/**
 *
 * @returns {*}
 */
function getmaxNodFailedTimes() {

    if (utils.isNotNull(chainConfig.configFileData.local.maxNodFailedTimes)) {
        return chainConfig.configFileData.local.maxNodFailedTimes;
    }

    return maxNodFailedTimes;

}


//清除失败用户
function clearFailedUser(user) {
    if (failedAccountPramList.length == 0) {
        return;
    }

    var findflag = false;
    failedAccountPramList.forEach(function (item,index) {
        if (item.owner == user) {
            delete failedAccountPramList[index]
            findflag = true;
        }
    })

    if (findflag == false) {
        return;
    }


    var array = [];
    failedAccountPramList.forEach(function (item, index) {
        if (utils.isNotNull(item)) {
            array.push(item)
        }
    })

    failedAccountPramList = array;

}



/**
 * 同步新用户和权限到子链
 * @returns {Promise<void>}
 */
async function syncUser() {

    logger.info("sync user start");

    if (syncChainData == true && isMainChain() == false) {


        //投票计数，一轮不超过最大值
        let voteCount =0;

        //获取新增用户bulletin-并发送投票到子链
        let userBulletinList = await getUserBulletin(chainConfig.config, chainConfig.localChainName);
        logger.info("user userBulletinList:", userBulletinList);


        /**
         * 使用块头进行同步用户
         */
        if (monitor.needSyncUserResByBlock() == true) {

            let syncNumCount =0;
            logger.info("sync user by block is enabled by flag control");

            //增加本轮是否是出块节点的判断
            var headBlockProposer = await chainApi.getHeadBlockProposer(chainConfig.configSub);
            //判断是否要上传块头
            if (blockUtil.needPushBlockByProducerList(headBlockProposer, chainConfig.myAccountAsCommittee, localProducers) == false)  {
                logger.info("[Sync user]headBlockProposer("+headBlockProposer+") is not myself("+chainConfig.myAccountAsCommittee+"),do not sync user");
                return;
            }

            //投票结果
            var userCountRes = {
                totalNum: 0,
                successAccountNum: 0,
                syncBlockNum: 0,
                blockNotReadyNum:0,
            }

            let maxConfirmBlockNum = monitor.getConfirmBlockMaster();
            if (maxConfirmBlockNum <= 0) {
                maxConfirmBlockNum = await getMaxConfirmBlock(chainConfig.configSub, chainNameConstants.MAIN_CHAIN_NAME_TRANSFER);
            }
            logger.error("[sync user]maxConfirmBlockNum master in subchain is :",maxConfirmBlockNum);

            for (var i in userBulletinList) {
                userCountRes.totalNum++;
                var newUser = userBulletinList[i];

                if (syncNumCount >= monitor.getSyncBlockHeaderMaxTranNum()) {
                    logger.error("[sync user]syncNumCount("+syncNumCount+") >= maxnum("+monitor.getSyncBlockHeaderMaxTranNum()+"),need break");
                    break;
                }

                //检查缓存中是否有
                var hitFlag = successAccountCacheList.has(newUser.owner);
                //检查子链是否已经有改账户
                if (hitFlag == false && utils.isNull(await chainApi.getAccount(chainConfig.nodPort, newUser.owner))) {
                    logger.info("[sync user]account(" + newUser.owner + ") is not ready,need sync block..");
                    let blockHeight = newUser.block_height;
                    logger.info("[sync user]check account("+newUser.owner+") is in block("+blockHeight+"),check it is ready...");
                    //跨高不能小于已同步的块告
                    if (blockHeight > maxConfirmBlockNum) {
                        logger.error("([sync user]blockheight:" + blockHeight + "> maxConfirmBlockNum : "+maxConfirmBlockNum+" ,need not work");
                        userCountRes.blockNotReadyNum++;
                        continue;
                    } else {
                        logger.info("([sync user]blockheight:" + blockHeight + "<= maxConfirmBlockNum : "+maxConfirmBlockNum+" ,need work");
                    }

                    //获取主链的块信息
                    let blockInfo = await chainApi.getBlockInfoData(chainConfig.config.httpEndpoint,(blockHeight).toString());
                    logger.info("[sync user] block info:", blockInfo);
                    let trans = chainUtil.getSyncUserTransFromBlockHeader(blockInfo, chainConfig.localChainName,newUser.owner);
                    logger.info("[sync user]find trans length:", trans.length);
                    //调用MerkleProof
                    for (let t = 0; t < trans.length; t++) {
                        let tranId = trans[t].trx.id;
                        if (trxCacheSet.has(tranId) == true) {
                            userCountRes.successAccountNum++;
                            logger.info("([sync user]blockheight:" + blockHeight + ",trxid:" + tranId + " trx-m-root  is in cache,need not work");
                            continue;
                        }

                        syncNumCount++;
                        if (syncNumCount >= monitor.getSyncBlockHeaderMaxTranNum()) {
                            logger.error("syncNumCount("+syncNumCount+") >= maxnum("+monitor.getSyncBlockHeaderMaxTranNum()+"),need break");
                            break;
                        }

                        logger.info("([sync user]blockheight:" + blockHeight + ",trxid:" + tranId + " is not in cache,need query table check is ready");
                        logger.info("[sync user]getMerkleProof(blockheight:" + blockHeight + ",trxid:" + tranId);
                        let merkleProof = await chainApi.getMerkleProof(chainConfig.config, blockHeight, tranId);
                        logger.info("[sync user]merkleProof:", merkleProof);

                        if (utils.isNotNull(merkleProof)) {
                            logger.info("[sync user]merkleProof trx_receipt_bytes:", merkleProof.trx_receipt_bytes);
                            let tx_bytes_array = chainUtil.transferTrxReceiptBytesToArray(merkleProof.trx_receipt_bytes);
                            logger.info("[sync user]merkleProof trx_receipt_bytes convert to array length:", tx_bytes_array.length)
                            logger.debug("[sync user]merkleProof trx_receipt_bytes convert to array:", tx_bytes_array.toString());

                            let blockHeightInfo = await chainApi.getBlockHeaderInfo(chainConfig.getLocalHttpEndpoint(),chainNameConstants.MAIN_CHAIN_NAME_TRANSFER,blockHeight);
                            logger.info("[sync user]master blockHeightInfo("+blockHeight+"):",blockHeightInfo);
                            let hashIsReady = checkHashIsready(blockHeightInfo,tranId);
                            if (hashIsReady == true) {
                                logger.info("[sync user]master blockHeightInfo("+blockHeight+") trx id : "+tranId+", is ready, need not push");
                                trxCacheSet.add(tranId);
                                userCountRes.successAccountNum++;
                            } else {
                                logger.info("[sync user]master blockHeightInfo(" + blockHeight + ") trx id : " + tranId + ", is not ready, need push");

                                let param = {
                                    chain_name: chainNameConstants.MAIN_CHAIN_NAME_TRANSFER, block_number: blockHeight,
                                    merkle_proofs: merkleProof.merkle_proof, tx_bytes: tx_bytes_array.toString()
                                }
                                logger.info("[sync user]prepare to push sync  transfer trx:", param);

                                param = {
                                    chain_name: chainNameConstants.MAIN_CHAIN_NAME_TRANSFER, block_number: blockHeight,
                                    merkle_proofs: merkleProof.merkle_proof, tx_bytes: tx_bytes_array
                                }

                                let res = await chainApi.contractInteract(chainConfig.configSub, contractConstants.ULTRAINIO, "synclwctx", param, chainConfig.myAccountAsCommittee, chainConfig.config.keyProvider[0]);
                                logger.info("[sync user]synclwctx res:", res);
                                userCountRes.syncBlockNum++;
                            }
                        } else {
                            logger.error("[sync user] merkleProof is null");
                        }

                    }
                } else {

                    //账号已存在不处理
                    logger.info("[sync user]account(" + newUser.owner + ") is ready,need not sync block..");
                    clearFailedUser(newUser.owner);
                    userCountRes.successAccountNum++;
                    successAccountCacheList.add(newUser.owner);

                }

            }

            logger.info("[sync user] sync block user result", userCountRes);
        }  else {
            logger.error("sync user by block is disabled by flag control");
        }

    }

    //logger.info("successAccountCacheList:",successAccountCacheList);
    logger.info("sync user end");
}

/**
 * //判断自己是否是出块节点的判断
 * @returns {Promise<boolean>}
 */
async function isHeadBlockProposer() {

    var headBlockProposer = await chainApi.getHeadBlockProposer(chainConfig.configSub);
    if (headBlockProposer == chainConfig.mySkAsCommittee) {
        return true;
    }

    return false;
}

/**
 *
 * @returns {Promise<void>}
 */
async function syncUgas() {

    logger.info("start sync ugas");

    if (monitor.isDeploying()) {
        logger.error("monitor.isDeploying(),wait...", monitor.isDeploying());
        return;
    } else {
        logger.info("monitor.isDeploying()", monitor.isDeploying());
    }

    if (monitor.needSyncUgas() == false) {
        logger.error("[Sync Ugas]monitor set ugas is false,need not sync ugas");
        return;
    }

    if (syncChainData == true && isMainChain() == false) {

        //增加本轮是否是出块节点的判断
        var headBlockProposer = await chainApi.getHeadBlockProposer(chainConfig.configSub);

        //判断是否要上传块头
        if (blockUtil.needPushBlockByProducerList(headBlockProposer, chainConfig.myAccountAsCommittee, localProducers) == false)  {
            logger.info("[Sync Ugas]headBlockProposer("+headBlockProposer+") is not myself("+chainConfig.myAccountAsCommittee+"),do not sync ugas");
            return;
        }

        //主链到子链
        await syncMasterUgasToSubchain();

        //子链到主链
        await syncSubchainUgasToMaster();

    } else {
        logger.info("need not sync ugas");
    }

}

/**
 *
 * @param config
 * @param chainName
 * @returns {Promise<number>}
 */
async function getMaxConfirmBlock(config,chainName) {
    let maxConfirmBlock = 0;
    logger.info("getMaxConfirmBlock:",chainName);
    try {
        let tableData = await chainApi.getTableAllData(config,contractConstants.ULTRAINIO,contractConstants.ULTRAINIO,tableConstants.CHAINS,null);
        //logger.info("getMaxConfirmBlock rows:",tableData);
        if (tableData.rows.length >0) {
            for (let i=0;i<tableData.rows.length;i++) {
                let row = tableData.rows[i];
                if (row.chain_name == chainName) {
                    maxConfirmBlock = row.confirmed_block_number;
                    break;
                }
            }
        }

    } catch (e) {
        logger.error("getMaxConfirmBlock error:",e);
    }

    return maxConfirmBlock;
}


/**
 *
 * @param blockHeightInfo
 * @param trx_receipt_bytes
 * @returns {boolean}
 */
function checkHashIsready(blockHeightInfo,tranId) {
    let flag = false;
    try {
        if (blockHeightInfo.length > 0) {
            let trx_ids = blockHeightInfo[0].trx_ids;
            for (let i = 0; i < trx_ids.length; i++) {
                if (trx_ids[i] == tranId) {
                    return true;
                } else {
                    logger.debug("table tra_id("+trx_ids[i]+") is not equal tra_id("+tranId+"):");
                }
            }

        }
    } catch (e) {
        logger.error("checkHashIsready error,",e);
    }

    return flag;
}


/**
 * 同步主链ugas到子链
 * @returns {Promise<void>}
 */
async function syncMasterUgasToSubchain() {

    logger.info("[Sync Ugas-Master]syncMasterUgasToSubchain start");
    let bulletinBankData = await chainApi.getTableAllData(chainConfig.config,contractConstants.UTRIO_BANK,chainConfig.localChainName,tableConstants.BULLETIN_BANK,null);
    if (bulletinBankData.rows.length == 0) {
        logger.error("[Sync Ugas-Master]bulletinBankData list is null");
    }

    let maxConfirmBlockNum = monitor.getConfirmBlockMaster();
    if (maxConfirmBlockNum <= 0) {
        maxConfirmBlockNum = await getMaxConfirmBlock(chainConfig.configSub, chainNameConstants.MAIN_CHAIN_NAME_TRANSFER);
    }
    logger.error("[Sync Ugas-Master]maxConfirmBlockNum master in subchain is :",maxConfirmBlockNum);

    let minSavedSyncBlockNum = await chainApi.getMinBlockHeaderInfo(chainConfig.getLocalHttpEndpoint(),chainNameConstants.MAIN_CHAIN_NAME_TRANSFER);
    logger.info("[Sync Ugas-Master]minSavedSyncBlockNum is :",minSavedSyncBlockNum);

    let syncNumCount = 0;

    logger.info("[Sync Ugas-Master]bulletinBankData data rows length:",bulletinBankData.rows.length);
    //投票结果
    var syncUgas = {
        totalBlcokNum: 0,
        successAccountNum: 0,
        syncBlockNum: 0,
        blockNotReadyNum:0,
    }

    syncUgas.totalBlcokNum = bulletinBankData.rows.length;

    for (let i = 0; i < bulletinBankData.rows.length; i++) {
        try {
            let bankBlockData = bulletinBankData.rows[i];
            logger.debug("row " + i + ", bulletinBankData :", bankBlockData);
            try {
                let blockHeight = bankBlockData.block_height;
                let bulletinInfoLength = bankBlockData.bulletin_infos.length;
                logger.debug("row " + i + ", blockHeight :" + bankBlockData + ",bulletinInfoLength:" + bulletinInfoLength);

                //块高不能小于保留最小的块高
                if (blockHeight < minSavedSyncBlockNum) {
                    logger.error("[Sync Ugas-Master](blockheight:" + blockHeight + "< minSavedSyncBlockNum : "+minSavedSyncBlockNum+" ,need not work");
                    continue;
                }

                //跨高不能小于已同步的块高
                if (blockHeight > maxConfirmBlockNum) {
                    logger.error("[Sync Ugas-Master](blockheight:" + blockHeight + "> maxConfirmBlockNum : "+maxConfirmBlockNum+" ,need not work");
                    syncUgas.blockNotReadyNum++;
                    continue;
                } else {
                    logger.info("[Sync Ugas-Master](blockheight:" + blockHeight + "<= maxConfirmBlockNum : "+maxConfirmBlockNum+" ,need work");
                }

                //获取主链的块信息
                let blockInfo = await chainApi.getBlockInfoData(chainConfig.config.httpEndpoint,(blockHeight).toString());
                logger.debug("[Sync Ugas-Master]sync ugas info:", blockInfo);
                let trans = chainUtil.getTransFromBlockHeader(blockInfo, chainConfig.localChainName);
                logger.info("[Sync Ugas-Master]find trans length:", trans.length);
                //调用MerkleProof
                for (let t = 0; t < trans.length; t++) {
                    let tranId = trans[t].trx.id;

                    if (trxCacheSet.has(tranId) == true) {
                        logger.info("[Sync Ugas-Master](blockheight:" + blockHeight + ",trxid:" + tranId+" trx-m-root  is in cache,need not work");
                        syncUgas.successAccountNum++;
                        continue;
                    }

                    logger.info("[Sync Ugas-Master](blockheight:" + blockHeight + ",trxid:" + tranId+" is not in cache,need query table check is ready");

                    logger.info("[Sync Ugas-Master]getMerkleProof(blockheight:" + blockHeight + ",trxid:" + tranId);
                    let merkleProof = await chainApi.getMerkleProof(chainConfig.config, blockHeight, tranId);
                    logger.info("[Sync Ugas-Master]merkleProof:", merkleProof);
                    if (utils.isNotNull(merkleProof)) {
                        logger.debug("[Sync Ugas-Master]merkleProof trx_receipt_bytes:", merkleProof.trx_receipt_bytes);
                        let tx_bytes_array = chainUtil.transferTrxReceiptBytesToArray(merkleProof.trx_receipt_bytes);
                        logger.debug("[Sync Ugas-Master]merkleProof trx_receipt_bytes convert to array length:", tx_bytes_array.length)

                        logger.debug("[Sync Ugas-Master]merkleProof trx_receipt_bytes convert to array:", tx_bytes_array.toString());

                        let blockHeightInfo = await chainApi.getBlockHeaderInfo(chainConfig.getLocalHttpEndpoint(),chainNameConstants.MAIN_CHAIN_NAME_TRANSFER,blockHeight);
                        logger.debug("[Sync Ugas-Master]master blockHeightInfo("+blockHeight+"):",blockHeightInfo);
                        let hashIsReady = checkHashIsready(blockHeightInfo,tranId);
                        if (hashIsReady == true) {
                            logger.info("[Sync Ugas-Master]master blockHeightInfo("+blockHeight+") trx id : "+tranId+", is ready, need not push");
                            trxCacheSet.add(tranId);
                            syncUgas.successAccountNum++;
                        } else {
                            logger.info("[Sync Ugas-Master]master blockHeightInfo(" + blockHeight + ") trx id : " + tranId + ", is not ready, need push");

                            //控制最大次数
                            syncNumCount++;
                            if (syncNumCount >= monitor.getSyncBlockHeaderMaxTranNum()) {
                                logger.error("[Sync Ugas-Master]syncNumCount("+syncNumCount+") >= maxnum("+monitor.getSyncBlockHeaderMaxTranNum()+"),need break");
                                break;
                            }

                            let param = {
                                chain_name: chainNameConstants.MAIN_CHAIN_NAME_TRANSFER, block_number: blockHeight,
                                merkle_proofs: merkleProof.merkle_proof, tx_bytes: tx_bytes_array.toString()
                            }
                            logger.info("[Sync Ugas-Master]prepare to push sync  transfer trx:", param);

                            param = {
                                chain_name: chainNameConstants.MAIN_CHAIN_NAME_TRANSFER, block_number: blockHeight,
                                merkle_proofs: merkleProof.merkle_proof, tx_bytes: tx_bytes_array
                            }

                            let res = await chainApi.contractInteract(chainConfig.configSub, contractConstants.ULTRAINIO, "synclwctx", param, chainConfig.myAccountAsCommittee, chainConfig.config.keyProvider[0]);
                            logger.info("[Sync Ugas-Master]synclwctx res:", res);

                            syncUgas.syncBlockNum++;
                        }
                    }
                }

                if (syncNumCount >= monitor.getSyncBlockHeaderMaxTranNum()) {
                    logger.error("[Sync Ugas-Master]syncNumCount("+syncNumCount+") >= maxnum("+monitor.getSyncBlockHeaderMaxTranNum()+"),need break");
                    break;
                }
            } catch (e) {
                logger.error("[Sync Ugas-Master]bank block data row error:",e);
            }

        } catch (e) {
            logger.error("[Sync Ugas-Master]bank block data error:",e);
        }
    }

    logger.info("[Sync Ugas-Master]sync res:",syncUgas);

    logger.info("[Sync Ugas-Master]syncMasterUgasToSubchain end");
}

/**
 * 同步子链ugas到主链
 * @returns {Promise<void>}
 */
async function syncSubchainUgasToMaster() {

    logger.info("[Sync Ugas-Subchain]syncSubchainUgasToMaster start");
    let bulletinBankData = await chainApi.getTableAllData(chainConfig.configSub,contractConstants.UTRIO_BANK,chainNameConstants.MAIN_CHAIN_NAME,tableConstants.BULLETIN_BANK,null);
    if (bulletinBankData.rows.length == 0) {
        logger.error("[Sync Ugas-Subchain]bulletinBankData master list is null");
    }

    let maxConfirmBlockNum = monitor.getConfirmBlockLocal();
    if (maxConfirmBlockNum <= 0) {
        maxConfirmBlockNum = await getMaxConfirmBlock(chainConfig.config, chainConfig.localChainName);
    }
    logger.info("[Sync Ugas-Subchain]maxConfirmBlockNum subchain in master is :",maxConfirmBlockNum);

    let minSavedSyncBlockNum = await chainApi.getMinBlockHeaderInfo(chainConfig.config.httpEndpoint,chainConfig.localChainName);
    logger.info("[Sync Ugas-Subchain]minSavedSyncBlockNum is :",minSavedSyncBlockNum);

    logger.info("[Sync Ugas-Subchain]bulletinBankData master data rows length:",bulletinBankData.rows.length);
    let syncNumCount = 0;
    var syncUgas = {
        totalBlcokNum: 0,
        successAccountNum: 0,
        syncBlockNum: 0,
        blockNotReadyNum:0,
    }

    syncUgas.totalBlcokNum = bulletinBankData.rows.length;
    for (let i = 0; i < bulletinBankData.rows.length; i++) {
        try {
            let bankBlockData = bulletinBankData.rows[i];
            logger.debug("[Sync Ugas-Subchain]row " + i + ", master bulletinBankData :", bankBlockData);
            try {
                let blockHeight = bankBlockData.block_height;
                let bulletinInfoLength = bankBlockData.bulletin_infos.length;
                logger.info("[Sync Ugas-Subchain]master row " + i + ", blockHeight :" + bankBlockData + ",bulletinInfoLength:" + bulletinInfoLength);


                //块高不能小于保留最小的块高
                if (blockHeight < minSavedSyncBlockNum) {
                    logger.error("[Sync Ugas-Master](blockheight:" + blockHeight + "< minSavedSyncBlockNum : "+minSavedSyncBlockNum+" ,need not work");
                    continue;
                }

                //跨高不能小于已同步的块告
                if (blockHeight > maxConfirmBlockNum) {
                    logger.error("[Sync Ugas-Subchain](subchain("+chainConfig.localChainName+") blockheight:" + blockHeight + "> maxConfirmBlockNum : "+maxConfirmBlockNum+" ,need not work");
                    syncUgas.blockNotReadyNum++;
                    continue;
                } else {
                    logger.info("[Sync Ugas-Subchain](subchain("+chainConfig.localChainName+") blockheight:" + blockHeight + "<= maxConfirmBlockNum : "+maxConfirmBlockNum+" ,need work");
                }

                //获取子链的块信息
                let blockInfo = await chainApi.getBlockInfoData(chainConfig.getLocalHttpEndpoint(),(blockHeight).toString());
                logger.debug("[Sync Ugas-Subchain]sync ugas master info:", blockInfo);
                let trans = chainUtil.getTransFromBlockHeader(blockInfo, chainNameConstants.MAIN_CHAIN_NAME);
                logger.info("[Sync Ugas-Subchain]find trans length:", trans.length);
                //调用MerkleProof
                for (let t = 0; t < trans.length; t++) {
                    let tranId = trans[t].trx.id;

                    if (trxCacheSet.has(tranId) == true) {
                        logger.info("[Sync Ugas-Subchain](blockheight:" + blockHeight + ",trxid:" + tranId+" is in cache,need not work");
                        syncUgas.successAccountNum++;
                        continue;
                    }

                    logger.info("[Sync Ugas-Subchain]master getMerkleProof(blockheight:" + blockHeight + ",trxid:" + tranId);
                    let merkleProof = await chainApi.getMerkleProof(chainConfig.configSub, blockHeight, tranId);
                    logger.info("[Sync Ugas-Subchain]master merkleProof:", merkleProof);
                    if (utils.isNotNull(merkleProof)) {
                        logger.info("[Sync Ugas-Subchain]merkleProof trx_receipt_bytes:", merkleProof.trx_receipt_bytes);
                        let tx_bytes_array = chainUtil.transferTrxReceiptBytesToArray(merkleProof.trx_receipt_bytes);
                        logger.debug("[Sync Ugas-Subchain]merkleProof mastertrx_receipt_bytes convert to array length:", tx_bytes_array.length)
                        logger.debug("[Sync Ugas-Subchain]merkleProof master trx_receipt_bytes convert to array:", tx_bytes_array.toString());

                        let blockHeightInfo = await chainApi.getBlockInfoData(chainConfig.config.httpEndpoint,chainConfig.localChainName,blockHeight);
                        logger.debug("[Sync Ugas-Subchain]subchain blockHeightInfo("+blockHeight+"):",blockHeightInfo);
                        let hashIsReady = checkHashIsready(blockHeightInfo,tranId);
                        if (hashIsReady == true) {
                            logger.info("[Sync Ugas-Subchain]subchain blockHeightInfo("+blockHeight+") trx id : "+tranId+", is ready, need not push");
                            trxCacheSet.add(tranId);
                            syncUgas.successAccountNum++;
                        } else {

                            //控制最大次数
                            syncNumCount++;
                            if (syncNumCount >= monitor.getSyncBlockHeaderMaxTranNum()) {
                                logger.error("[Sync Ugas-Subchain]syncNumCount("+syncNumCount+") >= maxnum("+monitor.getSyncBlockHeaderMaxTranNum()+"),need break");
                                break;
                            }

                            syncUgas.syncBlockNum++;
                            logger.info("[Sync Ugas-Subchain]subchain blockHeightInfo(" + blockHeight + ") trx id : " + tranId + ", is not ready, need push");

                            let param = {
                                chain_name: chainConfig.localChainName, block_number: blockHeight,
                                merkle_proofs: merkleProof.merkle_proof, tx_bytes: tx_bytes_array.toString()
                            }
                            logger.info("[Sync Ugas-Subchain]prepare to push sync master transfer trx:", param);

                            param = {
                                chain_name: chainConfig.localChainName, block_number: blockHeight,
                                merkle_proofs: merkleProof.merkle_proof, tx_bytes: tx_bytes_array
                            }
                            let res = await chainApi.contractInteract(chainConfig.config, contractConstants.ULTRAINIO, "synclwctx", param, chainConfig.myAccountAsCommittee, chainConfig.config.keyProvider[0]);
                            logger.info("[Sync Ugas-Subchain]synclwctx res:", res);
                        }
                    }
                }

                if (syncNumCount >= monitor.getSyncBlockHeaderMaxTranNum()) {
                    logger.error("[Sync Ugas-Subchain]syncNumCount("+syncNumCount+") >= maxnum("+monitor.getSyncBlockHeaderMaxTranNum()+"),need break");
                    break;
                }

            } catch (e) {
                logger.error("[Sync Ugas-Subchain]bank block data master row error:",e);
            }

        } catch (e) {
            logger.error("[Sync Ugas-Subchain]bank block data master error:",e);
        }
    }

    logger.info("[Sync Ugas-Subchain] res data:",syncUgas);

    logger.info("[Sync Ugas-Subchain]syncSubchainUgasToMaster end");
}

/**
 * getBlockSubmitRatio
 * @returns {*}
 */
function getBlockSubmitRatio() {

    try {
        var num = monitor.getMaxBlockSubmitorNum();
        var committeeCount = localProducers.length;
        if (num > 0 && committeeCount > 0) {
            let ratio = num / committeeCount;
            if (ratio < 1) {
                return ratio;
            }

            return chainConfig.configFileData.local.syncBlockRatio;
        }
    } catch (e) {
        logger.error("getBlockSubmitRatio error:",e)
    }

    return 0;
}

/**
 *
 * @param confirmed_block
 * @returns {*}
 */
function getHasConfirmBlock(confirmed_block) {
    try {
         let number = confirmed_block.number;
         if (utils.isNotNull(number)) {
             return number;
         }
    } catch (e) {
        logger.error("confirmed_block process error:",e)
    }

    return -1;
}

/**
 * 同步块头
 * @returns {Promise<void>}
 */
async function syncBlock() {

    if (monitor.isDeploying()) {
        logger.error("monitor.isDeploying(),wait...", monitor.isDeploying());
        return;
    } else {
        logger.info("monitor.isDeploying()", monitor.isDeploying());
    }

    logger.info("sync block start");

    //一次最大块数
    var blockSyncMaxNum = chainConfig.getLocalConfigInfo("blockSyncMaxNum", 10);

    if (syncChainData == true && isMainChain() == false) {

        var subBlockNumMax = await chainApi.getHeadBlockNum(chainConfig.nodPort);
        if (subBlockNumMax == null) {
            logger.error("subBlockNumMax is null,abort sync block");
            return;
        }
        logger.info("subBlockNumMax:"+subBlockNumMax);
        var traceMode = true;
        //获取本地最新的块头，获取服务端最新的块头
        let result = await await chainApi.getBlockInfoData(chainConfig.getLocalHttpEndpoint(),(subBlockNumMax).toString());
        logger.info("result.proposer,",result.proposer);

        //判断是否要上传块头
        if (blockUtil.needPushBlockByProducerList(result.proposer, chainConfig.myAccountAsCommittee, localProducers) == false) {
            logger.info("finish sync block..");
            return;
        }

        logger.info("start to push block..");
        let blockNum = 0;
        let subchainBlockNumResult = await chainApi.getSubchainBlockNum(chainConfig.config, chainConfig.localChainName);
        logger.info("mainchain block num:", subchainBlockNumResult);

        //设置本链已同步最高的块告
        let confirmed_block = subchainBlockNumResult.confirmed_block;
        logger.error("[sync block]confirmed_block:",getHasConfirmBlock(confirmed_block));
        monitor.setConfirmBlockLocal(getHasConfirmBlock(confirmed_block));

        let forks = subchainBlockNumResult.forks;
        let findFlag = false;
        if (utils.isNullList(forks) == false) {
            for (let i = 0; i < forks.length; i++) {
                let fork = forks[i];
                let block_id = fork.block_id;
                logger.info("fork:", fork);
                let localBlockId = 0;
                try {
                    let result = await chainApi.getBlockInfoData(chainConfig.getLocalHttpEndpoint(),(fork.number).toString());
                    logger.debug("block info", result);
                    localBlockId = result.id;
                } catch (e) {
                    logger.error("get block(" + fork.number + ") error,", e);
                }

                if (block_id == localBlockId) {
                    findFlag = true;
                    logger.info("block id(" + block_id + ") == local block id(" + localBlockId + "),match");
                    blockNum = fork.number;
                    break;
                } else {
                    logger.info("block id(" + block_id + ") != local block id(" + localBlockId + "),mot match");
                }

            }
        } else {
            findFlag = true;
        }

        //如果找不到块，不上传块头
        if (findFlag == false) {
            logger.error("can't find matched block info , nedd not sync block");
            return;
        }

        logger.info("subchain head block num=", subBlockNumMax);
        logger.info("mainchain(subchain:" + chainConfig.localChainName + ") max blockNum =" + blockNum);


        //初始化block Num
        let blockNumInt = parseInt(blockNum, 10) + 1;
        var traceBlcokCount = subBlockNumMax - blockNumInt;
        logger.debug("trace block num count =", traceBlcokCount);

        if (subBlockNumMax - blockNumInt >= blockSyncMaxNum) {
            subBlockNumMax = blockNumInt + blockSyncMaxNum;
        }

        logger.info("need upload block range [" + blockNumInt + " -> " + subBlockNumMax - 1 + "]");
        let results = [];
        let blockListStr = "(";
        for (var i = blockNumInt; i < subBlockNumMax; i++) {
            let result = await chainApi.getBlockInfoData(chainConfig.getLocalHttpEndpoint(),(i).toString());
            logger.info("block " + i + ": (proposer:", result.proposer + ")");
            logger.debug("block:",result);
            logger.debug("header_extensions:", result.header_extensions);
            var extensions = [];
            if (result.header_extensions.length > 0) {
                result.header_extensions.forEach(function (item, index) {
                    extensions.push({"type": item[0], "data": item[1]})
                })
            }

            //logger.info("extensions:",extensions);
            /**
             * 需要上传
             */

            logger.debug("add push array(block num ：" + i + ")");
            results.push({
                "timestamp": result.timevalue,
                "proposer": result.proposer,
                // "proposerProof": proposerProof,
                "version": result.version,
                "previous": result.previous,
                "transaction_mroot": result.transaction_mroot,
                "action_mroot": result.action_mroot,
                "committee_mroot": result.committee_mroot,
                "header_extensions": extensions,
                "signature": result.signature,
                //blockNum : i
            });

            blockListStr += i + ",";


        }
        blockListStr += ")";
        logger.info("local uncommit blocklist :", blockListStr);

        /**
         * 块头上传
         */
        if (results) {
            const params = {
                chain_name: chainConfig.localChainName,
                headers: results
            };

            logger.info("block params:", params);
            logger.info("pushing block to head (chain_name :" + chainConfig.localChainName + " count :" + results.length + ")");
            if (results.length > 0) {
                await chainApi.contractInteract(chainConfig.config, contractConstants.ULTRAINIO, "acceptheader", params, chainConfig.myAccountAsCommittee, chainConfig.config.keyProvider[0]);
            }
        }

        //同步主链块头
        await syncMasterBlock();

        //上传同步块高的数据
        if (monitor.checkNeedSync() ) {
            await chainApi.confirmBlockCheckIn(monitor.getMonitorUrl(), {
                "baseChain": chainConfig.localChainName,
                "targetChain": constants.chainNameConstants.MAIN_CHAIN_NAME,
                "confirmBlock": monitor.getConfirmBlockLocal()
            });
            await chainApi.confirmBlockCheckIn(monitor.getMonitorUrl(), {
                "baseChain": constants.chainNameConstants.MAIN_CHAIN_NAME,
                "targetChain": chainConfig.localChainName,
                "confirmBlock": monitor.getConfirmBlockMaster()
            });
        } else {
            logger.error("monitor is false,need not upload confirm block info");
        }
        return;


    } else {
        logger.error("sync block is not needed")
    }

    logger.info("sync block finish")

}


/**
 * 同步主链块头
 * @returns {Promise<void>}
 */
async function syncMasterBlock() {

    if (monitor.isDeploying()) {
        logger.error("monitor.isDeploying(),wait...", monitor.isDeploying());
        return;
    } else {
        logger.info("monitor.isDeploying()", monitor.isDeploying());
    }

    if (isMainChain() == true) {
        logger.error("I am in masterchain ,need not sync master chain block");
        return;
    }

    logger.info("sync master block start");

    //一次最大块数
    var blockSyncMaxNum = chainConfig.getLocalConfigInfo("blockSyncMaxNum", 10);

    if (syncChainData == true) {

        var masterBlockNumMax = await chainApi.getMasterHeadBlockNum(chainConfig.config.httpEndpoint);
        if (masterBlockNumMax == null) {
            logger.error("masterBlockNumMax is null,abort sync block");
            return;
        }

        //获取主链现在最大的块头
        logger.info("masterBlockNumMax:",masterBlockNumMax);
        logger.info("start to push master block..");
        let blockNum = 0;
        let subchainBlockNumResult = await chainApi.getMasterBlockNum(chainConfig.nodPort);
        logger.info("subchain max block num:", subchainBlockNumResult);

        //设置已同步主链的块高信息
        let confirmed_block = subchainBlockNumResult.confirmed_block;
        logger.error("[sync master block]confirmed_block:",getHasConfirmBlock(confirmed_block));
        monitor.setConfirmBlockMaster(getHasConfirmBlock(confirmed_block));


        let forks = subchainBlockNumResult.forks;
        let findFlag = false;
        if (utils.isNullList(forks) == false) {
            for (let i = 0; i < forks.length; i++) {
                let fork = forks[i];
                let block_id = fork.block_id;
                logger.info("master fork:", fork);
                let localBlockId = 0;
                try {
                    let result = await chainApi.getBlockInfoData(chainConfig.config.httpEndpoint,fork.number.toString());
                    logger.debug("block info", result);
                    localBlockId = result.id;
                } catch (e) {
                    logger.error("get master block(" + fork.number + ") error,", e);
                }

                if (block_id == localBlockId) {
                    findFlag = true;
                    logger.info("master block id(" + block_id + ") == local master block id(" + localBlockId + "),match");
                    blockNum = fork.number;
                    break;
                } else {
                    logger.error("master block id(" + block_id + ") != local master block id(" + localBlockId + "),mot match");
                }

            }
        } else {
            findFlag = true;
        }

        //如果找不到块，不上传块头
        if (findFlag == false) {
            logger.error("can't find matched master block info , nedd not sync master block");
            return;
        }

        logger.info("master chain  head block num=", masterBlockNumMax);
        logger.info("subchain info(subchain:" + chainConfig.localChainName + ") max master synced blockNum =" + blockNum);


        //初始化block Num
        let blockNumInt = parseInt(blockNum, 10) + 1;


        if (masterBlockNumMax - blockNumInt >= blockSyncMaxNum) {
            masterBlockNumMax = blockNumInt + blockSyncMaxNum;
        }

        logger.info("need upload block range [" + blockNumInt + " -> " + masterBlockNumMax - 1 + "]");
        let results = [];
        let blockListStr = "(";
        for (var i = blockNumInt; i < masterBlockNumMax; i++) {
            let result = await chainApi.getBlockInfoData(chainConfig.config.httpEndpoint,(i).toString());
            logger.debug("master block " + i + ": (proposer:", result.proposer + ")");
            logger.debug("master block header_extensions:", result.header_extensions);
            var extensions = [];
            if (result.header_extensions.length > 0) {
                result.header_extensions.forEach(function (item, index) {
                    extensions.push({"type": item[0], "data": item[1]})
                })
            }

            //logger.info("extensions:",extensions);
            /**
             * 需要上传
             */

            logger.debug("add push array(block num ：" + i + ")");
            results.push({
                "timestamp": result.timevalue,
                "proposer": result.proposer,
                // "proposerProof": proposerProof,
                "version": result.version,
                "previous": result.previous,
                "transaction_mroot": result.transaction_mroot,
                "action_mroot": result.action_mroot,
                "committee_mroot": result.committee_mroot,
                "header_extensions": extensions,
                "signature": result.signature,
                //blockNum : i
            });
            blockListStr += i + ",";


        }
        blockListStr += ")";
        logger.info("local uncommit master blocklist :", blockListStr);

        /**
         * 主链块头上传
         */
        if (results) {
            const params = {
                headers: results
            };

            if (results.length > 0) {
                logger.debug("master block params:", params);
                logger.info("pushing master block to subchain ( count :" + results.length + ")");
                if (results.length > 0) {
                    let resAcceptMaster = await chainApi.contractInteract(chainConfig.configSub, contractConstants.ULTRAINIO, "acceptmaster", params, chainConfig.myAccountAsCommittee, chainConfig.config.keyProvider[0]);
                    //logger.info("resAcceptMaster res:",resAcceptMaster);
                }
            }
        }


    } else {
        logger.error("sync master block is not needed")
    }

    logger.info("sync master block finish")

}

/**
 * 同步委员会
 * @returns {Promise<void>}
 */
async function syncCommitee() {

    try {
        logger.info("syncCommitee start");

        if (syncChainData == true && isMainChain() == false) {

            var subBlockNumMax = await chainApi.getHeadBlockNum(chainConfig.nodPort);
            if (subBlockNumMax == null) {
                logger.error("[committee trx] subBlockNumMax is null,abort sync block");
                return;
            }
            logger.info("[committee trx] subBlockNumMax:" + subBlockNumMax);
            //获取本地最新的块头，获取服务端最新的块头
            let result = await chainApi.getBlockInfoData(chainConfig.getLocalHttpEndpoint(),(subBlockNumMax).toString());

            //判断是否要上传块头
            if (blockUtil.needPushBlockByProducerList(result.proposer, chainConfig.myAccountAsCommittee, localProducers) == false) {
                logger.info("[committee trx] finish sync committee,is not me");
                return;
            }

            let bulletin = await chainApi.getCommitteeBulletin(chainConfig.config, chainConfig.localChainName);

            if (bulletin.length == 0) {
                logger.error("no data in committee bulletin,need not do anythin");
            } else {
                logger.error("find data in committee bulletin,need to process:", bulletin);
                let syncNumCount = 0;
                //投票结果
                var committeeCountRes = {
                    totalNum: 0,
                    successAccountNum: 0,
                    syncNum: 0,
                    blockNotReadyNum: 0,
                }

                let maxConfirmBlockNum = monitor.getConfirmBlockMaster();

                committeeCountRes.totalNum = bulletin.length;

                for (let i = 0; i < bulletin.length; i++) {
                    let bulletinObj = bulletin[i];
                    logger.info("[committee trx] check block(" + bulletinObj.block_num + ") confirmBlock(" + maxConfirmBlockNum + ")");

                    /**
                     * 判断confirm block是否已经同步了
                     */
                    if (bulletinObj.block_num > maxConfirmBlockNum) {
                        committeeCountRes.blockNotReadyNum++;
                        continue;
                    }

                    try {
                        //获取块信息并找到交易id
                        let blockInfo = await chainApi.getBlockInfoData(chainConfig.config.httpEndpoint,(bulletinObj.block_num).toString());
                        let trans = chainUtil.getMoveProdRTransFromBlockHeader(blockInfo, chainConfig.localChainName);
                        logger.error("[committee trx] trans(block:" + bulletinObj.block_num + "):", trans.length);
                        if (trans.length > 0) {
                            for (let t = 0; t < trans.length; t++) {
                                let tranId = trans[t].trx.id;
                                logger.info("[committee trx]block(" + bulletinObj.block_num + ") trxid(" + tranId + ")");

                                if (trxCacheSet.has(tranId) == true) {
                                    logger.info("[committee trx](blockheight:" + bulletinObj.block_num + ",trxid:" + tranId + " trx-m-root  is in cache,need not work");
                                    committeeCountRes.successAccountNum++;
                                    continue;
                                }

                                logger.info("[committee trx](blockheight:" + bulletinObj.block_num + ",trxid:" + tranId + " is not in cache,need query table check is ready");

                                logger.info("[committee trx]getMerkleProof(blockheight:" + bulletinObj.block_num + ",trxid:" + tranId);
                                let merkleProof = await chainApi.getMerkleProof(chainConfig.config, bulletinObj.block_num, tranId);
                                logger.info("[committee trx]merkleProof:", merkleProof);
                                if (utils.isNotNull(merkleProof)) {
                                    logger.debug("[committee trx]merkleProof trx_receipt_bytes:", merkleProof.trx_receipt_bytes);
                                    let tx_bytes_array = chainUtil.transferTrxReceiptBytesToArray(merkleProof.trx_receipt_bytes);

                                    let blockHeightInfo = await chainApi.getBlockHeaderInfo(chainConfig.getLocalHttpEndpoint(), chainNameConstants.MAIN_CHAIN_NAME_TRANSFER, bulletinObj.block_num);
                                    logger.debug("[committee trx]master blockHeightInfo(" + bulletinObj.block_num + "):", blockHeightInfo);
                                    let hashIsReady = checkHashIsready(blockHeightInfo, tranId);
                                    if (hashIsReady == true) {
                                        logger.info("[committee trx]master blockHeightInfo(" + bulletinObj.block_num + ") trx id : " + tranId + ", is ready, need not push");
                                        trxCacheSet.add(tranId);
                                        committeeCountRes.successAccountNum++;
                                        continue;
                                    }

                                    /**
                                     * 未在缓存中，重新投递消息
                                     */
                                    logger.info("[committee trx]master blockHeightInfo(" + bulletinObj.block_num + ") trx id : " + tranId + ", is not ready, need push");

                                    //控制最大次数
                                    if (syncNumCount >= 1) {
                                        logger.error("[committee trx]sync Committee count (" + syncNumCount + ") >= maxnum(" + monitor.getSyncBlockHeaderMaxTranNum() + "),need break");
                                        break;
                                    }

                                    let param = {
                                        chain_name: chainNameConstants.MAIN_CHAIN_NAME_TRANSFER,
                                        block_number: bulletinObj.block_num,
                                        merkle_proofs: merkleProof.merkle_proof,
                                        tx_bytes: tx_bytes_array.toString()
                                    }
                                    logger.info("[Sync Ugas-Master]prepare to push sync  transfer trx:", param);

                                    param = {
                                        chain_name: chainNameConstants.MAIN_CHAIN_NAME_TRANSFER,
                                        block_number: bulletinObj.block_num,
                                        merkle_proofs: merkleProof.merkle_proof,
                                        tx_bytes: tx_bytes_array
                                    }

                                    syncNumCount++;
                                    committeeCountRes.syncNum++;

                                    let res = await chainApi.contractInteract(chainConfig.configSub, contractConstants.ULTRAINIO, "synclwctx", param, chainConfig.myAccountAsCommittee, chainConfig.config.keyProvider[0]);
                                    logger.info("[committee trx]synclwctx res:", res);
                                }

                                //控制最大次数
                                if (syncNumCount >= 1) {
                                    logger.error("[committee trx]sync Committee count (" + syncNumCount + ") >= maxnum(1),need break");
                                    break;
                                }
                            }

                        }

                    } catch (e) {
                        logger.error("[committee trx] sync committee tran error:", e);
                    }

                }


                logger.error("[committee trx]sync committee res:", committeeCountRes);

            }
        }
    }
    catch
        (e)
        {
            logger.error("[committee trx] error:", e);
        }

        logger.info("sync committee end");
    }

/**
 * 同步链信息
 * @returns {Promise<void>}
 */
async function syncChainInfo() {
    try {

        logger.info("sync chain info and committee start..");

        //如果已在切换链过程中，不需要再同步数据
        if (syncChainChanging == true) {
            logger.info("chain changing , need not sync chain info");
            return;
        }

        logger.info("[seed check] start to check seed is alive");

        //同步seed的信息
        seedCheckCount++;
        logger.info("sync chain seed count :",seedCheckCount);
        if (seedCheckCount >= 120) {
            //检查主链seed是否有变化
            let mainSeedList = await chainApi.getChainHttpList(chainNameConstants.MAIN_CHAIN_NAME, chainConfig);
            if (mainSeedList.length > 0 && mainSeedList.length != chainConfig.config.seedHttpList.length) {
                logger.error("mainSeedList.length(" + mainSeedList.length + ") != config.seedHttpList(" + chainConfig.config.seedHttpList.length + "),need update main seedList:", mainSeedList);
                chainConfig.config.seedHttpList = mainSeedList;
            } else {
                logger.info("mainSeedList.length(" + mainSeedList.length + ") == config.seedHttpList(" + chainConfig.config.seedHttpList.length + "),need not update main seedList:", chainConfig.config.seedHttpList);
            }

            //检查子链seed是否有变化
            let subSeedList = await chainApi.getChainHttpList(chainConfig.localChainName, chainConfig);
            if (subSeedList.length > 0 && subSeedList.length != chainConfig.configSub.seedHttpList.length) {
                logger.error("subSeedList.length(" + subSeedList.length + ") != configSub.seedHttpList(" + chainConfig.configSub.seedHttpList.length + "),need update subchain seedList:", subSeedList);
                chainConfig.configSub.seedHttpList = subSeedList;
            } else {
                logger.info("subSeedList.length(" + subSeedList.length + ") == configSub.seedHttpList(" + chainConfig.configSub.seedHttpList.length + "),need not update sub seedList:", chainConfig.configSub.seedHttpList);
            }
            seedCheckCount = 0;
        }

        //定期更新configsub
        await chainApi.checkSubchainSeed(chainConfig);

        //定期更新config
        await chainApi.checkMainchainSeed(chainConfig);

        //同步链名称（子链id,链名称等）
        let chainName = null;
        let chainId = null;
        let genesisTime = null;
        let genesisPk = null;
        if (utils.isNull(chainConfig.configSub.chainId)) {
            chainConfig.configSub.chainId = await chainApi.getChainId(chainConfig.configSub);
        }
        logger.debug("configSub.chainId=", chainConfig.configSub.chainId);

        if (utils.isNull(chainConfig.config.chainId)) {
            chainConfig.config.chainId = await chainApi.getChainId(chainConfig.config);
        }
        logger.debug("config.chainId=", chainConfig.config.chainId);


        let chainInfo = await chainApi.getChainInfo(chainConfig.config, chainConfig.myAccountAsCommittee);
        logger.info("chain info from mainchain:", chainInfo);
        if (utils.isNotNull(chainInfo)) {
            chainName = chainInfo.location;
            chainId = chainInfo.chain_id;
            genesisTime = chainUtil.formatGensisTime(chainInfo.genesis_time);
            genesisPk = chainInfo.genesis_pk;
        }

        //设置用户属于的chainid和chainname信息
        if (utils.isNotNull(chainName)) {
            chainConfig.chainName = chainName;
        }
        if (utils.isNotNull(chainId) && chainIdConstants.NONE_CHAIN != chainId) {
            chainConfig.chainId = chainId;
        }
        if (utils.isNotNull(genesisTime)) {
            chainConfig.genesisTime = genesisTime;
            chainConfig.genesisPK = genesisPk;
        }

        logger.error("genesis-pk is ",chainConfig.genesisPK);

        //如果是主链，啥都不操作
        if (isMainChain()) {
            logger.error(chainConfig.myAccountAsCommittee + " runing in main chain, need not work");
            //check alive
            await checkNodAlive();
        }

        //如果是非出块节点，啥都不操作
        if (chainConfig.isNoneProducer()) {
            syncChainData = false;
            logger.error(chainConfig.myAccountAsCommittee + " runing is none-producer, need not work");
            //check alive
            await checkNodAlive();
            return;
        }

        logger.info(chainConfig.myAccountAsCommittee + " belongs to chaininfo (name:" + chainConfig.chainName + ",chain_id:" + chainConfig.chainId + " ,genesisTime:" + chainConfig.genesisTime + ") from mainchain");
        logger.info("now subchain's chainid :" + chainConfig.configSub.chainId);

        //主链返回的chainname非法，说明主链返回的有问题，或者是该用户在主链不存在,不工作
        if (chainConfig.chainName == chainNameConstants.INVAILD_CHAIN_NAME) {
            syncChainData = false;
            logger.error(chainConfig.myAccountAsCommittee + " is a invalid name in main chain,need not work");
            await checkNodAlive();
            return;
        }

        //同步本地委员会
        logger.info("sync local commitee");
        //获取本地prodcuers信息
        let producerList = await chainApi.getProducerLists(chainConfig.configSub.httpEndpoint);
        logger.info("subchain producers: ", producerList);
        if (utils.isNotNull(producerList) && producerList.length > 0) {
            localProducers = producerList;
        } else {
            logger.error("get subchain producers is null,sync committee end",producerList);
        }

        var isStrillInCommittee = committeeUtil.isStayInCommittee(localProducers, chainConfig.myAccountAsCommittee);
        //检查自己是否不在委员会里面
        if (!isStrillInCommittee) {
            syncChainData = false;
            logger.info("I(" + chainConfig.myAccountAsCommittee + ") am not in subchain committee")
        } else {
            syncChainData = true;
            logger.info("I(" + chainConfig.myAccountAsCommittee + ") am still in subchain committee")
        }

        //非主链需要检查是否要调度
        if (isMainChain() == false) {
            var rightChain = chainConfig.isInRightChain()
            if (!rightChain) {
                //我已不属于这条链，准备迁走
                if (isStrillInCommittee) {
                    logger.error("I(" + chainConfig.myAccountAsCommittee + ") am still in subchain committee,can't be transfer,wait...")
                } else {
                    syncChainData = false;
                    logger.info(chainConfig.myAccountAsCommittee + " are not in subchain committee , need trandfer to chain(" + chainName + "）, start transfer...");
                    if (monitor.isDeploying() == true) {
                        logger.error("monitor isDeploying, wait to switchChain");
                        sleep.msleep(1000);
                    } else {
                        sleep.msleep(1000);
                        syncChainChanging = true;
                        monitor.disableDeploy();

                        //清除数据
                        clearCacheData()
                        //开始迁移
                        await switchChain();
                        return;
                    }
                }
            } else {
                syncChainChanging = false;
                logger.info("i am in right chain");
            }
        }

        //check nod alive
        await checkNodAlive();

    } catch (e) {
        logger.error("sync chain error:", e);
    }

    logger.info("sync chain info and committee end");

}

/**
 *
 * @returns {Promise<void>}
 */
async function checkNodProcess() {

    let command = "ps axu | grep nodultrain";
    process.exec(command, function (error, stdout, stderr, finish) {
        try {
            let runingFlag = false;
            let nodFilePath = utils.formatHomePath(chainConfig.configFileData.local.nodpath) + "/" + filenameConstants.NOD_EXE_FILE;
            logger.info("nod path:", nodFilePath);
            if (error !== null) {
                logger.error("exec ps nod error: " + error);
            } else {
                logger.info("exccmd success:" + command);
                logger.info("command res :", stdout);
                let resData = stdout.split("\n");
                //logger.info("command resData :",resData.length);
                for (let i = 0; i < resData.length; i++) {
                    //logger.info("command line :",resData[i]);
                    if (resData[i].indexOf(nodFilePath) != -1) {
                        logger.info("nodultrain process is :", resData[i]);
                        runingFlag = true;
                        break;
                    }
                }

                processRuning = runingFlag;
            }
        } catch (e) {
            logger.error("checkNodProcess error:",e);
        }

    });
}

/**
 * 检查nod是否还存活
 * @returns {Promise<void>}
 */
async function checkNodAlive() {

    logger.info("start to checkNodAlive....");

    //检查程序进程
    await checkNodProcess();

    let rsdata = await NodUltrain.checkAlive(chainConfig.nodPort);
    if (rsdata != null) {
        logger.info("head_block_num:",rsdata.head_block_num);
        monitor.setHeadBlockNum(rsdata.head_block_num);
    } else {
        logger.error("head_block_num is null");
    }
    //配置文件和monitor配置同时关闭才生效
    if (chainConfig.configFileData.local.enableRestart == false || monitor.needCheckNod() == false) {
        logger.error("local config enable restart("+chainConfig.configFileData.local.enableRestart+") == false || monitor enable restart("+monitor.needCheckNod()+") == false, need not check nod alive");
        return;
    }

    logger.info("local config enable restart == true || monitor enable restart == true,need check nod alive");

    //如果不在进行链切换且本地访问不到本地链信息，需要重启下
    if (syncChainChanging == false) {
        logger.info("checking nod is alive ....");
        logger.debug("check alive data:", rsdata);

        if (utils.isNull(rsdata)) {
            if (nodFailedTimes >= getmaxNodFailedTimes()) {
                nodFailedTimes = 0;
                logger.info("nod is not runing ,need restart it..");
                await restartNod();
                logger.info("nod restart end..");
            } else {

                if (chainConfig.isProducer() == false) {
                    logger.info("I am a none-producer,need to check process");
                    if (processRuning == true) {
                        logger.info("I am a none-producer,and processRuning is running,need not check restart");
                    } else {
                        logger.error("I am a none-producer,and processRuning is not running,need check restart");
                        nodFailedTimes++;
                        logger.info("nod is not alive,count(" + nodFailedTimes + ")");
                    }

                } else {

                    logger.info("I am a producer,and processRuning is "+processRuning);

                    nodFailedTimes++;
                    logger.info("nod is not alive,count(" + nodFailedTimes + ")");
                }

                NodUltrain.getNewestLog(chainConfig.getLocalConfigInfo("nodLogPath", "/log"), function (log) {
                    nodLogData = log;
                    if (utils.isNotNull(nodLogData)) {
                        let l = nodLogData.length;
                        logger.info("get nod log data:", nodLogData.substring(l - 100));
                    }

                });
            }
        } else {
            nodFailedTimes = 0;
            nodLogData = "";
        }
    }
}

/**
 * 清除缓存信息
 */
function clearCacheData() {
    successAccountCacheList.clear();
    failedAccountPramList = [];
    WorldState.status = null;
    nodFailedTimes = 0;
    trxCacheSet.clear()
    monitor.setHeadBlockNum(0);
    monitor.clearConfirmBlock();
}

function clearCache() {
    logger.info("clear chain cache");
    trxCacheSet.clear();
    failedAccountPramList = [];
    successAccountCacheList.clear();
}

/**
 * 链接切换
 * @returns {Promise<void>}
 */
async function switchChain() {

    loggerChainChanging.info("starting to switch chain...");
    let param = [];
    let logMsg = "";
    try {

        param = await monitor.buildParam();
        param.chainNameFrom = chainConfig.localChainName;
        param.chainNameTo = chainConfig.chainName;
        param.startTime = new Date().getTime();

        //停止nod程序
        loggerChainChanging.info("shuting down nod...")
        let result = await NodUltrain.stop(120000,chainConfig.nodPort);
        if (result == false) {
            loggerChainChanging.info("nod is stopped");
            logMsg = utils.addLogStr(logMsg,"nod is stopped");
        } else {
            loggerChainChanging.info("nod is not stopped");
            logMsg = utils.addLogStr(logMsg,"nod is not stopped");
        }

        //停止worldstate的程序
        if (chainConfig.configFileData.local.worldstate == true) {
            result = await WorldState.stop(120000);
            if (result) {
                logger.info("worldstate is stopped");
                logMsg = utils.addLogStr(logMsg,"worldstate is stopped");
            } else {
                logger.info("worldstate is not stopped");
                logMsg = utils.addLogStr(logMsg,"worldstate is not stopped");
            }
        }

        //删除block和shared_memory.bin数据
        await NodUltrain.removeData();
        loggerChainChanging.info("remove block data and shared_memory.bin");
        logMsg = utils.addLogStr(logMsg,"remove block data");
        sleep.msleep(5000);


        //清除世界状态数据
        if (chainConfig.configFileData.local.worldstate == true) {
            await WorldState.clearDB();
            loggerChainChanging.info("remove worldstate data files");
            logMsg = utils.addLogStr(logMsg,"remove worldstate file");
            sleep.msleep(5000);
        }

        //通过chainid拿到seedList
        var seedIpInfo = await chainApi.getChainSeedIP(chainConfig.chainName, chainConfig);
        logger.info("get chainid(" + chainConfig.chainName + ")'s seed ip info:", seedIpInfo);
        if (utils.isNull(seedIpInfo)) {
            loggerChainChanging.error("seed ip info is null");
            logMsg = utils.addLogStr(logMsg,"seed ip info is null");
            syncChainChanging = false;
            monitor.enableDeploy();
            param.endTime = new Date().getTime();
            param.status = 0;
            param.result = logMsg;
            await chainApi.addSwitchLog(monitor.getMonitorUrl(),param);
            return;
        }

        //通过chainid拿到peerkeys
        var chainPeerInfo = await chainApi.getChainPeerKey(chainConfig.chainName, chainConfig);
        logger.info("get chainid(" + chainConfig.chainName + ")'s peer info:", chainPeerInfo);
        if (utils.isNull(chainPeerInfo)) {
            loggerChainChanging.error("chainPeerInfo is null");
            logMsg = utils.addLogStr(logMsg,"chainPeerInfo is null");
        }


        let wssinfo = " ";
        let wssFilePath = null;
        //重启世界状态并拉块
        if (chainConfig.configFileData.local.worldstate == true) {
            logger.info("start world state");
            logMsg = utils.addLogStr(logMsg,"start world state");
            result = await WorldState.start(chainConfig.chainName, seedIpInfo, 120000, utils.formatHomePath(chainConfig.configFileData.local.wsspath),chainConfig.localTest,chainConfig);
            if (result == true) {
                logger.info("start ws success");
                logMsg = utils.addLogStr(logMsg,"start ws success");
            } else {
                logger.info("start ws error");
                logMsg = utils.addLogStr(logMsg,"start ws error");
                // syncChainChanging = false;
                // return;
            }

            sleep.msleep(2000);

            //调用世界状态程序同步数据
            var worldstatedata = null;
            let mainChainData = await chainApi.getTableAllData(chainConfig.config, contractConstants.ULTRAINIO, chainConfig.chainName, tableConstants.WORLDSTATE_HASH, "block_num");
            if (utils.isNotNull(mainChainData) && mainChainData.rows.length > 0) {
                //worldstatedata = mainChainData.rows[mainChainData.rows.length - 1];
                worldstatedata = voteUtil.getMaxValidWorldState(mainChainData.rows);
                logger.info("get worldstate data:", worldstatedata);
            } else {
                logger.error("can not get world state file,or data is null");
                logMsg = utils.addLogStr(logMsg,"ws data is null");
            }

            if (worldstatedata != null) {
                sleep.msleep(1000);
                loggerChainChanging.info("start to require ws:");
                let hash = worldstatedata.hash_v[0].hash;
                let blockNum = worldstatedata.block_num;
                let filesize = worldstatedata.hash_v[0].file_size;
                logger.info("start to require ws : (block num : " + blockNum + " " + "hash:" + hash);
                result = await WorldState.syncWorldState(hash, blockNum, filesize, chainConfig.chainId);
                if (result == true) {
                    logger.info("sync worldstate request success");
                    logMsg = utils.addLogStr(logMsg,"sync ws req success");
                } else {
                    logger.info("sync worldstate request failed");
                    logMsg = utils.addLogStr(logMsg,"sync ws req error");
                }

                loggerChainChanging.info("polling worldstate sync status ..")

                sleep.msleep(1000);

                /**
                 * 轮询检查同步世界状态情况
                 */
                wssFilePath = pathConstants.WSS_DATA+ chainConfig.chainId + "-" + blockNum + ".ws";
                wssinfo = "--worldstate " + pathConstants.WSS_DATA + chainConfig.chainId + "-" + blockNum + ".ws";

                result = await WorldState.pollingkWSState(1000, 1200000);
                if (result == false) {
                    logger.error("require ws error："+wssinfo);
                    logMsg = utils.addLogStr(logMsg,"require ws error");
                    wssinfo = " ";
                    wssFilePath = " ";
                } else {
                    logMsg = utils.addLogStr(logMsg,"require ws success");
                    logger.info("require ws success");
                    logger.info("wssinfo:" + wssinfo);
                    //check file exist
                    if (fs.existsSync(wssFilePath)) {
                        logger.info("file exists :",wssFilePath);
                    } else {
                        logger.error("file not exists :",wssFilePath)
                    }
                }

                sleep.msleep(1000);

                //判断配置是否需要拉块
                if (chainConfig.configFileData.local.wsSyncBlock == true) {
                    logger.info("wsSyncBlock is true，need sync block");
                    logMsg = utils.addLogStr(logMsg,"require block start");
                    /**
                     * 调用block
                     */
                    logger.info("start to sync block:(chainid:" + chainConfig.chainId + ",block num:" + blockNum);
                    result = await WorldState.syncBlocks(chainConfig.chainId, blockNum);
                    if (result == false) {
                        logger.info("sync block request error");
                    } else {
                        logger.info("sync block request success");
                    }

                    sleep.msleep(1000);

                    /**
                     * 轮询检查同步世界状态情况block
                     */
                    logger.info("pollingBlockState start...");
                    result = await WorldState.pollingBlockState(1000, 300000);
                    if (result == false) {
                        logger.info("require block error");
                        logMsg = utils.addLogStr(logMsg,"require block error");
                    } else {
                        logger.info("require block success");
                        logMsg = utils.addLogStr(logMsg,"require block success");
                    }

                    sleep.msleep(1000);

                } else {
                    logger.info("wsSyncBlock is false，need not sync block");
                    logMsg = utils.addLogStr(logMsg,"require block not need");
                    sleep.msleep(3000);
                }
            }
        }


        //修改nod程序配置信息
        var subchainEndPoint = await chainApi.getSubchanEndPoint(chainConfig.chainName, chainConfig);
        var subchainMonitorService = await chainApi.getSubchanMonitorService(chainConfig.chainName, chainConfig);
        logger.info("get chainid(" + chainConfig.chainName + ")'s seed ip info:", seedIpInfo);
        logger.info("subchainEndPoint:", subchainEndPoint);
        logger.info("genesisTime:", chainConfig.genesisTime);
        logger.info("genesisPK:", chainConfig.genesisPK);
        logger.info("get chainid(" + chainConfig.chainName + ")'s", subchainMonitorService);
        result = await NodUltrain.updateConfig(seedIpInfo, subchainEndPoint, chainConfig.genesisTime, chainConfig.genesisPK, subchainMonitorService,chainPeerInfo,chainConfig.chainName);
        if (result == true) {
            logMsg = utils.addLogStr(logMsg,"update nod config success");
            loggerChainChanging.info("update nod config file success")
            //重新加载配置文件信息
            loggerChainChanging.info("reload config files")
            await chainConfig.waitSyncConfig();
            loggerChainChanging.info("reload config files ready")
        } else {
            loggerChainChanging.error("update nod config file error")
            logMsg = utils.addLogStr(logMsg,"update nod config file error");
            syncChainChanging = false;
            monitor.enableDeploy();
            param.endTime = new Date().getTime();
            param.status = 0;
            param.result = logMsg;
            await chainApi.addSwitchLog(monitor.getMonitorUrl(),param);
            return;
        }


        //check file exist
        if (fs.existsSync(wssFilePath)) {
            logger.info("file exists :",wssFilePath);
            logger.info("start nod use wss:",wssinfo);
        } else {
            logger.error("file not exists :",wssFilePath);
            logger.info("start nod not use wss:",wssinfo);
            wssinfo = " "
        }

        //启动nod
        result = await NodUltrain.start(120000, utils.formatHomePath(chainConfig.configFileData.local.nodpath),wssinfo,chainConfig.localTest,chainConfig.nodPort);
        if (result == true) {
            loggerChainChanging.info("nod start success");
            logMsg = utils.addLogStr(logMsg,"nod start success");
        } else {
            loggerChainChanging.error("node start error");
            logMsg = utils.addLogStr(logMsg,"nod start error");
        }

        //等待配置信息同步完成-重新加载配置
        chainConfig.clearChainInfo();
        await chainConfig.waitSyncConfig()

        //结束设置结束flag
        syncChainChanging = false;
        monitor.enableDeploy();
        logMsg = utils.addLogStr(logMsg,"switching chain end");
        loggerChainChanging.info("switching chain successfully...");
        param.endTime = new Date().getTime();
        param.status = 1;
        param.result = logMsg;
        param = monitor.generateSign(param);
        await chainApi.addSwitchLog(monitor.getMonitorUrl(),param);
    } catch (e) {

        loggerChainChanging.info("fail to switch chain...", e);
        //结束设置结束flag
        syncChainChanging = false;
        monitor.enableDeploy();
        param.endTime = new Date().getTime();
        param.status = 0;
        param.result = "error,"+e.toString();
        param = monitor.generateSign(param);
        await chainApi.addSwitchLog(monitor.getMonitorUrl(),param);
    }
}


var nodLogData = "";


/**
 * 重启nod
 * @returns {Promise<void>}
 */
async function restartNod() {

    if (monitor.isDeploying() == true) {
        logger.info("monitor isDeploying, wait to restart");
        sleep.msleep(1000);
        return;
    }

    /**
     * 半小时最多执行一次
     */
    if (checkRestartCount() == false) {
        return;
    }

    restartCount++;

    //producer节点
    if (chainConfig.isNoneProducer() == false) {
        await restartNodProducer();
        return;
    }

    //seed节点
    if (chainConfig.isSeed() == true) {
        await restartSeed();
        return;
    }

    //mongo节点
    if (chainConfig.isMongo() == true) {
        await restartMongo();
        return;
    }

}

/**
 * 重启nod-seed节点
 * @returns {Promise<void>}
 */
async function restartSeed() {
    logger.info("start to restart nod(seed)");

    //标志位设置
    syncChainChanging = true;
    monitor.disableDeploy();


    let param = [];
    let logMsg = "";

    try {
        param = await monitor.buildParam();
        param.chainName = chainConfig.localChainName;
        param.startTime = new Date().getTime();
        param.status = 0;

        logger.info("[seed restart]start to launch nod directly...");
        logMsg = utils.addLogStr(logMsg, "[seed restart]start to launch nod directly...");

        logger.info("[seed restart]stop nod start");
        logMsg = utils.addLogStr(logMsg, "[seed restart]stop nod start");
        //停止nod
        await NodUltrain.stop(120000, chainConfig.nodPort);
        sleep.msleep(1000);
        logger.info("[seed restart]stop nod finish");
        logMsg = utils.addLogStr(logMsg, "[seed restart]stop nod finish");

        //启动nod
        //let result = await NodUltrain.start(10000, utils.formatHomePath(chainConfig.configFileData.local.nodpath), "", chainConfig.localTest,chainConfig.nodPort);
        let result = false;
        if (result == false) {
            //logger.error("[seed restart]node start error");
            //logMsg = utils.addLogStr(logMsg, "[seed restart]nod start error,maybe database is dirty");

            //启动ws
            //停止worldstate的程序
            result = await WorldState.stop(120000);
            if (result) {
                logger.info("[seed restart]worldstate is stopped");
                logMsg = utils.addLogStr(logMsg, "[seed restart]worldstate is stopped");
            } else {
                logger.info("[seed restart]worldstate is not stopped");
                logMsg = utils.addLogStr(logMsg, "[seed restart]worldstate is not stopped");
            }

            //通过chainid拿到seedList
            var seedIpInfo = await chainApi.getChainSeedIP(chainConfig.localChainName, chainConfig);
            logger.info("[seed restart]get chainid(" + chainConfig.localChainName + ")'s seed ip info:", seedIpInfo);
            if (utils.isNull(seedIpInfo)) {
                loggerChainChanging.error("[seed restart]seed ip info is null");
                logMsg = utils.addLogStr(logMsg, "seed ip info is null(chainName:" + chainConfig.localChainName + ")");
            } else {
                logger.info("[seed restart]start world state");
                result = await WorldState.start(chainConfig.localChainName, seedIpInfo, 60000, utils.formatHomePath(chainConfig.configFileData.local.wsspath), chainConfig.localTest, chainConfig);
                if (result == false) {
                    logger.info("[seed restart]start ws error");
                    logMsg = utils.addLogStr(logMsg, "start ws error");
                } else {
                    logger.info("[seed restart]start ws success");
                    logMsg = utils.addLogStr(logMsg, "start ws success");

                    //通过ws获取本地最大块信息
                    let localMaxBlockHeight = await WorldState.getLocalBlockInfo();
                    logger.info("[seed restart]localMaxBlockHeight is:", localMaxBlockHeight);
                    if (utils.isNull(localMaxBlockHeight)) {
                        logMsg = utils.addLogStr(logMsg, "localMaxBlockHeight is null");
                    } else {
                        logMsg = utils.addLogStr(logMsg, "localMaxBlockHeight is " + localMaxBlockHeight);

                        //ws文件ready
                        let wsFileReady = false;

                        //世界状态文件
                        let wssFilePath = "";
                        let wssinfo = "";

                        //调用表数据找到需要下载的世界状态文件信息
                        let wsTableData = await chainApi.getTableAllData(chainConfig.config, contractConstants.ULTRAINIO, chainConfig.localChainName, tableConstants.WORLDSTATE_HASH, "block_num");
                        let wsRes = chainUtil.getNearestWsInfoByBlockHeight(wsTableData, localMaxBlockHeight);
                        if (utils.isNull(wsRes)) {
                            logger.error("[seed restart]get ws info by block height is null");
                            logMsg = utils.addLogStr(logMsg, "get ws info by block height is null");
                        } else {
                            logger.info("[seed restart]get ws info by block height is ", wsRes);
                            logMsg = utils.addLogStr(logMsg, "[seed restart]get ws info by block height is "+wsRes);

                            //世界状态文件
                            wssFilePath = pathConstants.WSS_LOCAL_DATA + chainConfig.configSub.chainId + "-" + wsRes.block_num + ".ws";
                            wssinfo = "--worldstate " + wssFilePath + " --truncate-at-block " + localMaxBlockHeight;

                            //从本地查找ws文件
                            if (fs.existsSync(wssFilePath) == true) {
                                logger.info("[seed restart]ws file exists in local:" + wssFilePath);
                                logMsg = utils.addLogStr(logMsg, "[seed restart]ws file exists in local:"+wssFilePath+")");
                                wsFileReady = true;
                            } else {
                                //本地不存在世界状态文件，需要通过ws拉取
                                logger.error("[seed restart]ws file not exists in local:" + wssFilePath);
                                logMsg = utils.addLogStr(logMsg, "[seed restart]ws file not exists in local:\"+wssFilePath)");

                                wssFilePath = pathConstants.WSS_DATA + chainConfig.configSub.chainId + "-" + wsRes.block_num + ".ws";
                                wssinfo = "--worldstate " + wssFilePath + " --truncate-at-block " + localMaxBlockHeight;

                                //通过seed拉取世界状态
                                result = await WorldState.syncWorldState(wsRes.hash, wsRes.block_num, wsRes.file_size, chainConfig.configSub.chainId);
                                if (result == false) {
                                    logger.error("sync worldstate request failed");
                                    logMsg = utils.addLogStr(logMsg, "[seed restart]sync worldstate request failed");
                                } else {
                                    logger.info("sync worldstate request success");
                                    logMsg = utils.addLogStr(logMsg, "[seed restart]sync worldstate request success");

                                    loggerChainChanging.info("polling worldstate sync status ..")
                                    sleep.msleep(1000);

                                    /**
                                     * 轮询检查同步世界状态情况
                                     */
                                    result = await WorldState.pollingkWSState(1000, 1200000);
                                    if (result == false) {
                                        logMsg = utils.addLogStr(logMsg, "require ws error");
                                        logger.error("require ws error：" + wssinfo);
                                    } else {
                                        logger.info("require ws success");
                                        logMsg = utils.addLogStr(logMsg, "require ws success");
                                        logger.info("wssinfo:" + wssinfo);

                                        //检查文件是否下载成功
                                        if (fs.existsSync(wssFilePath) == false) {
                                            //下载失败
                                            logger.error("[seed restart]file not exists :", wssFilePath);
                                            logger.info("[seed restart]start nod not use wss:", wssinfo);
                                            logMsg = utils.addLogStr(logMsg, "file not exists :" + wssFilePath);
                                        } else {
                                            //下载成功
                                            logger.info("[seed restart]file exists :", wssFilePath);
                                            logger.info("[seed restart]start nod use wss:", wssinfo);
                                            logMsg = utils.addLogStr(logMsg, "file exists :" + wssFilePath + ", start nod use wss:" + wssinfo);
                                            wsFileReady = true;

                                        }
                                    }
                                }
                            }

                            //世界状态文件确认存在
                            if (wsFileReady == true) {

                                //清除state目录
                                WorldState.clearStateDir();
                                logger.info("[seed restart]clear state dir");
                                logMsg = utils.addLogStr(logMsg, "clear state dir");

                                logMsg = utils.addLogStr(logMsg, "start nod :"+wssinfo);
                                //启动nod
                                result = await NodUltrain.start(120000, utils.formatHomePath(chainConfig.configFileData.local.nodpath), wssinfo, chainConfig.localTest, chainConfig.nodPort);
                                if (result == true) {
                                    logger.info("[seed restart]nod start success");
                                    logMsg = utils.addLogStr(logMsg, "nod start success");
                                    param.status = 1;
                                } else {
                                    logger.error("[seed restart]node start error");
                                    logMsg = utils.addLogStr(logMsg, "nod start error");
                                }
                            }

                        }

                        //使用hard-replay方式启动
                        if (utils.isNotNull(localMaxBlockHeight) && wsFileReady == false) {
                            let startinfo = "--hard-replay-blockchain --truncate-at-block "+localMaxBlockHeight;
                            logger.info("[seed restart] localMaxBlockHeight("+localMaxBlockHeight+") is not null,use hard-replay to start nod: "+startinfo);
                            logMsg = utils.addLogStr(logMsg, "[seed restart] localMaxBlockHeight("+localMaxBlockHeight+") is not null,use hard-replay to start nod:"+startinfo);

                            //启动nod
                            result = await NodUltrain.start(120000, utils.formatHomePath(chainConfig.configFileData.local.nodpath), startinfo, chainConfig.localTest, chainConfig.nodPort);
                            if (result == true) {
                                logger.info("[seed restart]nod start success");
                                logMsg = utils.addLogStr(logMsg, "nod start success");
                                param.status = 1;
                            } else {
                                logger.error("[seed restart]node start error");
                                logMsg = utils.addLogStr(logMsg, "nod start error");
                            }
                        }
                    }
                }
            }

        } else {
            //nod 启动成功，结束
            logger.info("nod start success");
            logMsg = utils.addLogStr(logMsg, "nod start success");
            param.status = 1;
        }

    } catch (e) {
        logger.error("restartSeed error");
    }

    //调用接口完成重启
    param.endTime = new Date().getTime();
    param.result = logMsg;
    param.log = nodLogData;
    await chainApi.addRestartLog(monitor.getMonitorUrl(), param);

    //标志位恢复
    syncChainChanging = false;
    monitor.enableDeploy();

    logger.info("finish restart nod(seed)");
}

/**
 * 重启nod-mongo节点
 * @returns {Promise<void>}
 */
async function restartMongo() {
    logger.info("start to restart nod(mongo)");

    //标志位设置
    syncChainChanging = true;
    monitor.disableDeploy();


    let param = [];
    let logMsg = "";

    try {

        param = await monitor.buildParam();
        param.chainName = chainConfig.localChainName;
        param.startTime = new Date().getTime();
        param.status = 0;

        logger.info("[mongo restart]stop nod start");
        logMsg = utils.addLogStr(logMsg, "[mongo restart]stop nod start");
        //停止nod
        await NodUltrain.stop(120000, chainConfig.nodPort);
        sleep.msleep(1000);
        logger.info("[mongo restart]stop nod finish");
        logMsg = utils.addLogStr(logMsg, "[mongo restart]stop nod finish");

        //执行脚本获取mongo中最大的块高
        let mongoMaxBlock = null;
        let mongoPath = "~/mongodb";
        let mongoDBPath = "~/mongodb";

        if (utils.isNotNull(chainConfig.configFileData.local.mongoPath)) {
            mongoPath = chainConfig.configFileData.local.mongoPath;
        }

        if (utils.isNotNull(chainConfig.configFileData.local.mongoDBPath)) {
            mongoDBPath = chainConfig.configFileData.local.mongoDBPath;
        }

        let mongoMaxBlockObj = await mongoUtil.getLocalMongoMaxBlock(600000,mongoPath,mongoDBPath);
        if (mongoMaxBlockObj.code != 0 ) {
            logger.error("[mongo restart]mongoMaxBlock error:",mongoMaxBlockObj);
            logMsg = utils.addLogStr(logMsg, "mongoMaxBlock error:",JSON.stringify(mongoMaxBlockObj));
        } else {
            logger.info("[mongo restart]mongoMaxBlock correct:",mongoMaxBlockObj);
            logMsg = utils.addLogStr(logMsg, "mongoMaxBlock correct:",JSON.stringify(mongoMaxBlockObj));

            mongoMaxBlock = mongoMaxBlockObj.block_num;

            logger.info("[mongo restart]mongo mongoMaxBlock :",mongoMaxBlock);
            logMsg = utils.addLogStr(logMsg, "mongo mongoMaxBlock :"+mongoMaxBlock);

            //停止worldstate的程序
            result = await WorldState.stop(120000);
            if (result) {
                logger.info("[mongo restart]worldstate is stopped");
                logMsg = utils.addLogStr(logMsg, "[mongo restart]worldstate is stopped");
            } else {
                logger.info("[mongo restart]worldstate is not stopped");
                logMsg = utils.addLogStr(logMsg, "[mongo restart]worldstate is not stopped");
            }

            //通过chainid拿到seedList
            var seedIpInfo = await chainApi.getChainSeedIP(chainConfig.localChainName, chainConfig);
            logger.info("[mongo restart]get chainid(" + chainConfig.localChainName + ")'s seed ip info:", seedIpInfo);
            if (utils.isNull(seedIpInfo)) {
                loggerChainChanging.error("[mongo restart]seed ip info is null");
                logMsg = utils.addLogStr(logMsg, "seed ip info is null(chainName:" + chainConfig.localChainName + ")");
            } else {
                logger.info("[mongo restart]start world state");
                result = await WorldState.start(chainConfig.localChainName, seedIpInfo, 60000, utils.formatHomePath(chainConfig.configFileData.local.wsspath), chainConfig.localTest, chainConfig);
                if (result == false) {
                    logger.info("[mongo restart]start ws error");
                    logMsg = utils.addLogStr(logMsg, "start ws error");
                } else {
                    logger.info("[mongo restart]start ws success");
                    logMsg = utils.addLogStr(logMsg, "start ws success");

                    //通过ws获取本地最大块信息
                    let localMaxBlockHeight = await WorldState.getLocalBlockInfo();
                    logger.info("[mongo restart]localMaxBlockHeight is:", localMaxBlockHeight);
                    if (utils.isNull(localMaxBlockHeight)) {
                        logMsg = utils.addLogStr(logMsg, "localMaxBlockHeight is null");
                    } else {
                        logMsg = utils.addLogStr(logMsg, "localMaxBlockHeight is " + localMaxBlockHeight);

                        //ws文件ready
                        let wsFileReady = false;

                        //世界状态文件
                        let wssFilePath = "";
                        let wssinfo = "";

                        let minNum = utils.calcMin(localMaxBlockHeight,mongoMaxBlock);
                        logger.info("[mongo restart]minNum is ",minNum);
                        logMsg = utils.addLogStr(logMsg, "minNum is "+minNum);

                        //调用表数据找到需要下载的世界状态文件信息
                        let wsTableData = await chainApi.getTableAllData(chainConfig.config, contractConstants.ULTRAINIO, chainConfig.localChainName, tableConstants.WORLDSTATE_HASH, "block_num");
                        let wsRes = chainUtil.getNearestWsInfoByBlockHeight(wsTableData, minNum);
                        if (utils.isNull(wsRes)) {
                            logger.error("[mongo restart]get ws info by block height is null");
                            logMsg = utils.addLogStr(logMsg, "get ws info by block height is null");
                        } else {
                            logger.info("[mongo restart]get ws info by block height is ", wsRes);
                            logMsg = utils.addLogStr(logMsg, "[mongo restart]get ws info by block height is "+wsRes);

                            //世界状态文件
                            wssFilePath = pathConstants.WSS_LOCAL_DATA + chainConfig.configSub.chainId + "-" + wsRes.block_num + ".ws";
                            wssinfo = "--worldstate " + wssFilePath + " --truncate-at-block " + localMaxBlockHeight;

                            //从本地查找ws文件
                            if (fs.existsSync(wssFilePath) == true) {
                                logger.info("[seed restart]ws file exists in local:" + wssFilePath);
                                logMsg = utils.addLogStr(logMsg, "[seed restart]ws file exists in local:"+wssFilePath+")");
                                wsFileReady = true;
                            } else {
                                //本地不存在世界状态文件，需要通过ws拉取
                                logger.error("[seed restart]ws file not exists in local:" + wssFilePath);
                                logMsg = utils.addLogStr(logMsg, "[seed restart]ws file not exists in local:\"+wssFilePath)");

                                wssFilePath = pathConstants.WSS_DATA + chainConfig.configSub.chainId + "-" + wsRes.block_num + ".ws";
                                wssinfo = "--worldstate " + wssFilePath + " --truncate-at-block " + localMaxBlockHeight;

                                //通过seed拉取世界状态
                                result = await WorldState.syncWorldState(wsRes.hash, wsRes.block_num, wsRes.file_size, chainConfig.configSub.chainId);
                                if (result == false) {
                                    logger.error("sync worldstate request failed");
                                    logMsg = utils.addLogStr(logMsg, "[seed restart]sync worldstate request failed");
                                } else {
                                    logger.info("sync worldstate request success");
                                    logMsg = utils.addLogStr(logMsg, "[seed restart]sync worldstate request success");

                                    loggerChainChanging.info("polling worldstate sync status ..")
                                    sleep.msleep(1000);

                                    /**
                                     * 轮询检查同步世界状态情况
                                     */
                                    result = await WorldState.pollingkWSState(1000, 1200000);
                                    if (result == false) {
                                        logMsg = utils.addLogStr(logMsg, "require ws error");
                                        logger.error("require ws error：" + wssinfo);
                                    } else {
                                        logger.info("require ws success");
                                        logMsg = utils.addLogStr(logMsg, "require ws success");
                                        logger.info("wssinfo:" + wssinfo);

                                        //检查文件是否下载成功
                                        if (fs.existsSync(wssFilePath) == false) {
                                            //下载失败
                                            logger.error("[mongo restart]file not exists :", wssFilePath);
                                            logger.info("[mongo restart]start nod not use wss:", wssinfo);
                                            logMsg = utils.addLogStr(logMsg, "file not exists :" + wssFilePath);
                                        } else {
                                            //下载成功
                                            logger.info("[mongo restart]file exists :", wssFilePath);
                                            logger.info("[mongo restart]start nod use wss:", wssinfo);
                                            logMsg = utils.addLogStr(logMsg, "file exists :" + wssFilePath + ", start nod use wss:" + wssinfo);
                                            wsFileReady = true;

                                        }
                                    }
                                }
                            }

                            //世界状态文件确认存在
                            if (wsFileReady == true) {

                                //清除state目录
                                WorldState.clearStateDir();
                                logger.info("[seed restart]clear state dir");
                                logMsg = utils.addLogStr(logMsg, "clear state dir");

                                logMsg = utils.addLogStr(logMsg, "start nod :"+wssinfo);

                                //更新nod配置，增加
                                let resUpdateConfig = NodUltrain.updateMongoStartNum(mongoMaxBlock);
                                if (resUpdateConfig == false) {
                                    logger.error("updateMongoStartNum error");
                                    utils.addLogStr(logMsg, "updateMongoStartNum error");
                                } else {
                                    logger.info("updateMongoStartNum ",mongoMaxBlock);
                                    utils.addLogStr(logMsg, "updateMongoStartNum "+mongoMaxBlock);
                                    //启动nod
                                    result = await NodUltrain.start(120000, utils.formatHomePath(chainConfig.configFileData.local.nodpath), wssinfo, chainConfig.localTest, chainConfig.nodPort);
                                    if (result == true) {
                                        logger.info("[mongo restart]nod start success");
                                        logMsg = utils.addLogStr(logMsg, "nod start success");
                                        param.status = 1;
                                    } else {
                                        logger.error("[mongo restart]node start error");
                                        logMsg = utils.addLogStr(logMsg, "nod start error");
                                    }
                                }
                            }

                        }

                        //使用hard-replay方式启动
                        if (utils.isNotNull(localMaxBlockHeight) && wsFileReady == false) {
                            let startinfo = "--hard-replay-blockchain --truncate-at-block "+localMaxBlockHeight;
                            logger.info("[mongo restart] localMaxBlockHeight("+localMaxBlockHeight+") is not null,use hard-replay to start nod: "+startinfo);
                            logMsg = utils.addLogStr(logMsg, "[mongo restart] localMaxBlockHeight("+localMaxBlockHeight+") is not null,use hard-replay to start nod:"+startinfo);

                            //更新nod配置，增加
                            let resUpdateConfig = NodUltrain.updateMongoStartNum(mongoMaxBlock);
                            if (resUpdateConfig == false) {
                                logger.error("updateMongoStartNum error");
                                utils.addLogStr(logMsg, "updateMongoStartNum error");
                            } else {

                                logger.info("updateMongoStartNum ",mongoMaxBlock);
                                utils.addLogStr(logMsg, "updateMongoStartNum "+mongoMaxBlock);

                                //启动nod
                                result = await NodUltrain.start(120000, utils.formatHomePath(chainConfig.configFileData.local.nodpath), startinfo, chainConfig.localTest, chainConfig.nodPort);
                                if (result == true) {
                                    logger.info("[mongo restart]nod start success");
                                    logMsg = utils.addLogStr(logMsg, "nod start success");
                                    param.status = 1;
                                } else {
                                    logger.error("[mongo restart]node start error");
                                    logMsg = utils.addLogStr(logMsg, "nod start error");
                                }
                            }
                        }
                    }
                }
            }


        }

    } catch (e) {
        logger.error("restartMongo error");
    }

    //调用接口完成重启
    param.endTime = new Date().getTime();
    param.result = logMsg;
    param.log = nodLogData;
    await chainApi.addRestartLog(monitor.getMonitorUrl(), param);

    //标志位恢复
    syncChainChanging = false;
    monitor.enableDeploy();

    logger.info("finish restart nod(mongo)");
}

/**
 * 重启nod-矿工节点
 * @returns {Promise<void>}
 */
async function restartNodProducer() {

    //标志位设置
    syncChainChanging = true;
    monitor.disableDeploy();


    logger.info("start to restart nod(producer)");
    let param = [];
    let logMsg = "";
    try {

        let wssinfo = " ";
        let wssFilePath = " ";
        param = await monitor.buildParam();
        param.chainName = chainConfig.localChainName;
        param.startTime = new Date().getTime();
        //使用world state to recover
        if (chainConfig.configFileData.local.worldstate == true && chainConfig.isNoneProducer() == false) {

            logger.info("world state is on("+chainConfig.configFileData.local.worldstate+") && this nod is not none-producer("+chainConfig.configFileData.target["is-non-producing-node"]+") ,need  use ws to recover",);
            logMsg = utils.addLogStr(logMsg,"world state is on("+chainConfig.configFileData.local.worldstate+") && this nod is not none-producer("+chainConfig.configFileData.target["is-non-producing-node"]+") ,need  use ws to recover");

            //停止worldstate的程序
            if (chainConfig.configFileData.local.worldstate == true) {
                result = await WorldState.stop(120000);
                if (result) {
                    logger.info("worldstate is stopped");
                    logMsg = utils.addLogStr(logMsg,"worldstate is stopped");
                } else {
                    logger.info("worldstate is not stopped");
                    logMsg = utils.addLogStr(logMsg,"worldstate is not stopped");
                }
            }

            //通过chainid拿到seedList
            var seedIpInfo = await chainApi.getChainSeedIP(chainConfig.localChainName, chainConfig);
            logger.info("get chainid(" + chainConfig.localChainName + ")'s seed ip info:", seedIpInfo);
            let wsStarted = true;
            if (utils.isNull(seedIpInfo)) {
                loggerChainChanging.error("seed ip info is null");
                logMsg = utils.addLogStr(logMsg,"seed ip info is null(chainName:"+chainConfig.localChainName+")");
            } else {
                logger.info("start world state");
                result = await WorldState.start(chainConfig.localChainName, seedIpInfo, 120000, utils.formatHomePath(chainConfig.configFileData.local.wsspath), chainConfig.localTest,chainConfig);
                if (result == true) {
                    logger.info("start ws success");
                    logger.info("world state is on , use world state to revocer");
                    logMsg = utils.addLogStr(logMsg,"start ws success");
                } else {
                    logger.info("start ws error");
                    logger.info("world state is off");
                    logMsg = utils.addLogStr(logMsg,"start ws error");
                    wsStarted = false;
                }
            }

            if (wsStarted == true) {
                //调用世界状态程序同步数据
                var worldstatedata = null;
                let maxBlockHeight = 0;
                let mainChainData = await chainApi.getTableAllData(chainConfig.config, contractConstants.ULTRAINIO, chainConfig.localChainName, tableConstants.WORLDSTATE_HASH, "block_num");
                if (utils.isNotNull(mainChainData) && mainChainData.rows.length > 0) {
                    //worldstatedata = mainChainData.rows[mainChainData.rows.length - 1];
                    worldstatedata = voteUtil.getMaxValidWorldState(mainChainData.rows);
                    logger.info("get worldstate data:", worldstatedata);
                    if (worldstatedata != null) {
                        maxBlockHeight = worldstatedata.block_num;
                    }
                } else {
                    logger.error("can not get world state file,or data is null");
                    logMsg = utils.addLogStr(logMsg,"can not get world state file,or data is null");
                }

                //查看本地ws最大文件
                let localHashIsMax = false;
                if (utils.isNotNull(WorldState.status)) {
                    logger.info("local ws status:", WorldState.status);
                    logger.info("local ws max block heght(" + WorldState.status.block_height + "),mainchain max block height(" + maxBlockHeight + ")")

                    logMsg = utils.addLogStr(logMsg,"local ws max block heght(" + WorldState.status.block_height + "),mainchain max block height(" + maxBlockHeight + ")");
                    if (WorldState.status.block_height == maxBlockHeight) {
                        logger.info("block height is equal, check ws file");
                        logMsg = utils.addLogStr(logMsg,"block height is equal, check ws file");
                        wssFilePath = pathConstants.WSS_LOCAL_DATA + chainConfig.configSub.chainId + "-" + WorldState.status.block_height + ".ws";
                        logger.info("file path:", wssFilePath);
                        if (fs.existsSync(wssFilePath)) {
                            logger.info("file path exists:", wssFilePath);
                            logMsg = utils.addLogStr(logMsg,"file path  exists:", wssFilePath);
                            wssinfo = "--worldstate " + pathConstants.WSS_LOCAL_DATA + chainConfig.configSub.chainId + "-" + WorldState.status.block_height + ".ws";
                            localHashIsMax = true;
                        } else {
                            logger.info("file path not exists:", wssFilePath);
                            logMsg = utils.addLogStr(logMsg,"file path not exists:", wssFilePath);

                        }
                    } else {
                        logger.info("block height is not equal,not use local ws file to start");
                        logMsg = utils.addLogStr(logMsg,"block height is not equal,not use local ws file to start");
                    }
                } else {
                    logger.info("local ws status is null");
                    logMsg = utils.addLogStr(logMsg,"local ws status is null");
                }

                //本地没有最新，拉取最新的
                if (localHashIsMax == false && worldstatedata != null) {
                    sleep.msleep(1000);
                    logger.info("start to require ws:");
                    let hash = worldstatedata.hash_v[0].hash;
                    let blockNum = worldstatedata.block_num;
                    let filesize = worldstatedata.hash_v[0].file_size;
                    logger.info("start to require ws : (block num : " + blockNum + " " + "hash:" + hash);
                    result = await WorldState.syncWorldState(hash, blockNum, filesize, chainConfig.configSub.chainId);
                    if (result == true) {
                        logger.info("sync worldstate request success");
                    } else {
                        logger.info("sync worldstate request failed");
                    }

                    loggerChainChanging.info("polling worldstate sync status ..")

                    sleep.msleep(1000);

                    /**
                     * 轮询检查同步世界状态情况
                     */
                    wssFilePath = pathConstants.WSS_DATA + chainConfig.configSub.chainId + "-" + blockNum + ".ws";
                    wssinfo = "--worldstate " + pathConstants.WSS_DATA + chainConfig.configSub.chainId + "-" + blockNum + ".ws";

                    result = await WorldState.pollingkWSState(1000, 1200000);
                    if (result == false) {
                        logMsg = utils.addLogStr(logMsg,"require ws error");
                        logger.error("require ws error：" + wssinfo);
                        wssinfo = " ";
                        wssFilePath = " ";
                    } else {
                        logger.info("require ws success");
                        logger.info("wssinfo:" + wssinfo);
                        //check file exist
                        if (fs.existsSync(wssFilePath)) {
                            logger.info("file exists :", wssFilePath);
                        } else {
                            logger.error("file not exists :", wssFilePath)
                        }
                    }

                    sleep.msleep(1000);

                    //判断配置是否需要拉块
                    if (chainConfig.configFileData.local.wsSyncBlock == true) {
                        logger.info("wsSyncBlock is true，need sync block");
                        /**
                         * 调用block
                         */
                        logger.info("start to sync block:(chainid:" + chainConfig.configSub.chainId + ",block num:" + blockNum);
                        result = await WorldState.syncBlocks(chainConfig.configSub.chainId, blockNum);
                        if (result == false) {
                            logger.info("sync block request error");
                        } else {
                            logger.info("sync block request success");
                        }

                        sleep.msleep(1000);

                        /**
                         * 轮询检查同步世界状态情况block
                         */
                        logger.info("pollingBlockState start...");
                        result = await WorldState.pollingBlockState(1000, 300000);
                        if (result == false) {
                            logger.info("require block error");
                        } else {
                            logger.info("require block success");
                        }

                        sleep.msleep(1000);

                    } else {
                        logger.info("wsSyncBlock is false，need not sync block");
                        sleep.msleep(3000);
                    }
                }

            }
        } else {

            logger.info("world state may be off("+chainConfig.configFileData.local.worldstate+") || this nod is none-producer("+chainConfig.configFileData.target["is-non-producing-node"]+") ,need not use ws to recover",);

            logMsg = utils.addLogStr(logMsg,"world state may be off("+chainConfig.configFileData.local.worldstate+") || this nod is none-producer("+chainConfig.configFileData.target["is-non-producing-node"]+") ,need not use ws to recover");
        }

        //启动nod
        await NodUltrain.stop(120000,chainConfig.nodPort);
        sleep.msleep(1000);
        logger.info("clear Nod DB data before restart it..");
        logMsg = utils.addLogStr(logMsg,"clear Nod DB data");
        await NodUltrain.removeData();

        // if (chainConfig.configFileData.local.worldstate == true) {
        //     await WorldState.clearDB();
        // }

        //check file exist
        if (fs.existsSync(wssFilePath)) {
            logger.info("file exists :", wssFilePath);
            logger.info("start nod use wss:", wssinfo);
            logMsg = utils.addLogStr(logMsg,"file exists :"+wssFilePath+", start nod use wss:"+wssinfo);
        } else {
            logger.error("file not exists :", wssFilePath);
            logger.info("start nod not use wss:", wssinfo);
            logMsg = utils.addLogStr(logMsg,"file not exists :"+wssFilePath+", start nod not use wss:");
            wssinfo = " "
        }

        //启动nod
        result = await NodUltrain.start(120000, utils.formatHomePath(chainConfig.configFileData.local.nodpath), wssinfo, chainConfig.localTest,chainConfig.nodPort);
        if (result == true) {
            logger.info("nod start success");
            logMsg = utils.addLogStr(logMsg,"nod start success");
        } else {
            logger.error("node start error");
            logMsg = utils.addLogStr(logMsg,"nod start error");
        }

        param.status = 1;

    } catch (e) {
        param.status = 0;
        logMsg = utils.addLogStr(logMsg,"exception:"+e.toString());
        logger.error("restartNod error,",e);
    }

    param.endTime = new Date().getTime();
    param.result = logMsg;
    //todo
    param.log = nodLogData;
    await chainApi.addRestartLog(monitor.getMonitorUrl(),param);

    nodLogData = "";

    syncChainChanging = false;

    monitor.enableDeploy();

}
/**
 * 同步资源（全表）
 * @returns {Promise<void>}
 */
async function syncAllResource() {
    await syncResource(true);
}

/**
 * 同步资源（最新半小时）
 * @returns {Promise<void>}
 */
async function syncNewestResource() {
    await syncResource(false);
}

/**
 * 同步资源
 * @returns {Promise<void>}
 */
async function syncResource(allFlag) {
    logger.info("syncResource start");
    if (syncChainData == true && isMainChain() == false) {

        let voteCount = 0;

        let changeList = [];
        /**
         * 全表对比
         */
        if (allFlag == true) {
            logger.info("sync all resource");

            /**
             * 手动开关不同步
             */
            if (monitor.needSyncUserRes() == false) {
                logger.error("sync all resource is disabled by flag control");
                return;
            }

            //获取子链上所有资源的信息
            let subChainResourceList = await chainApi.getTableAllData(chainConfig.configSub, contractConstants.ULTRAINIO, scopeConstants.SCOPE_MAIN_CHAIN, tableConstants.RESOURCE_LEASE, "owner");
            logger.info("subChainResourceList:", subChainResourceList);

            //获取主链上所有资源的信息
            let mainChainResourceList = await chainApi.getTableAllData(chainConfig.config, contractConstants.ULTRAINIO, chainConfig.localChainName, tableConstants.RESOURCE_LEASE, "owner");
            logger.info("mainChainResourceList:", mainChainResourceList);

            //对比两张表，获取更新的信息
            changeList = await voteUtil.genVoteResList(subChainResourceList, mainChainResourceList, chainConfig);
        } else {
            logger.info("sync newest resource");
            /**
             * 从公告栏上获取最新的资源
             */
            let newestChangeList = await chainApi.getSubchainResource(chainConfig.localChainName,chainConfig);
            logger.info("newestChangeList:",newestChangeList.length);

            if (utils.isNullList(newestChangeList) == false && newestChangeList.length > 0) {
                for (var i = 0; i < newestChangeList.length; i++) {
                    let resObj = newestChangeList[i];
                    logger.debug("resobj:",resObj);
                    let localtableObj = await chainApi.getTableInfo(chainConfig.getLocalHttpEndpoint(), contractConstants.ULTRAINIO, chainNameConstants.MAIN_CHAIN_NAME, tableConstants.RESOURCE_LEASE, null, resObj.owner, null, null);
                    let localresObj = null;
                    if (localtableObj != null && localtableObj.rows != null && localtableObj.rows.length > 0) {
                        localresObj = localtableObj.rows[0];
                    }
                    logger.debug("localresObj:",localresObj);
                    if (utils.isNull(localresObj)) {
                        logger.debug("subchain has no res of owner:"+resObj.owner+",need add to array");
                        changeList.push(resObj);
                    } else {
                        logger.debug("subchain has  res of owner:"+resObj.owner+",need check");
                        if (chainUtil.isResourceChanged(resObj,localresObj,chainConfig)) {
                            logger.debug("subchain has  res of owner:"+resObj.owner+" has not synced,need add to array");
                            changeList.push(resObj);
                        } else {
                            logger.debug("subchain has  res of owner:"+resObj.owner+" has already synced");
                        }
                    }
                }
            }
        }

        logger.info("change resList:", changeList);

        //获取主链上所有资源的信息
        if (changeList.length > 0) {

            //使用块头同步来控制资源同步
            if (monitor.needSyncUserResByBlock() == true) {

                logger.info("sync newest res by block is enabled by control flag");

                //增加本轮是否是出块节点的判断
                var headBlockProposer = await chainApi.getHeadBlockProposer(chainConfig.configSub);
                //判断是否要上传块头
                if (blockUtil.needPushBlockByProducerList(headBlockProposer, chainConfig.myAccountAsCommittee, localProducers) == false) {
                    logger.info("[sync newest res]headBlockProposer("+headBlockProposer+") is not myself("+chainConfig.myAccountAsCommittee+"),do not sync newest res");
                    return;
                }

                let syncNumCount = 0;
                if (changeList.length == 0) {
                    logger.error("[sync newest res] changeList.length  is null");
                } else {

                    //执行结果
                    var syncCountRes = {
                        totalNum: 0,
                        successAccountNum: 0,
                        syncBlockNum: 0,
                        blockNotReadyNum:0,
                    }

                    let maxConfirmBlockNum = monitor.getConfirmBlockMaster();
                    if (maxConfirmBlockNum <= 0) {
                        maxConfirmBlockNum = await getMaxConfirmBlock(chainConfig.configSub, chainNameConstants.MAIN_CHAIN_NAME_TRANSFER);
                    }
                    logger.error("[sync newest res]maxConfirmBlockNum master in subchain is :",maxConfirmBlockNum);
                    for (var i = 0; i < changeList.length; i++) {
                        syncCountRes.totalNum++;
                        let resObj = changeList[i];
                        let blockHeight = resObj.modify_block_height;

                        //跨高不能小于已同步的块告
                        if (blockHeight > maxConfirmBlockNum) {
                            logger.error("([sync newest res]blockheight:" + blockHeight + "> maxConfirmBlockNum : "+maxConfirmBlockNum+" ,need not work");
                            syncCountRes.blockNotReadyNum++;
                            continue;
                        } else {
                            logger.info("([sync newest res]blockheight:" + blockHeight + "<= maxConfirmBlockNum : "+maxConfirmBlockNum+" ,need work");
                        }

                        if (syncNumCount >= monitor.getSyncBlockHeaderMaxTranNum()) {
                            logger.error("syncNumCount("+syncNumCount+") >= maxnum("+monitor.getSyncBlockHeaderMaxTranNum()+"),need break");
                            break;
                        }

                        //获取主链的块信息
                        let blockInfo = await chainApi.getBlockInfoData(chainConfig.config.httpEndpoint,(blockHeight).toString());
                        logger.debug("[sync newest res]block info:", blockInfo);
                        let trans = chainUtil.getSyncResTransFromBlockHeader(blockInfo, chainConfig.localChainName,resObj.owner);
                        logger.info("[sync newest res]find trans length:", trans.length);
                        //调用MerkleProof
                        for (let t = 0; t < trans.length; t++) {
                            let tranId = trans[t].trx.id;
                            if (trxCacheSet.has(tranId) == true) {
                                logger.info("([sync newest res]blockheight:" + blockHeight + ",trxid:" + tranId + " trx-m-root  is in cache,need not work");
                                syncCountRes.successAccountNum++;
                                continue;
                            }

                            logger.info("([sync newest res]blockheight:" + blockHeight + ",trxid:" + tranId + " is not in cache,need query table check is ready");
                            logger.debug("[sync newest res]getMerkleProof(blockheight:" + blockHeight + ",trxid:" + tranId);
                            let merkleProof = await chainApi.getMerkleProof(chainConfig.config, blockHeight, tranId);
                            logger.debug("[sync newest res]merkleProof:", merkleProof);

                            if (utils.isNotNull(merkleProof)) {
                                logger.debug("[sync newest res]merkleProof trx_receipt_bytes:", merkleProof.trx_receipt_bytes);
                                let tx_bytes_array = chainUtil.transferTrxReceiptBytesToArray(merkleProof.trx_receipt_bytes);
                                logger.debug("[sync newest res]merkleProof trx_receipt_bytes convert to array length:", tx_bytes_array.length)
                                logger.debug("merkleProof trx_receipt_bytes convert to array:", tx_bytes_array.toString());

                                let blockHeightInfo = await chainApi.getBlockHeaderInfo(chainConfig.getLocalHttpEndpoint(),chainNameConstants.MAIN_CHAIN_NAME_TRANSFER,blockHeight);
                                logger.debug("[sync newest res]master blockHeightInfo("+blockHeight+"):",blockHeightInfo);
                                let hashIsReady = checkHashIsready(blockHeightInfo,tranId);
                                if (hashIsReady == true) {
                                    logger.info("[sync newest res]master blockHeightInfo("+blockHeight+") trx id : "+tranId+", is ready, need not push");
                                    trxCacheSet.add(tranId);
                                    syncCountRes.successAccountNum++;
                                } else {
                                    logger.info("[sync newest res]master blockHeightInfo(" + blockHeight + ") trx id : " + tranId + ", is not ready, need push");


                                    syncCountRes.syncBlockNum++;
                                    syncNumCount++;
                                    if (syncNumCount >= monitor.getSyncBlockHeaderMaxTranNum()) {
                                        logger.error("[sync newest res]syncNumCount(" + syncNumCount + ") >= maxnum(" + monitor.getSyncBlockHeaderMaxTranNum() + "),need break");
                                        break;
                                    }

                                    let param = {
                                        chain_name: chainNameConstants.MAIN_CHAIN_NAME_TRANSFER,
                                        block_number: blockHeight,
                                        merkle_proofs: merkleProof.merkle_proof,
                                        tx_bytes: tx_bytes_array.toString()
                                    }
                                    logger.info("[sync newest res]prepare to push sync  transfer trx:", param);

                                    param = {
                                        chain_name: chainNameConstants.MAIN_CHAIN_NAME_TRANSFER,
                                        block_number: blockHeight,
                                        merkle_proofs: merkleProof.merkle_proof,
                                        tx_bytes: tx_bytes_array
                                    }

                                    let res = await chainApi.contractInteract(chainConfig.configSub, contractConstants.ULTRAINIO, "synclwctx", param, chainConfig.myAccountAsCommittee, chainConfig.config.keyProvider[0]);
                                    logger.info("[sync newest res]synclwctx res:", res);
                                }
                            } else {
                                logger.error("[sync newest res]merkleProof is null");
                            }

                        }


                    }

                    logger.info("[sync newest res] res info:",syncCountRes);
                }

            } else {
                logger.error("sync newest res by block is disabled by control flag");
            }
        }
    }
    logger.info("syncResource end");
}

/**
 * 是否是主链
 * @returns {Promise<boolean>}
 */
function isMainChain() {
    return chainNameConstants.MAIN_CHAIN_NAME == chainConfig.localChainName;
}

/**
 * 同步世界状态
 * @returns {Promise<void>}
 */
async function syncWorldState() {

    try {

        await WorldState.syncStatus();
        monitor.setHashInfo(WorldState.status.block_height, WorldState.status.hash_string)

        if (chainConfig.configFileData.local.worldstate == false) {
            logger.info("syncWorldState is disabled");
            return;
        }
        logger.info("syncWorldState start");

        if (syncChainData == true) {
            let vaildMaxWSNum = 0;
            try {
                //同步状态
                logger.info("WorldState.status:", WorldState.status);
                logger.info("WorldState.status chain_id:", WorldState.status.chain_id);
                if (utils.isNotNull(WorldState.status) && utils.isNotNull(WorldState.status.chain_id) && WorldState.status.chain_id != chainIdConstants.NONE_CHAIN) {
                    logger.info("WorldState.status not null");
                    //调用主链查询当前已同步的块高
                    //let mainChainData = await chainApi.getTableInfo(chainConfig.config, contractConstants.ULTRAINIO, chainConfig.chainName, tableConstants.WORLDSTATE_HASH,1000,null,null,null);
                    let mainChainData = await chainApi.getTableAllData(chainConfig.config, contractConstants.ULTRAINIO, chainConfig.localChainName, tableConstants.WORLDSTATE_HASH, "block_num");

                    logger.error("mainChainData:", mainChainData);
                    let needUpload = true;
                    if (utils.isNotNull(mainChainData) && mainChainData.rows.length > 0) {
                        logger.debug("mainChainData:", mainChainData);
                        let worldstatedata = voteUtil.getMaxValidWorldState(mainChainData.rows);
                        if (worldstatedata != null) {
                            vaildMaxWSNum = worldstatedata.block_num;
                            logger.info("main chain's world state (main chain block num :" + worldstatedata.block_num + " subchain node block num :" + WorldState.status.block_height + ")");
                            if (worldstatedata.block_num >= WorldState.status.block_height) {
                                logger.info("main chain's world state is newest,need not upload:(main chain block num :" + worldstatedata.block_num + " subchain node block num :" + WorldState.status.block_height + ")");
                                needUpload = false;
                            }
                        }
                    } else {
                        logger.info("main chain's world state is null,need upload");
                        needUpload = true;
                    }
                    //需要上传
                    if (needUpload) {
                        let params = {
                            subchain: chainConfig.localChainName,
                            blocknum: WorldState.status.block_height,
                            hash: WorldState.status.hash_string,
                            file_size: WorldState.status.file_size
                        }

                        logger.info("reportsubchainhash params:", params);
                        let result = await chainApi.contractInteract(chainConfig.config, contractConstants.ULTRAINIO, "reportsubchainhash", params, chainConfig.myAccountAsCommittee, chainConfig.config.keyProvider[0]);
                        logger.info("upload ws hash to main chain:" + result);
                    }
                } else {
                    logger.info("local world state is none ,need not upload");
                }

                /**
                 *
                 */
                if (vaildMaxWSNum > 0) {
                    logger.info("vaildMaxWSNum is " + vaildMaxWSNum + ", need set wss..");
                    await WorldState.setValidWs(vaildMaxWSNum);
                } else {
                    logger.info("vaildMaxWSNum is " + vaildMaxWSNum + ", need not set wss..");
                }

            } catch (e) {
                logger.error("syncWorldState error:", e);
            }
        } else {
            logger.info("syncWorldState not need:", syncChainData);
        }

        logger.info("syncWorldState end");

    } catch (e) {
        logger.error("syncWorldState error:", e);
    }
}

/**
 * 清空重启次数，半小时最多做一次
 */
function clearRestartCount() {
    logger.info("clearRestartCount start");
    restartCount = 0;
}

/**
 * 检查是否需要去重启
 * @returns {boolean}
 */
function checkRestartCount() {
    if (restartCount >0) {
        logger.error("RestartCount("+restartCount+") > 0, need not restart");
        return false;
    }

    logger.error("RestartCount("+restartCount+") == 0, need restart");

    return true;
}




module.exports = {
    isMainChain,
    syncBlock,
    syncChainInfo,
    syncUser,
    syncResource,
    syncWorldState,
    syncAllResource,
    syncNewestResource,
    syncUgas,
    clearCache,
    syncCommitee,
    clearRestartCount,
}
