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
var successAccountCacheList = [];

//存储同步时失败的
var failedAccountPramList = [];

//nod请求失败次数
var nodFailedTimes=0;
var maxNodFailedTimes=5;

//每一轮用户/资源投票数
var maxVoteCountOneRound = 5;


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

/**
 *
 * @returns {*}
 */
function getMaxVoteCountOneRound() {

    if (utils.isNotNull(chainConfig.configFileData.local.maxVoteCountOneRound)) {
        return chainConfig.configFileData.local.maxVoteCountOneRound;
    }

    return maxVoteCountOneRound;
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

        if (utils.isNullList(userBulletinList)) {
            logger.info("userBulletinList is null ,check failed list count :",failedAccountPramList.length);
            if (failedAccountPramList.length >0) {
                failedAccountPramList.forEach(function (item,index) {
                    userBulletinList.push(item);
                })

                logger.info("retry user userBulletinList:", userBulletinList);
            }
        }

        //投票结果
        var userCountRes = {
            totalNum: 0,
            successAccountNum: 0,
            hasVotedNum: 0,
            votingNum: 0
        }

        for (var i in userBulletinList) {

            if (voteCount > getMaxVoteCountOneRound()) {
                logger.error("vote count >=("+getMaxVoteCountOneRound()+"),stop sync user.");
                break;
            }

            userCountRes.totalNum++;
            var newUser = userBulletinList[i];
            const params = {
                proposer: chainConfig.myAccountAsCommittee,
                proposeaccount: [{
                    account: newUser.owner,
                    owner_key: newUser.owner_pk,
                    active_key: newUser.active_pk,
                    location: constants.chainNameConstants.MAIN_CHAIN_NAME,
                    approve_num: 0,
                    updateable: utils.isNotNull(newUser.updateable) ? newUser.updateable : 1 //该账号后续部署合约能否被更新
                }]
            };

            logger.info("=======voteAccount to subchain:", newUser.owner);

            //检查缓存中是否有
            var hitFlag = successAccountCacheList.find((value, index, arr) => {
                return Object.is(newUser.owner, value)
            })

            //检查子链是否已经有改账户
            if (utils.isNull(hitFlag) && utils.isNull(await chainApi.getAccount(chainConfig.configSub.httpEndpoint, newUser.owner))) {
                logger.info("account(" + newUser.owner + ") is not ready,need vote..");

                //查询是否已经投过票
                let tableData = await chainApi.getTableInfo(chainConfig.configSub, contractConstants.ULTRAINIO, contractConstants.ULTRAINIO, tableConstants.PENDING_ACCOUNT, null, newUser.owner, null, null);
                logger.debug(tableData)
                if (voteUtil.findVoteRecord(tableData, chainConfig.myAccountAsCommittee, newUser.owner,) == false) {

                    //未找到投过票的记录，需要投票
                    logger.info("account(" + newUser.owner + ") has not been voted by " + chainConfig.myAccountAsCommittee + ", start voting....");

                    //发起投票合约action
                    //logger.debug("vote params ",params);
                    let res = await chainApi.contractInteract(chainConfig.configSub, contractConstants.ULTRAINIO, actionConstants.VOTE_ACCOUNT, params, chainConfig.myAccountAsCommittee, chainConfig.configSub.keyProvider[0]);
                    logger.debug("account(" + newUser.owner + ") proposer(" + chainConfig.myAccountAsCommittee + "):", res);
                    userCountRes.votingNum++;
                    //投票失败，加入到失败的队列中
                    if (res == null) {
                        failedAccountPramList.push(newUser);
                    }

                    //投票次数
                    voteCount++;
                } else {
                    //已投票不处理
                    logger.info("account(" + newUser.owner + ") has been voted by " + chainConfig.myAccountAsCommittee + "");
                    userCountRes.hasVotedNum++;
                }


            } else {
                //账号已存在不处理
                logger.info("account(" + newUser.owner + ") is ready,need not vote..");
                clearFailedUser(newUser.owner);
                userCountRes.successAccountNum++;
                successAccountCacheList.push(newUser.owner);
            }

            //chainApi.contractInteract(chainConfig.u3, 'ultrainio', "voteaccount", params, myAccountAsCommittee);
            logger.info("=======voteAccount to subchain end", newUser.owner);
        }

        logger.info("voting user result", userCountRes);
    }

    logger.info("sync user end");
}

