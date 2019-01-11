const {U3} = require('u3.js');
var logger = require("../config/logConfig").getLogger("Chain");
var chainConfig = require("./chainConfig")
var chainApi = require("./chainApi")
var timeConstats = require("../common/constant/constants").timeConstats
var chainNameConstants = require("../common/constant/constants").chainNameConstants
var contractConstants = require("../common/constant/constants").contractConstants
var sleep = require("sleep")
var utils = require("../common/util/utils")
var committeeUtil = require("./util/committeeUtil");
var blockUtil = require("./util/blockUtil");


/**
 * 同步子链内数据标志（当该字段为false时，不向主链同步块，资源，用户等信息
 * 1. 当自己是主链是为false
 * 2. 当自己是子链，但不是委员会成员是为false
 * @type {boolean}
 */
var syncChainData = false;


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
        let userBulletinList = await getUserBulletin(chainConfig.u3, chainConfig.chain_name);
        logger.debug(userBulletinList);
        for (var i in userBulletinList) {
            var newUser = userBulletinList[i];

            const params = {
                proposer: chainConfig.myAccountAsCommittee,
                proposeaccount: [{
                    account: newUser.owner,
                    owner_key: newUser.owner_pk,
                    active_key: newUser.active_pk,
                    location: 0
                }]
            };
            logger.debug("=======voteAccount to subchain:", newUser);
            chainApi.contractInteract(chainConfig.u3, 'ultrainio', "voteaccount", params, myAccountAsCommittee);
            logger.debug("=======voteAccount to subchain end", newUser);
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
    if (syncChainData == true) {
        chainConfig.u3Sub.getChainInfo(async (error, info) => {
            if (error) {
                logger.error(error);
                return;
            }

            var subBlockNumMax = info.head_block_num;
            logger.debug("head block num=", subBlockNumMax);
            let blockNum = await chainConfig.u3.getSubchainBlockNum({"chain_name": chainConfig.chainName.toString()});
            logger.debug("u3.getSubchainBlockNum  blockNum=" + blockNum);

            //初始化block Num
            let blockNumInt = parseInt(blockNum, 10) + 1;
            if (subBlockNumMax - blockNumInt >= 10) {
                subBlockNumMax = blockNumInt + 10;
            }

            let results = [];
            for (var i = blockNumInt; i < subBlockNumMax; i++) {
                let result = await chainConfig.u3Sub.getBlockInfo((i).toString());
                logger.debug("result=", result);

                //选取部分节点进行上报
                if (blockUtil.needPushBlock(result,chainConfig.myAccountAsCommittee)) {
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
                    });
                }

            }

            /**
             * 块头上传
             */
            if (results) {
                const params = {
                    chain_name: parseInt(chainConfig.chainName, 10),
                    headers: results
                };
                chainApi.contractInteract(chainConfig.u3, contractConstants.ULTRAINIO, "acceptheader", params, chainConfig.myAccountAsCommittee);
            }

        });


    } else {
        logger.error("sync block is not needed")
    }

    logger.info("sync block finish");


}

/**
 * 同步委员会
 * @returns {Promise<void>}
 */
async function syncCommitee() {
    logger.info("syncCommitee start");
    //获取本地prodcuers信息
    let producerList = await chainApi.getProducerLists(chainConfig.configSub);
    logger.debug("subchain producers: " + producerList);
    if (utils.isNotNull(producerList)) {
        localProducers = producerList;
    }

    let remoteProducers = await chainConfig.u3.getSubchainCommittee({"chain_name": chainConfig.chainName.toString()});
    logger.debug("subchain commitee from mainchain: " + remoteProducers);

    //有变化的成员列表（包括删除/新增的）
    var changeMembers = committeeUtil.genChangeMembers(producerList, remoteProducers);
    if (committeeUtil.isValidChangeMembers(changeMembers)) {
        //调用votecommittee来投票
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
        }
    }


}

/**
 * 同步链信息
 * @returns {Promise<void>}
 */
async function syncChainInfo() {
    try {
        //同步链名称（主子链id等）
        let chainName = await chainApi.getChainName(chainConfig.u3, chainConfig.myAccountAsCommittee);
        logger.debug("get chain name is:" + chainName);
        if (utils.isNotNull(chainName)) {
            if (chainConfig.chainName != chainName) {
                chainConfig.chainName = chainName;
            }
        }

        //如果是主链，啥都不操作
        if (isMainChain()) {
            logger.info("run in main chain, need not work");
            return;
        }

        //同步委员会
        await syncCommitee();

        //检查自己是否不在委员会里面
        if (!committeeUtil.isStayInCommittee(localProducers, chainConfig.myAccountAsCommittee)) {
            syncChainData = false;
            logger.info("not in subchain committee")
        } else {
            syncChainData = true;
            logger.info("still in subchain committee")
        }

    } catch (e) {
        logger.error("sync chain error:", e);
    }


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
