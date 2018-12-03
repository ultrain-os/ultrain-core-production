// import {getActionsByAccount} from "u3.js/src";

const {U3} = require('u3.js');
const axios = require('axios')
const path = require('path');
const fs = require('fs');
const ini = require('ini');
const schedule = require('node-schedule');
const {createU3, format} = U3;


var myAccountAsCommittee = "";
var mySkAsCommittee = "";
var jsonArray = [];
var prefix = "";
var endpoint = "";
var url="";
var chain_name="";


var config = {
    // httpEndpoint:'http://172.16.10.4:8888',
    // httpEndpoint_history: 'http://172.16.10.4:3000',
    // keyProvider: ['5HvhChtH919sEgh5YjspCa1wgE7dKP61f7wVmTPsedw6enz6g7H'], // WIF string or array of keys..


    // httpEndpoint: 'http://40.117.73.83:8888',
    // httpEndpoint_history: 'http://40.117.73.83:3000',
    // keyProvider: ['5KATDC5YqmSTfy99BWqRugriDmeaTAqpwZxXV8jQafdwJqTaqF4'], // WIF string or array of keys..
    httpEndpoint: "",
    httpEndpoint_history: "",
    keyProvider: ['5HvhChtH919sEgh5YjspCa1wgE7dKP61f7wVmTPsedw6enz6g7H'], // WIF string or array of keys..

    chainId: 'x', // 32 byte (64 char) hex string
    // chainId: '2616bfbc21e11d60d10cb798f00893c2befba10e2338b7277bb3865d2e658f58', // 32 byte (64 char) hex string
    expireInSeconds: 60,
    broadcast: true,
    sign: true,

    debug: true,
    verbose: false,
    logger: {
        log: console.log,
        error: console.error,
        debug: console.log
    },
    binaryen: require('binaryen'),
    symbol: 'UGAS'

};


