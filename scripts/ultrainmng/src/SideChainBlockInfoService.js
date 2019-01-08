// import {getActionsByAccount} from "u3.js/src";

const {U3} = require('u3.js');
const axios = require('axios')
const path = require('path');
const fs = require('fs');
const ini = require('ini');
const schedule = require('node-schedule');
const {createU3, format} = U3;
const utils = require("./common/util/utils")

/**
 * 全局变量定义
 * @type {string}
 */
//节点登录的委员会用户信息
var myAccountAsCommittee = "";
//用户私钥
var mySkAsCommittee = "";
//子链上所有producers的本地备份，做对比用
var jsonArray = [];
//子链名称
var chain_name = "";
//是否为本地内网环境测试
var localtest = false;

/**
 * 日志信息
 */
var logger = require("./config/logConfig");
var logUtil = require("./common/util/logUtil")

var config = {
    httpEndpoint: "",
    httpEndpoint_history: "",
    keyProvider: [], // WIF string or array of keys..

    chainId: '', // 32 byte (64 char) hex string
    // chainId: '2616bfbc21e11d60d10cb798f00893c2befba10e2338b7277bb3865d2e658f58', // 32 byte (64 char) hex string
    expireInSeconds: 60,
    broadcast: true,
    sign: true,

    debug: false,
    verbose: false,
    logger: {
        log: logUtil.log,
        error: logUtil.error,
        debug: logUtil.debug
    },
    binaryen: require('binaryen'),
    symbol: 'UGAS'

};


let configSub = {
    httpEndpoint: '',
    httpEndpoint_history: '',
    keyProvider: [], // WIF string or array of keys..

    chainId: "", // 32 byte (64 char) hex string
    // chainId: '2616bfbc21e11d60d10cb798f00893c2befba10e2338b7277bb3865d2e658f58', // 32 byte (64 char) hex string
    expireInSeconds: 60,
    broadcast: true,
    sign: true,

    debug: false,
    verbose: false,
    logger: {
        log: logUtil.log,
        error: logUtil.error,
        debug: logUtil.debug
    },
    binaryen: require('binaryen'),
    symbol: 'UGAS'

};


// let u3 = createU3(config);
var u3;
//= createU3({ ...config, sign: true, broadcast: true });
var u3Sub;// = createU3({ ...configSub, sign: true, broadcast: true });


/**
 * 读写相关的配置
 */
async function initConfig() {

    logger.debug("start to init config file");

    /**
     * 读取管家程序自己的config文件来读取
     */
    var configIniLocal = ini.parse(fs.readFileSync(path.join(__dirname, "../config.ini"), 'utf-8'));

    //读取nodultrain程序中的config.ini
    var configIniTarget = ini.parse(fs.readFileSync(configIniLocal.path, 'utf-8'));
    logger.debug('configIniTarget=', configIniTarget);

    //获取主链请求的http地址-默认使用
    const ip = await getRemoteIpAddress(configIniLocal.url);
    logger.debug('getRemoteIpAddress=', ip);
    config.httpEndpoint = `${configIniLocal.prefix}${ip}${configIniLocal.endpoint}`;

    //子链请求地址配置
    configSub.httpEndpoint = configIniLocal.subchainHttpEndpoint;

    /**
     * 获取配置中localtest配置
     */
    localtest = configIniLocal["localtest"];
    logger.debug("env: (localtest:" + localtest + ")");


    /**
     * 通过nodultrain的配置信息获取主子链的用户信息
     */
    myAccountAsCommittee = configIniTarget["my-account-as-committee"];
    mySkAsCommittee = configIniTarget["my-sk-as-committee"];
    config.keyProvider = [configIniTarget["my-sk-as-account"]];
    configSub.keyProvider = [configIniTarget["my-sk-as-account"]];

    /**
     * 如果localtest为true，表明当前是本地测试状态，更新主链url等信息
     */
    if (localtest) {
        if (utils.isNotNull(configIniLocal["mainchainHttpEndpoint"])) {
            logger.debug("local mainchain httpEndpoint is enabled :"+configIniLocal["mainchainHttpEndpoint"]);
            config.httpEndpoint = configIniLocal["mainchainHttpEndpoint"];
        }
        // config.httpEndpoint = "http://172.16.10.4:8877";
        if (utils.isNotNull(configIniLocal["chain_name"])) {
            chain_name = configIniLocal["chain_name"];
        }
        /**
         * 账号信息需要同时否不为空才进行替换
         */
        if (utils.isAllNotNull(configIniLocal["my-account-as-committee"], configIniLocal["my-sk-as-committee"], configIniLocal["my-sk-as-account"])) {
            logger.debug("local account config is enabled :"+configIniLocal["my-account-as-committee"]);
            myAccountAsCommittee = configIniLocal["my-account-as-committee"];
            mySkAsCommittee = configIniLocal["my-sk-as-committee"];
            config.keyProvider = [configIniLocal["my-sk-as-account"]];
            configSub.keyProvider = [configIniLocal["my-sk-as-account"]];
        }
    }

    try {
        //
        config.chainId = await getMainChainId();
        logger.debug("config.chainId=", config.chainId);

        configSub.chainId = await getSubChainId();
        logger.debug("configSub.chainId=", configSub.chainId);
    } catch (e) {
        logger.error("target node crashed, check main node or sub node", e)

    }

    logger.debug("finish init config file");
}


