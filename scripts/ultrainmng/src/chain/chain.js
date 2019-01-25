const {U3} = require('u3.js');
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
var syncChainData = true;

//链切换flag-表示正在做链切换
var syncChainChanging = false;


//本地委员会成员列表
var localProducers = []


//存储用户同步的时候已成功的账户信息
var successAccountCacheList = [];

/**
 * 同步新用户和权限到子链
 * @returns {Promise<void>}
 */
async function syncUser() {

    logger.info("sync user start");
    if (syncChainData == true) {
        //获取新增用户bulletin-并发送投票到子链
        let userBulletinList = await getUserBulletin(chainConfig.u3, chainConfig.chainName);
        //mock data
        // if (userBulletinList.length == 0) {
        //     userBulletinList = [{
        //         owner: "user.314",
        //         owner_pk: "UTR7FLuM19CdCppghCjeswxPWAQxQj2LZnrKsCNwoXmi1GBwwrHUU",
        //         active_pk: "UTR7FLuM19CdCppghCjeswxPWAQxQj2LZnrKsCNwoXmi1GBwwrHUU",
        //         issue_date: '2019-01-11T03:33:10.000'
        //     }];
        // }

        logger.info("user userBulletinList:", userBulletinList);

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
                if (voteUtil.findVoteRecord(tableData, chainConfig.myAccountAsCommittee, newUser.owner) == false) {
                    //未找到投过票的记录，需要投票
                    logger.info("account(" + newUser.owner + ") has not been voted by " + chainConfig.myAccountAsCommittee + ", start voting....");

                    //发起投票合约action
                    //logger.debug("vote params ",params);
                    let res = await chainApi.contractInteract(chainConfig.configSub, contractConstants.ULTRAINIO, actionConstants.VOTE_ACCOUNT, params, chainConfig.myAccountAsCommittee, chainConfig.configSub.keyProvider[0]);
                    logger.debug("account(" + newUser.owner + ") proposer(" + chainConfig.myAccountAsCommittee + "):", res);
                    userCountRes.votingNum++;
                } else {
                    //已投票不处理
                    logger.info("account(" + newUser.owner + ") has been voted by " + chainConfig.myAccountAsCommittee + "");
                    userCountRes.hasVotedNum++;
                }


            } else {
                //账号已存在不处理
                logger.info("account(" + newUser.owner + ") is ready,need not vote..");
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
    //主子链相差多少块进入追赶模式
    var blockTraceModeBlockNum = 3;

    if (syncChainData == true) {
        chainConfig.u3Sub.getChainInfo(async (error, info) => {
            if (error) {
                logger.error(error);
                return;
            }

            var traceMode = true;
            //获取本地最新的块头，获取服务端最新的块头
            var subBlockNumMax = info.head_block_num;
            let blockNum = await chainConfig.u3.getSubchainBlockNum({"chain_name": chainConfig.chainName.toString()});
            logger.info("subchain head block num=", subBlockNumMax);
            logger.info("mainchain(subchain:" + chainConfig.chainName + ") max blockNum =" + blockNum);

            //初始化block Num
            let blockNumInt = parseInt(blockNum, 10) + 1;
            var traceBlcokCount = subBlockNumMax - blockNumInt;
            logger.debug("trace block num count =", traceBlcokCount);
            if (traceBlcokCount > blockTraceModeBlockNum) {
                logger.info("traceBlcokCount > " + blockTraceModeBlockNum + " trace mode is enabled:");
            } else {
                logger.info("traceBlcokCount <= " + blockTraceModeBlockNum + " trace mode is disabled:");
                traceMode = false;
            }
            if (subBlockNumMax - blockNumInt >= blockSyncMaxNum) {
                subBlockNumMax = blockNumInt + blockSyncMaxNum;
            }

            logger.info("need upload block range [" + blockNumInt + " -> " + subBlockNumMax - 1 + "]");
            let results = [];
            let blockListStr = "(";
            for (var i = blockNumInt; i < subBlockNumMax; i++) {
                let result = await chainConfig.u3Sub.getBlockInfo((i).toString());
                logger.debug("block " + i + ": (proposer:", result.proposer + ")");

                let needpush = true;
                //非追赶模式下，选取部分节点进行上报
                if (traceMode == false) {
                    if (blockUtil.needPushBlock(result, chainConfig.myAccountAsCommittee)) {
                        needpush = true;
                    } else {
                        needpush = false;
                    }

                } else {
                    //追赶模式下，所有节点上报
                    needpush = true;
                }

                /**
                 * 需要上传
                 */
                if (needpush) {
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
                        "header_extensions": [],
                        //blockNum : i
                    });
                    blockListStr += i + ",";
                } else {
                    break;
                }

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
    //mock一个producers
    // remoteProducers.push({
    //     owner: 'user.311',
    //     miner_pk:
    //         '8f6e3b3336276138023617f0ae0e6fd0c37d27aa9995f9803fe78df4941dd3ec'
    // });

    //有变化的成员列表（包括删除/新增的）
    var changeMembers = committeeUtil.genChangeMembers(producerList, remoteProducers);
    logger.info("commite changeMembers:", changeMembers);


    if (committeeUtil.isValidChangeMembers(changeMembers)) {

        for (var i = 0; i<changeMembers.length;i++) {
            let committeeUser = changeMembers[i].account;
            let params = [];
            params.push(changeMembers[i]);
            //判断需要投票的委员会的账号是否已在子链上
            if (utils.isNull(await chainApi.getAccount(chainConfig.configSub, committeeUser))) {
                logger.info("account(" + committeeUser + ") is not exist in subchain,need not vote him to committee..");
            } else {
                logger.info("account(" + committeeUser + ") is ready in subchain,need vote him to committee");
                //判断是否已给他投过票
                let tableData = await chainApi.getTableInfo(chainConfig.configSub, contractConstants.ULTRAINIO, contractConstants.ULTRAINIO, tableConstants.PENDING_MINER, null, committeeUser, null, null);
                if (voteUtil.findVoteCommitee(tableData, chainConfig.myAccountAsCommittee, committeeUser) == false) {
                    logger.info("account(" + chainConfig.myAccountAsCommittee + ") has not voted account(" + committeeUser + ")  to committee, start to vote..");
                    try {
                        logger.info("vote commitee params:",params);
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
                    logger.debug("account(" + chainConfig.myAccountAsCommittee + ") has voted account(" + committeeUser + ")  to committee, need not vote..");
                }
            }
        }
        //调用votecommittee来投票
        logger.info("syncCommitee end");
    }


}


var testCount = 0;

/**
 * 同步链信息
 * @returns {Promise<void>}
 */
async function syncChainInfo() {
    try {

        testCount++;

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
        let chainInfo = await chainApi.getChainInfo(chainConfig.u3, chainConfig.myAccountAsCommittee);
        logger.info("chain info from mainchain:",chainInfo);
        if (utils.isNotNull(chainInfo)) {
            chainName = chainInfo.location;
            chainId = chainInfo.chain_id;
            genesisTime = chainUtil.formatGensisTime(chainInfo.genesis_time);
            // if (chainName == "11" || chainName == 11) {
            //     genesisTime = "2019-01-24 14:06:00";
            // }
            // if (chainName == "12" || chainName == 12) {
            //     genesisTime = "2019-01-24 14:11:00";
            // }
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

        //  logger.info("testCount:",testCount);
        // if (testCount >= 2) {
        //     var newChainName = "12";
        //     var chainObj = await chainApi.getSubchainConfig(newChainName,chainConfig);
        //     logger.info("chainObj:",chainObj);
        //     if (utils.isNotNull(chainObj)) {
        //         chainConfig.chainName = newChainName;
        //         chainConfig.chainId = chainObj.chainid;
        //         chainConfig.genesisTime = chainObj.genesisTime
        //     }
        //     logger.info("getSubchanEndPoint: ",await chainApi.getSubchanEndPoint(newChainName,chainConfig));
        //     logger.info("getChainSeedIP: ",await chainApi.getChainSeedIP(newChainName,chainConfig));
        // }

        //如果是主链，啥都不操作
        if (isMainChain()) {
            syncChainData = false;
            logger.error(chainConfig.myAccountAsCommittee + " runing in main chain, need not work");
            return;
        }

        logger.info(chainConfig.myAccountAsCommittee + " belongs to chaininfo (name:" + chainConfig.chainName + ",chain_id:" + chainConfig.chainId + " ,genesisTime:"+chainConfig.genesisTime+") from mainchain");
        logger.info("now subchain's chainid :" + chainConfig.configSub.chainId);
        var rightChain = chainConfig.isInRightChain()
        if (!rightChain) {
            syncChainData = false;
            //我已不属于这条链且我不在委员会，准备迁走
            logger.info(chainConfig.myAccountAsCommittee + "need trandfer to chain(" + chainConfig.chainName + "）,and  is not in committee, start transfer...");
            sleep.msleep(1000);
            syncChainChanging = true;

            //清除数据
            clearCacheData()
            //开始迁移
            await switchChain();
            return;
        } else {
            syncChainData = true;
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
                await NodUltrain.stop(5000);
                sleep.msleep(1000);
                await NodUltrain.start(5000,chainConfig.configFileData.local.nodpath);
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
        let result = await NodUltrain.stop(60000);
        if (result == false) {
            loggerChainChanging.info("nod is stopped");
        } else {
            loggerChainChanging.info("nod is not stopped");
        }

        //停止worldstate的程序
        if (chainConfig.configFileData.local.worldstate == true) {
            result = await WorldState.stop(60000);
            if (result) {
                logger.info("worldstate is stopped");
            } else {
                logger.info("worldstate is not stopped");
            }
        }

        //删除block和shared_memory.bin数据
        await NodUltrain.removeData();
        loggerChainChanging.info("remove block data and shared_memory.bin");
        sleep.msleep(1000);


        //清除世界状态数据
        if (chainConfig.configFileData.local.worldstate == true) {
            await WorldState.clearDB();
            loggerChainChanging.info("remove worldstate data files");
            sleep.msleep(1000);
        }

        //通过chainid拿到seedList
        var seedIpInfo = await chainApi.getChainSeedIP(chainConfig.chainName, chainConfig);
        logger.info("get chainid(" + chainConfig.chainName + ")'s seed ip info:", seedIpInfo);
        if (utils.isNull(seedIpInfo)) {
            loggerChainChanging.error("seed ip info is null");
            syncChainChanging = false;
            return;
        }


        //重启世界状态并拉块
        if (chainConfig.configFileData.local.worldstate == true) {
            logger.info("start world state");
            result = await WorldState.start(chainConfig.chainName, seedIpInfo, 60000, chainConfig.configFileData.local.wsspath);
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
                syncChainChanging = false;
                return;
            }

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
                syncChainChanging = false;
                return;
            }

            loggerChainChanging.info("polling worldstate sync status ..")

            sleep.msleep(1000);

            /**
             * 轮询检查同步世界状态情况
             */
            result = await WorldState.pollingkWSState(1000, 60000);
            if (result == false) {
                logger.info("require ws error");
            } else {
                logger.info("require ws success");
            }

            sleep.msleep(1000);

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
            result = await WorldState.pollingBlockState(1000, 60000);
            if (result == false) {
                logger.info("require block error");
            } else {
                logger.info("require block success");
            }

            sleep.msleep(1000);
        }


        //修改nod程序配置信息
        var subchainEndPoint = await chainApi.getSubchanEndPoint(chainConfig.chainName,chainConfig);
        logger.info("get chainid(" + chainConfig.chainName + ")'s seed ip info:", seedIpInfo);
        logger.info("subchainEndPoint:", subchainEndPoint);
        logger.info("genesisTime:", chainConfig.genesisTime);
        result = await NodUltrain.updateConfig(seedIpInfo, subchainEndPoint, chainConfig.genesisTime);
        if (result == true) {
            loggerChainChanging.info("update nod config file success")
            //重新加载配置文件信息
            loggerChainChanging.info("reload config files")
            await chainConfig.waitSyncConfig();
            loggerChainChanging.info("reload config files ready")
        } else {
            loggerChainChanging.error("update nod config file error")
        }


        //启动nod
        result = await NodUltrain.start(60000,chainConfig.configFileData.local.nodpath);
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
 * 同步资源
 * @returns {Promise<void>}
 */
async function syncResource() {
    logger.info("syncResource start");
    if (syncChainData == true) {
        //获取子链上所有资源的信息
        let subChainResourceList = await chainApi.getTableAllData(chainConfig.configSub, contractConstants.ULTRAINIO, scopeConstants.SCOPE_MAIN_CHAIN, tableConstants.RESOURCE_LEASE, "owner");
        logger.debug("subChainResourceList:", subChainResourceList);

        //获取主链上所有资源的信息
        let mainChainResourceList = await chainApi.getTableAllData(chainConfig.config, contractConstants.ULTRAINIO, chainConfig.chainName, tableConstants.RESOURCE_LEASE, "owner");
        logger.debug("mainChainResourceList:", mainChainResourceList);

        //对比两张表，获取更新的信息
        let changeList = await voteUtil.genVoteResList(subChainResourceList, mainChainResourceList, chainConfig);
        logger.info("change resList:", changeList);

        //获取主链上所有资源的信息
        if (changeList.length > 0) {

            //获取已透过的所有结果
            for (var i = 0; i < changeList.length; i++) {
                var changeResObj = changeList[i];
                logger.debug("change res:", changeResObj);

                let voteResList = await chainApi.getTableInfo(chainConfig.configSub, contractConstants.ULTRAINIO, contractConstants.ULTRAINIO, tableConstants.PENDING_RES, null, changeResObj.owner, null, null);
                logger.debug("voteResList:", voteResList);

                if (voteUtil.findVoteRes(voteResList, chainConfig.myAccountAsCommittee, changeResObj.owner, changeResObj.lease_num, changeResObj.end_time)) {
                    logger.info(chainConfig.myAccountAsCommittee + " has voted " + changeResObj.owner + "(resource:" + changeResObj.lease_num + ")");
                } else {
                    logger.info(chainConfig.myAccountAsCommittee + " has not voted " + changeResObj.owner + "(resource:" + changeResObj.lease_num + "), start vote...");

                    const params = {
                        proposer: chainConfig.myAccountAsCommittee,
                        proposeresource: [{
                            account: changeResObj.owner,
                            lease_num: changeResObj.lease_num,
                            end_time: changeResObj.end_time,
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

    if (chainConfig.configFileData.local.worldstate  == false) {
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
                    for (var i = mainChainData.rows.length-1; i >= 0; i--) {
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
    syncWorldState

}