/**
 * 同步块头
 * @returns {Promise<void>}
 */
async function syncBlock() {

    if (monitor.isDeploying()) {
        logger.info("monitor.isDeploying()",monitor.isDeploying());
    } else {
        logger.info("monitor.isDeploying()",monitor.isDeploying());
    }

    logger.info("sync block start");

    //一次最大块数
    var blockSyncMaxNum = chainConfig.getLocalConfigInfo("blockSyncMaxNum",10);

    if (syncChainData == true && isMainChain() == false) {
        chainConfig.u3Sub.getChainInfo(async (error, info) => {
            if (error) {
                logger.error(error);
                return;
            }

            var traceMode = true;
            //获取本地最新的块头，获取服务端最新的块头
            var subBlockNumMax = info.head_block_num;
            let result = await chainConfig.u3Sub.getBlockInfo((subBlockNumMax).toString());

            //判断是否要上传块头
            if (blockUtil.needPushBlock(result,chainConfig.myAccountAsCommittee,chainConfig.configFileData.local.syncBlockRatio) == false) {
                logger.info("finish sync block..");
                return;
            }

            logger.info("start to push block..");
            let blockNum = 0;
            let subchainBlockNumResult = await chainApi.getSubchainBlockNum(chainConfig.config,chainConfig.localChainName);
            logger.info("mainchain block num:",subchainBlockNumResult);

            let confirmed_block = subchainBlockNumResult.confirmed_block;
            let forks = subchainBlockNumResult.forks;
            let findFlag = false;
            if (utils.isNullList(forks) == false) {
                for (let i = 0; i < forks.length; i++) {
                    let fork = forks[i];
                    let block_id = fork.block_id;
                    logger.info("fork:",fork);
                    let localBlockId = 0;
                    try {
                        let result = await chainConfig.u3Sub.getBlockInfo(fork.number.toString());
                        logger.debug("block info",result);
                        localBlockId = result.id;
                    } catch (e) {
                        logger.error("get block("+fork.number+") error,",e);
                    }

                    if (block_id == localBlockId) {
                        findFlag = true;
                        logger.info("block id("+block_id+") == local block id("+localBlockId+"),match");
                        blockNum = fork.number;
                        break;
                    } else {
                        logger.info("block id("+block_id+") != local block id("+localBlockId+"),mot match");
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
                let result = await chainConfig.u3Sub.getBlockInfo((i).toString());
                logger.debug("block " + i + ": (proposer:", result.proposer + ")");
                logger.debug("header_extensions:",result.header_extensions);
                var extensions = [];
                if (result.header_extensions.length > 0 ) {
                    result.header_extensions.forEach(function (item,index) {
                        extensions.push({"type":item[0],"data":item[1]})
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

                logger.debug("block params:",params);
                logger.info("pushing block to head (chain_name :" + chainConfig.localChainName + " count :" + results.length + ")");
                await chainApi.contractInteract(chainConfig.config, contractConstants.ULTRAINIO, "acceptheader", params, chainConfig.myAccountAsCommittee, chainConfig.config.keyProvider[0]);
            }

            //同步主链块头
            await syncMasterBlock();
            return;

        });

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
        logger.info("monitor.isDeploying()",monitor.isDeploying());
    } else {
        logger.info("monitor.isDeploying()",monitor.isDeploying());
    }

    if (isMainChain()==true) {
        logger.error("I am in masterchain ,need not sync master chain block");
        return;
    }

    logger.info("sync master block start");

    //一次最大块数
    var blockSyncMaxNum = chainConfig.getLocalConfigInfo("blockSyncMaxNum",10);

    if (syncChainData == true) {
        chainConfig.u3.getChainInfo(async (error, info) => {
            if (error) {
                logger.error(utils.logNetworkError(error));
                return;
            }

            //获取主链现在最大的块头
            let masterBlockNumMax = info.head_block_num;

            logger.info("start to push master block..");
            let blockNum = 0;
            let subchainBlockNumResult = await chainApi.getMasterBlockNum(chainConfig.configSub);
            logger.info("subchain max block num:",subchainBlockNumResult);

            let confirmed_block = subchainBlockNumResult.confirmed_block;
            let forks = subchainBlockNumResult.forks;
            let findFlag = false;
            if (utils.isNullList(forks) == false) {
                for (let i = 0; i < forks.length; i++) {
                    let fork = forks[i];
                    let block_id = fork.block_id;
                    logger.info("master fork:",fork);
                    let localBlockId = 0;
                    try {
                        let result = await chainConfig.u3.getBlockInfo(fork.number.toString());
                        logger.debug("block info",result);
                        localBlockId = result.id;
                    } catch (e) {
                        logger.error("get master block("+fork.number+") error,",e);
                    }

                    if (block_id == localBlockId) {
                        findFlag = true;
                        logger.info("master block id("+block_id+") == local master block id("+localBlockId+"),match");
                        blockNum = fork.number;
                        break;
                    } else {
                        logger.error("master block id("+block_id+") != local master block id("+localBlockId+"),mot match");
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
                let result = await chainConfig.u3.getBlockInfo((i).toString());
                logger.debug("master block " + i + ": (proposer:", result.proposer + ")");
                logger.debug("master block header_extensions:",result.header_extensions);
                var extensions = [];
                if (result.header_extensions.length > 0 ) {
                    result.header_extensions.forEach(function (item,index) {
                        extensions.push({"type":item[0],"data":item[1]})
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

                logger.debug("master block params:",params);
                logger.info("pushing master block to subchain ( count :" + results.length + ")");
                await chainApi.contractInteract(chainConfig.configSub, contractConstants.ULTRAINIO, "acceptmaster", params, chainConfig.myAccountAsCommittee, chainConfig.config.keyProvider[0]);
            }

        });

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
    //获取本地prodcuers信息
    let producerList = await chainApi.getProducerLists(chainConfig.configSub.httpEndpoint);
    logger.info("subchain producers: ", producerList);
    if (utils.isNotNull(producerList) && producerList.length > 0) {
        localProducers = producerList;
    } else {
        logger.error("get subchain producers is null,sync committee end",producerList);
        return;
    }

    //主链只需要查委员会，不需要投票
    if (isMainChain()) {
        return;
    }

    let remoteProducers = await chainApi.getSubchainCommittee(chainConfig.config,chainConfig.localChainName);
    logger.info("subchain commitee from mainchain: ", remoteProducers);

    if (utils.isNullList(remoteProducers)) {
        logger.error("get subchain commitee from mainchain is null,sync committee end",remoteProducers);
        return;
    }

    //有变化的成员列表（包括删除/新增的）
    var seed = committeeUtil.genSeedByChainId(chainConfig.configSub.chainId);
    logger.info("commit seed :" +seed);
    var changeMembers = committeeUtil.genChangeMembers(producerList, remoteProducers,seed);
    logger.info("commite changeMembers:", changeMembers);

    if (committeeUtil.isValidChangeMembers(changeMembers)) {

        for (var i = 0; i < changeMembers.length; i++) {

            //每轮只做第一个
            if (i >=1) {
                break;
            }

            let committeeUser = changeMembers[i].account;
            let params = [];
            params.push(changeMembers[i]);
            //判断需要投票的委员会的账号是否已在子链上
            if (utils.isNull(await chainApi.getAccount(chainConfig.configSub.httpEndpoint, committeeUser))) {
                logger.info("account(" + committeeUser + ") is not exist in subchain,should add him first");
                let account = await chainApi.getAccount(chainConfig.config.httpEndpoint, committeeUser);
                logger.info("account info:",account);
                //查询是否已经投过票
                let tableData = await chainApi.getTableInfo(chainConfig.configSub, contractConstants.ULTRAINIO, contractConstants.ULTRAINIO, tableConstants.PENDING_ACCOUNT, null, committeeUser, null, null);
                logger.debug(tableData)
                if (voteUtil.findVoteRecord(tableData, chainConfig.myAccountAsCommittee, committeeUser) == true) {
                    logger.info("account(" + committeeUser + ") is not exist in subchain,has voted him to user");
                } else {
                    logger.info("account(" + committeeUser + ") is not exist in subchain,has not voted him to user,start to vote");

                    const params = {
                        proposer: chainConfig.myAccountAsCommittee,
                        proposeaccount: [{
                            account: committeeUser,
                            owner_key: chainUtil.getOwnerPkByAccount(account,"owner"),
                            active_key: chainUtil.getOwnerPkByAccount(account,"active"),
                            location: constants.chainNameConstants.MAIN_CHAIN_NAME,
                            approve_num: 0,
                            updateable: 1 //该账号后续部署合约能否被更新
                        }]
                    };

                    logger.info("vote account param :",params);

                    let res = await chainApi.contractInteract(chainConfig.configSub, contractConstants.ULTRAINIO, actionConstants.VOTE_ACCOUNT, params, chainConfig.myAccountAsCommittee, chainConfig.configSub.keyProvider[0]);
                }
            } else {
                logger.info("account(" + committeeUser + ") is ready in subchain,need vote him to committee");
                //判断是否已给他投过票
                let tableData = await chainApi.getTableInfo(chainConfig.configSub, contractConstants.ULTRAINIO, contractConstants.ULTRAINIO, tableConstants.PENDING_MINER, null, committeeUser, null, null);
                if (voteUtil.findVoteCommitee(tableData, chainConfig.myAccountAsCommittee, committeeUser,changeMembers[i].adddel_miner) == false) {
                    logger.info("account(" + chainConfig.myAccountAsCommittee + ") has not voted account(" + committeeUser + ")  to committee, start to vote..");
                    try {
                        logger.info("vote commitee params:", params);
                        chainConfig.u3Sub.contract(contractConstants.ULTRAINIO).then(actions => {
                            actions.votecommittee(chainConfig.myAccountAsCommittee, params).then((unsigned_transaction) => {
                                chainConfig.u3Sub.sign(unsigned_transaction, /*mySkAsCommittee*/chainConfig.config.keyProvider[0], chainConfig.config.chainId).then((signature) => {
                                    if (signature) {
                                        let signedTransaction = Object.assign({}, unsigned_transaction.transaction, {signatures: [signature]});
                                        logger.debug(signedTransaction);
                                        chainConfig.u3Sub.pushTx(signedTransaction).then((processedTransaction) => {
                                            logger.debug(processedTransaction);
                                            // assert.equal(processedTransaction.transaction_id, unsigned_transaction.transaction_id);
                                        });
                                    }
                                })
                            })
                        });
                    } catch (e) {
                        logger.error("u3 push tx error...", e)
                        //logger.error("account(" + chainConfig.myAccountAsCommittee + ") has voted account(" + committeeUser + ")  to committee error",e);
                    }
                } else {
                    //已投过票
                    logger.info("account(" + chainConfig.myAccountAsCommittee + ") has voted account(" + committeeUser + ")  to committee, need not vote..");
                }
            }
        }
        //调用votecommittee来投票
        logger.info("syncCommitee end");
    } } catch (e) {
        logger.error("sync committee error:",e);
    }
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
        //检查主链seed是否有变化
        let mainSeedList = await chainApi.getChainHttpList(chainNameConstants.MAIN_CHAIN_NAME,chainConfig);
        if (mainSeedList.length > 0 && mainSeedList.length != chainConfig.config.seedHttpList.length) {
            logger.error("mainSeedList.length("+mainSeedList.length+") != config.seedHttpList("+chainConfig.config.seedHttpList.length+"),need update main seedList:",mainSeedList);
            chainConfig.config.seedHttpList = mainSeedList;
        } else {
            logger.info("mainSeedList.length("+mainSeedList.length+") == config.seedHttpList("+chainConfig.config.seedHttpList.length+"),need not update main seedList:",chainConfig.config.seedHttpList);
        }

        //检查子链seed是否有变化
        let subSeedList = await chainApi.getChainHttpList(chainConfig.localChainName,chainConfig);
        if (subSeedList.length > 0 && subSeedList.length != chainConfig.configSub.seedHttpList.length) {
            logger.error("subSeedList.length("+subSeedList.length+") != configSub.seedHttpList("+chainConfig.configSub.seedHttpList.length+"),need update subchain seedList:",subSeedList);
            chainConfig.configSub.seedHttpList = subSeedList;
        } else {
            logger.info("subSeedList.length("+subSeedList.length+") == configSub.seedHttpList("+chainConfig.configSub.seedHttpList.length+"),need not update sub seedList:",chainConfig.configSub.seedHttpList);
        }

        //定期更新configsub
        await chainApi.checkSubchainSeed(chainConfig);

        //定期更新config
        await chainApi.checkMainchainSeed(chainConfig);

        //同步链名称（子链id,链名称等）
        let chainName = null;
        let chainId = null;
        let genesisTime = null;
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
        }

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

        //同步委员会
        await syncCommitee();

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
 * 检查nod是否还存活
 * @returns {Promise<void>}
 */
async function checkNodAlive() {

    //配置文件和monitor配置同时关闭才生效
    if (chainConfig.configFileData.local.enableRestart == false && monitor.needCheckNod() == false) {
        logger.error("local config enable restart == false && monitor enable restart == false, need not check nod alive");
        return;
    }

    logger.info("local config enable restart == true || monitor enable restart == true,need check nod alive");

    //如果不在进行链切换且本地访问不到本地链信息，需要重启下
    if (syncChainChanging == false) {
        logger.info("checking nod is alive ....");
        let rsdata = await NodUltrain.checkAlive();

        logger.debug("check alive data:", rsdata);

        if (utils.isNull(rsdata)) {
            if (nodFailedTimes >= getmaxNodFailedTimes()) {
                nodFailedTimes = 0;
                logger.info("nod is not runing ,need restart it..");
                await restartNod();
                logger.info("nod restart end..");
            } else {
                nodFailedTimes ++;
                logger.info("nod is not alive,count("+nodFailedTimes+")");
                NodUltrain.getNewestLog(chainConfig.getLocalConfigInfo("nodLogPath","/root/log"),function (log) {
                    nodLogData = log;
                    if (utils.isNotNull(nodLogData)) {
                        let l = nodLogData.length;
                        logger.info("get nod log data:",nodLogData.substring(l-100));
                    }

                });
            }
        } else {
            nodLogData = "";
        }
    }
}

/**
 * 清除缓存信息
 */
function clearCacheData() {
    successAccountCacheList = [];
    failedAccountPramList = [];
    WorldState.status = null;
    nodFailedTimes = 0;
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
        let result = await NodUltrain.stop(120000);
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
            logMsg = utils.addLogStr(logMsgm,"seed ip info is null");
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
            result = await WorldState.start(chainConfig.chainName, seedIpInfo, 120000, chainConfig.configFileData.local.wsspath,chainConfig.localTest);
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

                result = await WorldState.pollingkWSState(1000, 300000);
                if (result == false) {
                    logger.error("require ws error："+wssinfo);
                    logMsg = utils.addLogStr(logMsg,"require ws error");
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
        logger.info("get chainid(" + chainConfig.chainName + ")'s", subchainMonitorService);
        result = await NodUltrain.updateConfig(seedIpInfo, subchainEndPoint, chainConfig.genesisTime, subchainMonitorService,chainPeerInfo,chainConfig.chainName);
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
        result = await NodUltrain.start(120000, chainConfig.configFileData.local.nodpath,wssinfo,chainConfig.localTest);
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
        param.sign = monitor.generateSign(param.time,monitor.generateSignParamWithStatus(param));
        await chainApi.addSwitchLog(monitor.getMonitorUrl(),param);
    } catch (e) {

        loggerChainChanging.info("fail to switch chain...", e);
        //结束设置结束flag
        syncChainChanging = false;
        monitor.enableDeploy();
        param.endTime = new Date().getTime();
        param.status = 0;
        param.result = "error,"+e.toString();
        param.sign = monitor.generateSign(param.time,monitor.generateSignParamWithStatus(param));
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

    //标志位设置
    syncChainChanging = true;
    monitor.disableDeploy();


    logger.info("start to restart nod");
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
                result = await WorldState.start(chainConfig.localChainName, seedIpInfo, 120000, chainConfig.configFileData.local.wsspath, chainConfig.localTest);
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

                    result = await WorldState.pollingkWSState(1000, 300000);
                    if (result == false) {
                        logger.error("require ws error：" + wssinfo);
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
        await NodUltrain.stop(120000);
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
        result = await NodUltrain.start(120000, chainConfig.configFileData.local.nodpath, wssinfo, chainConfig.localTest);
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
             * 从公告蓝上获取最新的资源
             */
            let newestChangeList = await chainApi.getSubchainResource(chainConfig.localChainName,chainConfig);
            logger.info("newestChangeList:",newestChangeList.length)
            if (utils.isNullList(newestChangeList) == false && newestChangeList.length > 0) {
                for (var i = 0; i < newestChangeList.length; i++) {
                    let resObj = newestChangeList[i];
                    logger.debug("resobj:",resObj);
                    let localtableObj = await chainApi.getTableInfo(chainConfig.configSub, contractConstants.ULTRAINIO, chainNameConstants.MAIN_CHAIN_NAME, tableConstants.RESOURCE_LEASE, null, resObj.owner, null, null);
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

            //获取已透过的所有结果
            for (var i = 0; i < changeList.length; i++) {

                if (voteCount > getMaxVoteCountOneRound()) {
                    logger.error("vote count >=("+getMaxVoteCountOneRound()+"),stop res sync.");
                    break;
                }

                var changeResObj = changeList[i];
                logger.debug("change res:", changeResObj);

                let voteResList = await chainApi.getTableInfo(chainConfig.configSub, contractConstants.ULTRAINIO, contractConstants.ULTRAINIO, tableConstants.PENDING_RES, null, changeResObj.owner, null, null);
                logger.debug("voteResList:", voteResList);

                if (voteUtil.findVoteRes(voteResList, chainConfig.myAccountAsCommittee, changeResObj, chainConfig)) {
                    logger.info(chainConfig.myAccountAsCommittee + " has voted " + changeResObj.owner + "(resource:" + changeResObj.lease_num + ")");
                } else {
                    logger.info(chainConfig.myAccountAsCommittee + " has not voted " + changeResObj.owner + "(resource:" + changeResObj.lease_num + "), start vote...");

                    const params = {
                        proposer: chainConfig.myAccountAsCommittee,
                        proposeresource: [{
                            account: changeResObj.owner,
                            lease_num: changeResObj.lease_num,
                            block_height_interval: chainUtil.calcSubchainIntevalBlockHeight(changeResObj.start_block_height,changeResObj.end_block_height,chainConfig.mainChainBlockDuration,chainConfig.subChainBlockDuration),
                            location: constants.chainNameConstants.MAIN_CHAIN_NAME,
                            approve_num: 0
                        }]
                    };

                    console.info("vote resource params:", params);
                    let res = await chainApi.contractInteract(chainConfig.configSub, contractConstants.ULTRAINIO, actionConstants.VOTE_RESOURCE_LEASE, params, chainConfig.myAccountAsCommittee, chainConfig.configSub.keyProvider[0]);
                    logger.debug(chainConfig.myAccountAsCommittee + "  vote " + changeResObj.owner + "(resource:" + changeResObj.lease_num + ") result:", res);

                    voteCount++;

                }


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

    if (chainConfig.configFileData.local.worldstate == false) {
        logger.info("syncWorldState is disabled");
        return;
    }
    logger.info("syncWorldState start");

    if (syncChainData == true) {
        try {
            //同步状态
            await WorldState.syncStatus();
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
                    logger.debug("mainChainData:",mainChainData);
                    let worldstatedata = voteUtil.getMaxValidWorldState(mainChainData.rows);
                    if (worldstatedata != null) {
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

        } catch (e) {
            logger.error("syncWorldState error:", e);
        }
    } else {
        logger.info("syncWorldState not need:",syncChainData);
    }

    logger.info("syncWorldState end");
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
}