/**
 * 转账
 */
function transferToUltrainio() {
    u3.contract('utrio.token').then(actions => {
        actions.transfer('user.111', 'user.112', '1.0000 UGAS', 'transfer memo').then((unsigned_transaction) => {
            u3.sign(unsigned_transaction, config.keyProvider[0], config.chainId).then((signature) => {
                if (signature) {
                    let signedTransaction = Object.assign({}, unsigned_transaction.transaction, {signatures: [signature]});
                    logger.debug(signedTransaction);

                    u3.pushTx(signedTransaction).then((processedTransaction) => {
                        logger.debug(processedTransaction);
                        // assert.equal(processedTransaction.transaction_id, unsigned_transaction.transaction_id);
                    });
                }
            })
        })
    });
}

/**
 * 获取主网主链Chain Id
 * @returns {Promise<*>}
 */
const getMainChainId = async () => {

    var u3 = createU3({...config, sign: true, broadcast: true});
    var blockInfo = await u3.getBlockInfo("1");
    logger.debug("block info  blockInfo.action_mroot=", blockInfo.action_mroot);
    return blockInfo.action_mroot;
}

/**
 * 获取子网子链Chain Id
 * @returns {Promise<*>}
 */
const getSubChainId = async () => {

    var u3Sub = createU3({...configSub, sign: true, broadcast: true});
    var blockInfo = await u3Sub.getBlockInfo("1");
    logger.debug("block info  blockInfo.action_mroot=", blockInfo.action_mroot);
    return blockInfo.action_mroot;
}

/**
 * 根据官网数据获取主网ip地址
 * @returns {Promise<*>}
 */
async function getRemoteIpAddress(url) {
    const rs = await axios.get(url);
    return rs.data;
}

/**
 * 获取本地producer列表
 * @returns {Promise<Array>}
 */
async function getProducerLists() {
    const params = {"json": "true", "lower_bound": "0", "limit": 100};
    const rs = await axios.post(configSub.httpEndpoint + "/v1/chain/get_producers", params);
    // const rs = await axios.post("http://172.16.10.5:8899/v1/chain/get_producers", params);

    var result = [];

    var rows = rs.data.rows;

    for (var i in rows) {
        var row = rows[i];
        if (row.is_active == 1) {
            result.push({
                owner: row.owner,
                miner_pk: "",
            });
        }
    }

    logger.debug("getProducerLists result=", result);
    return result;
}

//curl  http://127.0.0.1:8888/v1/chain/get_producers   -d '{"json":"true","lower_bound":"0","limit":100}'

/**
 * 构建Committee
 * @param resultJson
 * @param jsonArray
 * @param add
 * @param result
 * @returns {*}
 */
