const {U3} = require('u3.js');
var logger = require("../config/logConfig").getLogger("Chain");
var chainConfig = require("./chainConfig")
var chainApi = require("./chainApi")
var timeConstats = require("../common/constant/constants").timeConstats
var chainNameConstants = require("../common/constant/constants").chainNameConstants
var contractConstants = require("../common/constant/constants").contractConstants
var tableConstants = require("../common/constant/constants").tableConstants
var scopeConstants = require("../common/constant/constants").scopeConstants
var actionConstants = require("../common/constant/constants").actionConstants
var sleep = require("sleep")
var utils = require("../common/util/utils")
var committeeUtil = require("./util/committeeUtil");
var blockUtil = require("./util/blockUtil");
var voteUtil = require("./util/voteUtil");


/**
 * 同步子链内数据标志（当该字段为false时，不向主链同步块，资源，用户等信息
 * 1. 当自己是主链是为false
 * 2. 当自己是子链，但不是委员会成员是为false
 * @type {boolean}
 */
var syncChainData = true;


//本地委员会成员列表
var localProducers = []



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
        //         owner: "user.313",
        //         owner_pk: "UTR7FLuM19CdCppghCjeswxPWAQxQj2LZnrKsCNwoXmi1GBwwrHUU",
        //         active_pk: "UTR7FLuM19CdCppghCjeswxPWAQxQj2LZnrKsCNwoXmi1GBwwrHUU",
        //         issue_date: '2019-01-11T03:33:10.000'
        //     }];
        // }

        logger.info("user userBulletinList:", userBulletinList);

        for (var i in userBulletinList) {
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
            //检查子链是否已经有改账户
            if (utils.isNull(await chainApi.getAccount(chainConfig.configSub, newUser.owner))) {
                logger.info("account(" + newUser.owner + ") is not ready,need vote..");

                //查询是否已经投过票
                let tableData = await chainApi.getTableAllData(chainConfig.configSub, contractConstants.ULTRAINIO, contractConstants.ULTRAINIO, tableConstants.PENDING_ACCOUNT);
                logger.debug(tableData)
                if (voteUtil.findVoteRecord(tableData, chainConfig.myAccountAsCommittee, newUser.owner) == false) {
                    //未找到投过票的记录，需要投票
                    logger.info("account(" + newUser.owner + ") has not been voted by " + chainConfig.myAccountAsCommittee + ", start voting....");

                    //发起投票合约action
                    //logger.debug("vote params ",params);
                    let res = await chainApi.contractInteract(chainConfig.configSub, contractConstants.ULTRAINIO, actionConstants.VOTE_ACCOUNT, params, chainConfig.myAccountAsCommittee, chainConfig.configSub.keyProvider[0]);
                    logger.debug("account(" + newUser.owner + ") proposer(" + chainConfig.myAccountAsCommittee + "):", res);
                } else {
                    //已投票不处理
                    logger.info("account(" + newUser.owner + ") has been voted by " + chainConfig.myAccountAsCommittee + "");
                }


            } else {
                //账号已存在不处理
                logger.info("account(" + newUser + ") is ready,need not vote..");
            }

            //chainApi.contractInteract(chainConfig.u3, 'ultrainio', "voteaccount", params, myAccountAsCommittee);
            logger.info("=======voteAccount to subchain end", newUser.owner);
        }
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
    var blockTraceModeBlockNum=3;

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
            logger.info("mainchain(subchain:"+chainConfig.chainName+") max blockNum =" + blockNum);

            //初始化block Num
            let blockNumInt = parseInt(blockNum, 10) + 1;
            var traceBlcokCount = subBlockNumMax - blockNumInt;
            logger.debug("trace block num count =", traceBlcokCount);
            if (traceBlcokCount > blockTraceModeBlockNum) {
                logger.info("traceBlcokCount > "+blockTraceModeBlockNum+" trace mode is enabled:");
            } else {
                logger.info("traceBlcokCount <= "+blockTraceModeBlockNum+" trace mode is disabled:");
                traceMode = false;
            }
            if (subBlockNumMax - blockNumInt >= blockSyncMaxNum) {
                subBlockNumMax = blockNumInt + blockSyncMaxNum;
            }

            logger.info("need upload block range ["+blockNumInt+" -> " + subBlockNumMax-1 +"]");
            let results = [];
            let blockListStr = "(";
            for (var i = blockNumInt; i < subBlockNumMax; i++) {
                let result = await chainConfig.u3Sub.getBlockInfo((i).toString());
                logger.debug("block "+i+": (proposer:", result.proposer+")");

                let needpush = true;
                //非追赶模式下，选取部分节点进行上报
                if (traceMode == false) {
                    if (blockUtil.needPushBlock(result, chainConfig.myAccountAsCommittee)) {
                        needpush = true;
                    } else  {
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
                    logger.debug("add push array(block num ："+i+")");
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
                    blockListStr += i+",";
                } else {
                    break;
                }

            }
            blockListStr+=")";
            logger.info("local uncommit blocklist :",blockListStr);

            /**
             * 块头上传
             */
            if (results) {
                const params = {
                    chain_name: parseInt(chainConfig.chainName, 10),
                    headers: results
                };
                logger.info("pushing block to head (chain_name :"+chainConfig.chainName +" count :"+results.length+")");
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
    logger.debug("subchain producers: ", producerList);
    if (utils.isNotNull(producerList)) {
        localProducers = producerList;
    }

    let remoteProducers = await chainConfig.u3.getSubchainCommittee({"chain_name": chainConfig.chainName.toString()});
    logger.debug("subchain commitee from mainchain: ", remoteProducers);
    //mock一个producers
    // remoteProducers.push({
    //     owner: 'user.311',
    //     miner_pk:
    //         '8f6e3b3336276138023617f0ae0e6fd0c37d27aa9995f9803fe78df4941dd3ec'
    // });

    //有变化的成员列表（包括删除/新增的）
    var changeMembers = committeeUtil.genChangeMembers(producerList, remoteProducers);
    logger.debug("commite changeMembers:", changeMembers);

    if (committeeUtil.isValidChangeMembers(changeMembers)) {
        let committeeUser = changeMembers[0].account;
        //判断需要投票的委员会的账号是否已在子链上
        if (utils.isNull(await chainApi.getAccount(chainConfig.configSub, committeeUser))) {
            logger.debug("account(" + committeeUser + ") is not exist in subchain,need not vote him to committee..");
        } else {
            logger.debug("account(" + committeeUser + ") is ready in subchain,need vote him to committee");
            //判断是否已给他投过票
            let tableData = await chainApi.getTableAllData(chainConfig.configSub, contractConstants.ULTRAINIO, contractConstants.ULTRAINIO, tableConstants.PENDING_MINER);
            if (voteUtil.findVoteCommitee(tableData, chainConfig.myAccountAsCommittee, committeeUser) == false) {
                logger.debug("account(" + chainConfig.myAccountAsCommittee + ") has not voted account(" + committeeUser + ")  to committee, start to vote..");
                try {
                    chainConfig.u3Sub.contract(contractConstants.ULTRAINIO).then(actions => {
                        actions.votecommittee(chainConfig.myAccountAsCommittee, changeMembers).then((unsigned_transaction) => {
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
        //同步链名称（主子链id等）
        let chainName = await chainApi.getChainName(chainConfig.u3, chainConfig.myAccountAsCommittee);
        logger.info("user("+chainConfig.myAccountAsCommittee+") belongs to chain name :" + chainName);
        if (utils.isNotNull(chainName)) {
            if (chainConfig.chainName != chainName) {
                chainConfig.chainName = chainName;
            }
        }

        //如果是主链，啥都不操作
        if (isMainChain()) {
            logger.error(chainConfig.myAccountAsCommittee+" runing in main chain, need not work");
            return;
        }

        //同步委员会
        await syncCommitee();

        //检查自己是否不在委员会里面
        if (!committeeUtil.isStayInCommittee(localProducers, chainConfig.myAccountAsCommittee)) {
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
 * 同步世界状态
 * @returns {Promise<void>}
 */
async function syncWorldStateStatus() {

}

/**
 * 同步资源
 * @returns {Promise<void>}
 */
async function syncResource() {
    logger.info("syncResource start");
    if (syncChainData == true) {
        //获取子链上所有资源的信息
        let subChainResourceList = await chainApi.getTableAllData(chainConfig.configSub, contractConstants.ULTRAINIO, scopeConstants.SCOPE_MAIN_CHAIN, tableConstants.RESOURCE_LEASE);
        logger.debug("subChainResourceList:", subChainResourceList);

        //获取主链上所有资源的信息
        let mainChainResourceList = await chainApi.getTableAllData(chainConfig.config, contractConstants.ULTRAINIO, chainConfig.chainName, tableConstants.RESOURCE_LEASE);
        logger.debug("mainChainResourceList:", mainChainResourceList);

        //对比两张表，获取更新的信息
        let changeList = await voteUtil.genVoteResList(subChainResourceList, mainChainResourceList, chainConfig);
        logger.info("change resList:", changeList);

        //获取主链上所有资源的信息
        if (changeList.length > 0) {

            //获取已透过的所有结果
            let voteResList = await chainApi.getTableAllData(chainConfig.configSub, contractConstants.ULTRAINIO, contractConstants.ULTRAINIO, tableConstants.PENDING_RES);
            logger.debug("voteResList:", voteResList);
            for (var i = 0; i < changeList.length; i++) {
                var changeResObj = changeList[i];
                logger.debug("change res:", changeResObj);

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


module.exports = {
    isMainChain,
    syncBlock,
    syncChainInfo,
    syncUser,
    syncResource

}