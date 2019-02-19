const {U3} = require('u3.js');
const fs = require('fs');
var logger = require("../config/logConfig").getLogger("Chain");
var loggerChainChanging = require("../config/logConfig").getLogger("ChainChanging");
var chainConfig = require("./chainConfig")
var chainApi = require("./chainApi")
var timeConstats = require("../common/constant/constants").timeConstats
var chainNameConstants = require("../common/constant/constants").chainNameConstants
var contractConstants = require("../common/constant/constants").contractConstants
var tableConstants = require("../common/constant/constants").tableConstants
var scopeConstants = require("../common/constant/constants").scopeConstants
var actionConstants = require("../common/constant/constants").actionConstants
var chainIdConstants = require("../common/constant/constants").chainIdConstants
var pathConstants = require("../common/constant/constants").pathConstants
var sleep = require("sleep")
var utils = require("../common/util/utils")
var committeeUtil = require("./util/committeeUtil");
var blockUtil = require("./util/blockUtil");
var voteUtil = require("./util/voteUtil");
var NodUltrain = require("../nodultrain/nodultrain")
var WorldState = require("../worldstate/worldstate")
var chainUtil = require("./util/chainUtil");



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
    if (syncChainData == true) {
        //获取新增用户bulletin-并发送投票到子链
        let userBulletinList = await getUserBulletin(chainConfig.u3, chainConfig.chainName);
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
            userCountRes.totalNum++;
            var newUser = userBulletinList[i];
            const params = {
                proposer: chainConfig.myAccountAsCommittee,
                proposeaccount: [{
                    account: newUser.owner,
                    owner_key: newUser.owner_pk,
                    active_key: newUser.active_pk,
                    location: 0,
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
            if (utils.isNull(hitFlag) && utils.isNull(await chainApi.getAccount(chainConfig.configSub, newUser.owner))) {
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

    logger.info("sync block start");

    //一次最大块数
    var blockSyncMaxNum = 10;

    if (syncChainData == true) {
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
            let subchainBlockNumResult = await chainConfig.u3.getSubchainBlockNum({"chain_name": chainConfig.chainName.toString()});
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
            logger.info("mainchain(subchain:" + chainConfig.chainName + ") max blockNum =" + blockNum);



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
                    chain_name: parseInt(chainConfig.chainName, 10),
                    headers: results
                };

                logger.debug("block params:",params);
                logger.info("pushing block to head (chain_name :" + chainConfig.chainName + " count :" + results.length + ")");
                await chainApi.contractInteract(chainConfig.config, contractConstants.ULTRAINIO, "acceptheader", params, chainConfig.myAccountAsCommittee, chainConfig.config.keyProvider[0]);
            }

        });

    } else {
        logger.error("sync block is not needed")
    }

    logger.info("sync block finish")

}

/**
 * 同步委员会
 * @returns {Promise<void>}
 */