function buildCommittee(resultJson, jsonArray, add, result) {
    /**
     *
     * [{
            "account": "user.114",
            "public_key": "b9a55c3c661abd8c539b7a7c05c8176036f87aeeb6b117a138327cfdb374cc23",
            "url": "https://user.115.com",
            "location": 0
        }, {
            "account": "user.115",
            "public_key": "8f6e3b3336276138023617f0ae0e6fd0c37d27aa9995f9803fe78df4941dd3ec",
            "url": "https://user.115.com",
            "location": 0
        }]

     * **/

    for (var i in resultJson) {

        if (resultJson[i]) {
            logger.debug("result[i]=", resultJson[i].owner);
            logger.debug("result[i]=", resultJson[i].miner_pk);

            if (jsonArray.length == 0) {
                result.push({
                    account: resultJson[i].owner,
                    public_key: resultJson[i].miner_pk,
                    url: "https://user.115.com",
                    location: 0,
                    adddel_miner: add
                });
                continue;
            }

            var isInJsonArray = false;
            for (var i2 in jsonArray) {

                if (jsonArray[i2]) {
                    if (jsonArray[i2].owner == resultJson[i].owner) {
                        isInJsonArray = true;
                        break;
                    }
                }
            }

            if (isInJsonArray) {
                continue;
            }

            result.push({
                account: resultJson[i].owner,
                public_key: resultJson[i].miner_pk,
                url: "https://user.115.com",
                location: 0,
                adddel_miner: add
            });
        }
    }

    logger.debug("result array=", result);
    logger.debug("result array=", result.length);

    return result;
}

/**
 * 调用系统合约更改子链committee
 *
 */

function invokeSystemContract(resultJson) {

    var result = [];

    result = buildCommittee(resultJson, jsonArray, 1, result);

    result = buildCommittee(jsonArray, resultJson, 0, result);

    if (result.length == 0) {
        return;
    }

    if (result.length > 1) {
        logger.error("error, Committee members is too many")
        return;
    }

    try {

        u3Sub.contract('ultrainio').then(actions => {
            actions.votecommittee(myAccountAsCommittee, result).then((unsigned_transaction) => {
                u3Sub.sign(unsigned_transaction, /*mySkAsCommittee*/config.keyProvider[0], config.chainId).then((signature) => {
                    if (signature) {
                        let signedTransaction = Object.assign({}, unsigned_transaction.transaction, {signatures: [signature]});
                        logger.debug(signedTransaction);

                        u3Sub.pushTx(signedTransaction).then((processedTransaction) => {
                            logger.debug(processedTransaction);
                            // assert.equal(processedTransaction.transaction_id, unsigned_transaction.transaction_id);
                        });
                    }
                })
            })
        });
        //列表更新
        // jsonArray = resultJson;
    } catch (e) {
        logger.error("u3 push tx error...", e)
    }

}

/**
 *
 * 把子链的header push到主网上
 *
 * 参数results
 *
 */
pushHeaderToTestnet = async (results) => {
    logger.debug("=======" + results)

    const params = {
        chain_name: parseInt(chain_name, 10),
        headers: results
    };

    contractInteract(config, 'ultrainio', "acceptheader", params, myAccountAsCommittee, config.keyProvider[0]);
}

/**
 * 根据user 获取chain name
 * @param user
 * @returns {Promise<*|number|Location|string|WorkerLocation>}
 */
async function initChainName(user) {

    let result = await u3.getProducerInfo({"owner": user});

    logger.debug(result);

    return result.location;

}

/**
 *
 * 获取用户名、用户公钥
 *
 * 参数chain_name
 *
 */
getUserBulletin = async () => {

    let result = await u3.getUserBulletin({"chain_name": parseInt(chain_name, 10)});

    return result;

}

/**
 * 新增账户
 * @param results
 * @returns {Promise<void>}
 */