let configSub = {
    // httpEndpoint:'http://172.16.10.4:8888',
    // httpEndpoint_history: 'http://172.16.10.4:3000',
    // keyProvider: ['5HvhChtH919sEgh5YjspCa1wgE7dKP61f7wVmTPsedw6enz6g7H'], // WIF string or array of keys..


    // httpEndpoint: 'http://40.117.73.83:8888',
    // httpEndpoint_history: 'http://40.117.73.83:3000',
    // keyProvider: ['5KATDC5YqmSTfy99BWqRugriDmeaTAqpwZxXV8jQafdwJqTaqF4'], // WIF string or array of keys..
    // httpEndpoint: 'http://172.16.10.5:8877',
    // httpEndpoint_history: 'http://172.16.10.5:3000',
    // httpEndpoint: 'http://172.16.10.4:8877',
    // httpEndpoint_history: 'http://172.16.10.4:3000',
    httpEndpoint: 'http://127.0.0.1:8888',
    httpEndpoint_history: 'http://127.0.0.1:3000',
    keyProvider: ['5HvhChtH919sEgh5YjspCa1wgE7dKP61f7wVmTPsedw6enz6g7H'], // WIF string or array of keys..

    chainId: "x", // 32 byte (64 char) hex string
    // chainId: '2616bfbc21e11d60d10cb798f00893c2befba10e2338b7277bb3865d2e658f58', // 32 byte (64 char) hex string
    expireInSeconds: 60,
    broadcast: true,
    sign: true,

    debug: true,
    verbose: false,
    logger: {
        log: console.log,
        error: console.error,
        debug: console.log
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
function initConfig() {
    var configIniLocal = ini.parse(fs.readFileSync('./config.ini', 'utf-8'));
    // const filePath  =   path.join(process.cwd(),'config.ini');
    // var configIniLocal = ini.parse(fs.readFileSync(filePath, 'utf-8'));

    configIniLocal.name = 'helloworld';
    configIniLocal.version = '2,0.0';
    // configIniLocal.httpEndpointMain=

    //读取指定config.ini里面的文件路径，只需要修改文件路径即可
    var configIniTarget = ini.parse(fs.readFileSync(configIniLocal.path, 'utf-8'));

    console.log(configIniTarget["my-account-as-committee"]);
    console.log(configIniTarget["my-sk-as-committee"]);
    myAccountAsCommittee = configIniTarget["my-account-as-committee"];
    mySkAsCommittee = configIniTarget["my-account-as-committee"];
    prefix = configIniTarget["prefix"];
    prefix="http://";
    endpoint = configIniTarget["endpoint"];
    endpoint=":8888";
    url= configIniTarget["url"];
    url="http://47.52.43.102:3335/api/remoteEndpoint";
    chain_name=configIniTarget["chain_name"];
    chain_name="11";

    //修改config.ini文件
    // fs.writeFileSync('./config.ini', ini.stringify(configIniLocal, {section: ''}));
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
                    console.log(signedTransaction);

                    u3.pushTx(signedTransaction).then((processedTransaction) => {
                        console.log(processedTransaction);
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
    console.log("block info  blockInfo.action_mroot=", blockInfo.action_mroot);
    return blockInfo.action_mroot;
}

/**
 * 获取子网子链Chain Id
 * @returns {Promise<*>}
 */
const getSubChainId = async () => {

    var u3Sub = createU3({...configSub, sign: true, broadcast: true});
    var blockInfo = await u3Sub.getBlockInfo("1");
    console.log("block info  blockInfo.action_mroot=", blockInfo.action_mroot);
    return blockInfo.action_mroot;
}

/**
 * 根据官网数据获取主网ip地址
 * @returns {Promise<*>}
 */
async function getRemoteIpAddress() {
    url="http://47.52.43.102:3335/api/remoteEndpoint";
    const rs = await axios.get(url);

    return rs.data;
}

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
            console.log("result[i]=", resultJson[i].owner);
            console.log("result[i]=", resultJson[i].miner_pk);

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

    console.log("result array=", result);
    console.log("result array=", result.length);

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

    try {

        u3Sub.contract('ultrainio').then(actions => {
            actions.votecommittee(myAccountAsCommittee, result).then((unsigned_transaction) => {
                u3Sub.sign(unsigned_transaction, mySkAsCommittee, config.chainId).then((signature) => {
                    if (signature) {
                        let signedTransaction = Object.assign({}, unsigned_transaction.transaction, {signatures: [signature]});
                        console.log(signedTransaction);

                        u3Sub.pushTx(signedTransaction).then((processedTransaction) => {
                            console.log(processedTransaction);
                            // assert.equal(processedTransaction.transaction_id, unsigned_transaction.transaction_id);
                        });
                    }
                })
            })
        });
        //列表更新
        jsonArray = resultJson;
    } catch (e) {
        console.log("u3 push tx error...", e)
    }

}

/**
 *
 * 把子链的header push到主网上
 *
 * 参数chain_name
 *
 */
pushHeaderToTestnet = async (timestamp, proposer, version, previous, transaction_mroot, action_mroot, committee_mroot, header_extensions) => {
    console.log("=======" + timestamp + "," + proposer + "," + version + "," + previous + "," + transaction_mroot + "," + action_mroot + "," + committee_mroot + "," + header_extensions)

    const params = {
        chain_name: parseInt(chain_name, 10),
        header: {
            "timestamp": timestamp,
            "proposer": proposer,
            // "proposerProof": proposerProof,
            "version": version,
            "previous": previous,
            "transaction_mroot": transaction_mroot,
            "action_mroot": action_mroot,
            "committee_mroot": committee_mroot,
            "header_extensions": [],
        }
    };

    contractInteract('ultrainio', "acceptheader", params, myAccountAsCommittee, config.keyProvider[0]);
}


/**
 *
 * 定时任务调度，3s获取一次
 *
 *
 */
async function scheduleCronstyle() {
    initConfig();

    var ip = await getRemoteIpAddress();

    console.log("before config.httpEndpoint=", config.httpEndpoint + ":8877");
    config.httpEndpoint = prefix + ip + endpoint;
    console.log("after config.httpEndpoint=", config.httpEndpoint);

    console.log("config.chainId=", config.chainId);

    config.chainId = await getMainChainId();

    console.log("config.chainId=", config.chainId);

    console.log("configSub.chainId=", configSub.chainId);

    configSub.chainId = await getSubChainId();

    console.log("configSub.chainId=", configSub.chainId);

    // let u3 = createU3(config);
    u3 = createU3({...config, sign: true, broadcast: true});
    u3Sub = createU3({...configSub, sign: true, broadcast: true});

    schedule.scheduleJob('*/3 * * * * *', function () {
        console.log('scheduleCronstyle:' + new Date());

        getBlocks();

        getSubchainCommittee();

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
    // console.log("===============u3.resultJson.results.results[0].block result is:", resultJson.results);

    let blockNum = await u3.getSubchainBlockNum({"chain_name": chain_name});
    console.log("u3.getSubchainBlockNum  blockNum=" + blockNum);

    let blockNumInt = parseInt(blockNum, 10) + 1;
    if (blockNumInt < 2) {
        blockNumInt = 2;
    }

    let result = await u3Sub.getBlockInfo((blockNumInt).toString());

    if (result) {
        console.log("u3Sub.getBlockInfo result is:", result);
        pushHeaderToTestnet(result.timevalue, result.proposer, result.version, result.previous, result.transaction_mroot, result.action_mroot, result.committee_mroot, result.header_extensions)

    }

}

/**
 * 获取主网子链的committee
 * @returns {Promise<void>}
 */
const getSubchainCommittee = async () => {
    console.log("缓存的jsonArray=" + jsonArray);
    let result = await u3.getSubchainCommittee({"chain_name": chain_name});


    invokeSystemContract(result);

}


/**
 * 调用智能合约的入口方法
 * @param contractName
 * @param actionName
 * @param params
 * @param accountName
 * @param privateKey
 * @returns {Promise<void>}
 */
async function contractInteract(contractName, actionName, params, accountName, privateKey) {
    try {
        const keyProvider = [privateKey];
        const u3 = createU3({...config, keyProvider});

        const contract = await u3.contract(contractName);
        console.log("contract",contract);
        console.log("params=",params);
        if (!contract) {
            throw new Error("can't found contract");
        }
        if (!contract[actionName] || typeof contract[actionName] !== 'function') {
            throw new Error("action doesn't exist");
        }
        const data = await contract[actionName](params, {
            authorization: [`${accountName}@active`],
        });
        console.log('success')
    } catch (err) {
        console.error(err);
    }
}


//程序主入口
scheduleCronstyle();