async function syncCommitee() {
    logger.info("syncCommitee start");
    //获取本地prodcuers信息
    let producerList = await chainApi.getProducerLists(chainConfig.configSub);
    logger.info("subchain producers: ", producerList);
    if (utils.isNotNull(producerList)) {
        localProducers = producerList;
    }

    let remoteProducers = await chainConfig.u3.getSubchainCommittee({"chain_name": chainConfig.chainName.toString()});
    logger.info("subchain commitee from mainchain: ", remoteProducers);

    //有变化的成员列表（包括删除/新增的）
    var changeMembers = committeeUtil.genChangeMembers(producerList, remoteProducers);
    logger.info("commite changeMembers:", changeMembers);

    if (committeeUtil.isValidChangeMembers(changeMembers)) {

        for (var i = 0; i < changeMembers.length; i++) {
            let committeeUser = changeMembers[i].account;
            let params = [];
            params.push(changeMembers[i]);
            //判断需要投票的委员会的账号是否已在子链上
            if (utils.isNull(await chainApi.getAccount(chainConfig.configSub, committeeUser))) {
                logger.info("account(" + committeeUser + ") is not exist in subchain,should add him first");
                let account = await chainApi.getAccount(chainConfig.config, committeeUser);
                logger.info("account info:",account);
                logger.info("account info:",account.permissions[0].required_auth);
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
                            account: account.account_name,
                            owner_key: chainUtil.getOwnerPkByAccount(account,"owner"),
                            active_key: chainUtil.getOwnerPkByAccount(account,"active"),
                            location: 0,
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

        //同步链名称（子链id,链名称等）
        let chainName = null;
        let chainId = null;
        let genesisTime = null;
        if (utils.isNull(chainConfig.configSub.chainId)) {
            chainConfig.configSub.chainId = await chainApi.getSubChainId(chainConfig.configSub);
        }
        logger.debug("configSub.chainId=", chainConfig.configSub.chainId);
        let chainInfo = await chainApi.getChainInfo(chainConfig.u3, chainConfig.myAccountAsCommittee);
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
            syncChainData = false;
            logger.error(chainConfig.myAccountAsCommittee + " runing in main chain, need not work");
            return;
        }

        //如果是非出块节点，啥都不操作
        if (chainConfig.isNoneProducer()) {
            syncChainData = false;
            logger.error(chainConfig.myAccountAsCommittee + " runing is none-producer, need not work");
            return;
        }

        logger.info(chainConfig.myAccountAsCommittee + " belongs to chaininfo (name:" + chainConfig.chainName + ",chain_id:" + chainConfig.chainId + " ,genesisTime:" + chainConfig.genesisTime + ") from mainchain");
        logger.info("now subchain's chainid :" + chainConfig.configSub.chainId);

        //主链返回的chainname非法，说明主链返回的有问题，或者是该用户在主链不存在,不工作
        if (chainConfig.chainName == chainNameConstants.INVAILD_CHAIN_NAME) {
            syncChainData = false;
            logger.error(chainConfig.myAccountAsCommittee + " is a invalid name in main chain,need not work");
            return;
        }

        var rightChain = chainConfig.isInRightChain()
        if (!rightChain) {
            syncChainData = false;
            //我已不属于这条链，准备迁走
            logger.info(chainConfig.myAccountAsCommittee + "need trandfer to chain(" + chainConfig.chainName + "）, start transfer...");
            sleep.msleep(1000);
            syncChainChanging = true;

            //清除数据
            clearCacheData()
            //开始迁移
            await switchChain();
            return;
        } else {
            syncChainChanging = false;
        }

        //如果不在进行链切换且本地访问不到本地链信息，需要重启下
        if (syncChainChanging == false) {
            logger.info("check nod is alive ....");
            let rsdata = await NodUltrain.checkAlive();
            logger.debug("check alive data:", rsdata);
            if (utils.isNull(rsdata)) {
                logger.info("nod is not runing ,need restart it..");
                //启动nod
                syncChainChanging = true;
                syncChainData = false;
                await NodUltrain.stop(120000);
                sleep.msleep(1000);
                logger.info("clear DB data before restart it..");
                await NodUltrain.removeData();
                if (chainConfig.configFileData.local.worldstate == true) {
                    await WorldState.clearDB();
                }
                logger.info("restart data");
                await NodUltrain.start(120000, chainConfig.configFileData.local.nodpath," ",chainConfig.localTest);
                syncChainChanging = false;
                syncChainData = true;
                logger.info("nod restart end..");
                return;
            }
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


    } catch (e) {
        logger.error("sync chain error:", e);
    }

    logger.info("sync chain info and committee end");

}

/**
 * 清除缓存信息
 */
function clearCacheData() {
    successAccountCacheList = [];
    failedAccountPramList = [];
    WorldState.status = null;
}

/**
 * 链接切换
 * @returns {Promise<void>}
 */
async function switchChain() {

    loggerChainChanging.info("starting to switch chain...");
    try {

        //停止nod程序
        loggerChainChanging.info("shuting down nod...")
        let result = await NodUltrain.stop(120000);
        if (result == false) {
            loggerChainChanging.info("nod is stopped");
        } else {
            loggerChainChanging.info("nod is not stopped");
        }

        //停止worldstate的程序
        if (chainConfig.configFileData.local.worldstate == true) {
            result = await WorldState.stop(120000);
            if (result) {
                logger.info("worldstate is stopped");
            } else {
                logger.info("worldstate is not stopped");
            }
        }

        //删除block和shared_memory.bin数据
        await NodUltrain.removeData();
        loggerChainChanging.info("remove block data and shared_memory.bin");
        sleep.msleep(5000);


        //清除世界状态数据
        if (chainConfig.configFileData.local.worldstate == true) {
            await WorldState.clearDB();
            loggerChainChanging.info("remove worldstate data files");
            sleep.msleep(5000);
        }

        //通过chainid拿到seedList
        var seedIpInfo = await chainApi.getChainSeedIP(chainConfig.chainName, chainConfig);
        logger.info("get chainid(" + chainConfig.chainName + ")'s seed ip info:", seedIpInfo);
        if (utils.isNull(seedIpInfo)) {
            loggerChainChanging.error("seed ip info is null");
            syncChainChanging = false;
            return;
        }


        let wssinfo = " ";
        let wssFilePath = null;
        //重启世界状态并拉块
        if (chainConfig.configFileData.local.worldstate == true) {
            logger.info("start world state");
            result = await WorldState.start(chainConfig.chainName, seedIpInfo, 120000, chainConfig.configFileData.local.wsspath,chainConfig.localTest);
            if (result == true) {
                logger.info("start ws success");
            } else {
                logger.info("start ws error");
                // syncChainChanging = false;
                // return;
            }

            sleep.msleep(2000);

            //调用世界状态程序同步数据
            var worldstatedata = null;
            let mainChainData = await chainApi.getTableAllData(chainConfig.config, contractConstants.ULTRAINIO, chainConfig.chainName, tableConstants.WORLDSTATE_HASH, "block_num");
            if (utils.isNotNull(mainChainData) && mainChainData.rows.length > 0) {
                worldstatedata = mainChainData.rows[mainChainData.rows.length - 1];
                logger.info("get worldstate data:", worldstatedata);
            } else {
                logger.error("can not get world state file,or data is null");
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
                } else {
                    logger.info("sync worldstate request failed");
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
                } else {
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


        //修改nod程序配置信息
        var subchainEndPoint = await chainApi.getSubchanEndPoint(chainConfig.chainName, chainConfig);
        var subchainMonitorService = await chainApi.getSubchanMonitorService(chainConfig.chainName, chainConfig);
        logger.info("get chainid(" + chainConfig.chainName + ")'s seed ip info:", seedIpInfo);
        logger.info("subchainEndPoint:", subchainEndPoint);
        logger.info("genesisTime:", chainConfig.genesisTime);
        logger.info("get chainid(" + chainConfig.chainName + ")'s", subchainMonitorService);
        result = await NodUltrain.updateConfig(seedIpInfo, subchainEndPoint, chainConfig.genesisTime, subchainMonitorService);
        if (result == true) {
            loggerChainChanging.info("update nod config file success")
            //重新加载配置文件信息
            loggerChainChanging.info("reload config files")
            await chainConfig.waitSyncConfig();
            loggerChainChanging.info("reload config files ready")
        } else {
            loggerChainChanging.error("update nod config file error")
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
            loggerChainChanging.info("nod start success")
        } else {
            loggerChainChanging.error("node start error");
        }

        //等待配置信息同步完成-重新加载配置
        await chainConfig.waitSyncConfig()

        //结束设置结束flag
        syncChainChanging = false;
        loggerChainChanging.info("switching chain successfully...");
    } catch (e) {

        loggerChainChanging.info("fail to switch chain...", e);
        //结束设置结束flag
        syncChainChanging = false;
    }
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
    if (syncChainData == true) {

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
            let mainChainResourceList = await chainApi.getTableAllData(chainConfig.config, contractConstants.ULTRAINIO, chainConfig.chainName, tableConstants.RESOURCE_LEASE, "owner");
            logger.info("mainChainResourceList:", mainChainResourceList);

            //对比两张表，获取更新的信息
            changeList = await voteUtil.genVoteResList(subChainResourceList, mainChainResourceList, chainConfig);
        } else {
            logger.info("sync newest resource");
            /**
             * 从公告蓝上获取最新的资源
             */
            let newestChangeList = await chainApi.getSubchainResource(chainConfig.chainName,chainConfig);
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
                            location: 0,
                            approve_num: 0
                        }]
                    };

                    console.info("vote resource params:", params);
                    let res = await chainApi.contractInteract(chainConfig.configSub, contractConstants.ULTRAINIO, actionConstants.VOTE_RESOURCE_LEASE, params, chainConfig.myAccountAsCommittee, chainConfig.configSub.keyProvider[0]);
                    logger.debug(chainConfig.myAccountAsCommittee + "  vote " + changeResObj.owner + "(resource:" + changeResObj.lease_num + ") result:", res);

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
    return chainNameConstants.MAIN_CHAIN_NAME == chainConfig.chainName;
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
                let mainChainData = await chainApi.getTableAllData(chainConfig.config, contractConstants.ULTRAINIO, chainConfig.chainName, tableConstants.WORLDSTATE_HASH, "block_num");

                logger.debug("mainChainData:", mainChainData);
                let needUpload = true;
                if (utils.isNotNull(mainChainData) && mainChainData.rows.length > 0) {
                    logger.info("mainChainData:" + mainChainData);
                    for (var i = mainChainData.rows.length - 1; i >= 0; i--) {
                        logger.info("main chain's world state (main chain block num :" + mainChainData.rows[i].block_num + " subchain node block num :" + WorldState.status.block_height + ")");
                        if (mainChainData.rows[i].block_num >= WorldState.status.block_height) {
                            logger.info("main chain's world state is newest,need not upload:(main chain block num :" + mainChainData.rows[i].block_num + " subchain node block num :" + WorldState.status.block_height + ")");
                            needUpload = false;
                            break;
                        }


                    }

                } else {
                    logger.info("main chain's world state is null,need upload");
                    needUpload = true;
                }
                //需要上传
                if (needUpload) {
                    let params = {
                        subchain: chainConfig.chainName,
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
    syncNewestResource
}