async function voteAccount(results) {

    let infos = await getUserBulletin();

    logger.debug("=======voteAccount params=", results);
    logger.debug("=======voteAccount info=", infos);

    for (var i in infos) {
        var info = infos[i];

        const params = {
            proposer: myAccountAsCommittee,
            proposeaccount: [{
                account: info.owner,
                owner_key: info.owner_pk,
                active_key: info.active_pk,
                location: 0
            }]
        };

        logger.debug("=======voteAccount to subchain:", info);
        contractInteract(configSub, 'ultrainio', "voteaccount", params, myAccountAsCommittee, config.keyProvider[0]);
        logger.debug("=======voteAccount to subchain end", info);
    }

}


/**
 *
 * 定时任务调度，10s获取一次
 *
 *vo
 */
async function scheduleCronstyle() {

    logger.info("ultrainmng start to work...")

    await initConfig();


    u3 = createU3({...config, sign: true, broadcast: true});
    u3Sub = createU3({...configSub, sign: true, broadcast: true});

    chain_name = await initChainName(myAccountAsCommittee);

    schedule.scheduleJob('*/10 * * * * *', function () {
        logger.debug('scheduleCronstyle:' + new Date());

        try {
            getBlocks();

            getSubchainCommittee();

            voteAccount();

        } catch (e) {
            logger.error("throw exceptions=", e)
        }


    });
}

/**
 * 获取主网 子链block数据当前已提交的blockNum，然后去获取子链下一个blockNum
 *
 * chain_name 需要配置
 * @returns {Promise<void>}
 */
const getBlocks = async () => {
    // let resultJson = await u3.getAllBlocksHeader(1, 2, {}, {_id: -1});
    //
    // logger.debug("===============u3.resultJson.results.results[0].block result is:", resultJson.results);
    u3Sub.getChainInfo(async (error, info) => {
        if (error) {
            logger.error(error);
            return;
        }

        logger.debug("u3.getChainInfo =", info);

        var subBlockNumMax = info.head_block_num;

        logger.debug("head block num=", subBlockNumMax);

        let blockNum = await u3.getSubchainBlockNum({"chain_name": chain_name.toString()});
        logger.debug("u3.getSubchainBlockNum  blockNum=" + blockNum);

        //初始化block Num
        let blockNumInt = parseInt(blockNum, 10) + 1;

        if (subBlockNumMax - blockNumInt >= 10) {
            subBlockNumMax = blockNumInt + 10;
        }

        let results = [];
        for (var i = blockNumInt; i < subBlockNumMax; i++) {
            let result = await u3Sub.getBlockInfo((i).toString());
            logger.debug("result=", result);

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

        if (results) {
            logger.debug("u3Sub.getBlockInfo result is:", results);
            //result.timevalue, result.proposer, result.version, result.previous, result.transaction_mroot, result.action_mroot, result.committee_mroot, result.header_extensions
            pushHeaderToTestnet(results)

        }

    });


}

/**
 * 获取主网子链的committee
 * @returns {Promise<void>}
 */
const getSubchainCommittee = async () => {
    //获取本地producer列表
    jsonArray = await getProducerLists();

    logger.debug("从本地获取的的jsonArray=" + jsonArray);


    let result = await u3.getSubchainCommittee({"chain_name": chain_name.toString()});


    invokeSystemContract(result);

}


/**
 * 调用智能合约的入口方法
 * @param config 配置文件
 * @param contractName
 * @param actionName
 * @param params
 * @param accountName
 * @param privateKey
 * @returns {Promise<void>}
 */
async function contractInteract(config, contractName, actionName, params, accountName, privateKey) {
    try {
        const keyProvider = [privateKey];
        const u3 = createU3({...config, keyProvider});

        const contract = await u3.contract(contractName);
        logger.debug("contract=", JSON.stringify(contract.fc.abi.structs));
        if (!contract) {
            throw new Error("can't found contract");
        }
        if (!contract[actionName] || typeof contract[actionName] !== 'function') {
            throw new Error("action doesn't exist");
        }
        const data = await contract[actionName](params, {
            authorization: [`${accountName}@active`],
        });
        logger.debug('contractInteract success :', actionName);
    } catch (err) {
        logger.debug('contractInteract error :', actionName);
        logger.error('contractInteract error :', err);
    }
}

//程序主入口
scheduleCronstyle();