const {U3} = require('u3.js');
const {createU3, format} = U3;
const axios = require('axios')
var qs = require('qs');


/**
 * 链相关操作的api
 */
var logger = require("../config/logConfig").getLogger("ChainApi");
var logUtil = require("../common/util/logUtil")
var constant = require("../common/constant/constants")
var utils = require("../common/util/utils");
var sleep = require("sleep")

/**
 * 获取主网主链Chain Id
 * @param config
 * @returns {Promise<*>}
 */
const getMainChainId = async (config) => {

    try {
        var u3 = createU3({...config, sign: true, broadcast: true});
        var blockInfo = await u3.getBlockInfo("1");
        //logger.debug("block info  blockInfo.action_mroot=", blockInfo.action_mroot);
        return blockInfo.action_mroot;
    } catch (e) {
        logger.error("get main chain id error:", utils.logNetworkError(e))
    }

}

/**
 * 获取子网子链Chain Idf
 * @param configSub
 * @returns {Promise<*>}
 */
const getSubChainId = async (configSub) => {

    try {
        var u3Sub = createU3({...configSub, sign: true, broadcast: true});
        var blockInfo = await u3Sub.getBlockInfo("1");
        //logger.debug("block info  blockInfo.action_mroot=", blockInfo.action_mroot);
        return blockInfo.action_mroot;
    } catch (e) {
        logger.error("get sub chain id error:", utils.logNetworkError(e))
    }
}

const getChainName = async (configSub) => {
    let chainName = null;
    try {
        let res = await getTableAllData(configSub, constant.contractConstants.ULTRAINIO, constant.contractConstants.ULTRAINIO, constant.tableConstants.GLOBAL, null);
        logger.debug("global table :", res);
        if (utils.isNotNull(res) && res.rows.length > 0) {
            chainName = res.rows[0].chain_name;
        }
    } catch (e) {
        logger.error("getChainName error,", e);
    }

    return chainName;
}

/**
 * 根据user 获取chain name
 * @param user
 * @returns {Promise<*|number|Location|string|WorkerLocation>}
 */
const getChainInfo = async function initChainName(u3, user) {

    let result = null;
    try {
        result = await u3.getProducerInfo({"owner": user});
        logger.debug("getChainInfo", result);
    } catch (e) {
        logger.error("getChainInfo error:", utils.logNetworkError(e));
    }

    return result;

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
 * 获取账号
 * @param config
 * @param accountName
 * @returns {Promise<null>}
 */
async function getAccount(config, accountName) {
    try {
        const rs = await axios.post(config.httpEndpoint + "/v1/chain/get_account_info", {"account_name": accountName});
        return rs.data;
    } catch (e) {
        //logger.error("getAccount("+accountName+") error",e);
    }
    return null;

}

/**
 * 获取本地producer列表
 * @returns {Promise<Array>}
 */
async function getProducerLists(configSub) {
    const params = {"json": "true", "lower_bound": "0", "limit": 100};
    const rs = await axios.post(configSub.httpEndpoint + "/v1/chain/get_producers", params);
    // const rs = await axios.post("http://172.16.10.5:8899/v1/chain/get_producers", params);

    logger.debug("getProducerLists:", rs.data.rows);
    var result = [];
    var rows = rs.data.rows;
    for (var i in rows) {
        var row = rows[i];
        if (row.is_enabled == 1) {
            result.push({
                owner: row.owner,
                miner_pk: row.producer_key,
                bls_pk: row.bls_key,
            });
        }
    }

    //logger.debug("getProducerLists result=", result);
    return result;
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

        //logger.error("contractInteract",privateKey);
        const keyProvider = [privateKey];
        const u3 = createU3({...config, keyProvider});

        logger.debug("keyProvider:", keyProvider);

        const contract = await u3.contract(contractName);
        //logger.debug("contract=", JSON.stringify(contract.fc.abi.structs));
        if (!contract) {
            throw new Error("can't found contract " + contractName);
        }
        if (!contract[actionName] || typeof contract[actionName] !== 'function') {
            throw new Error("action doesn't exist:" + actionName);
        }
        const data = await contract[actionName](params, {
            authorization: [`${accountName}@active`],
        });
        logger.debug('contractInteract success :', actionName);
        return data;
    } catch (err) {
        logger.debug('contractInteract error :', actionName);
        logger.error('' + actionName + ' error :', err);
    }
    return null;
}


/**
 * 通过主链获取子链的userbulletin
 * @returns {Promise<*>}
 */
getUserBulletin = async (u3, chain_name) => {
    try {
        return await u3.getUserBulletin({"chain_name": parseInt(chain_name, 10)});
    } catch (e) {
        logger.error("get user bulletin error :", e.code);
    }

    return null;

}

/**
 * 根据链名称获取种子ip
 * @param chain
 * @returns {Promise<string>}
 */
getChainSeedIP = async (chainName, chainConfig) => {

    // logger.debug(chainConfig.seedIpConfig);
    // logger.debug(chainName);
    try {
        if (utils.isNotNull(chainConfig.seedIpConfig)) {
            for (let i = 0; i < chainConfig.seedIpConfig.length; i++) {
                //logger.debug(chainConfig.seedIpConfig[i])
                if (chainConfig.seedIpConfig[i].chainName == chainName) {
                    return chainConfig.seedIpConfig[i].seedIp;
                }
            }
        }

    } catch (e) {
        logger.error("get chain seed ip error:", e);
    }
    return utils.getLocalIPAdress();
}

/**
 * 根据链名称获取白名单公钥信息（genesis+seed）
 * @param chain
 * @returns {Promise<string>}
 */
getChainPeerKey = async (chainName, chainConfig) => {

    // logger.debug(chainConfig.seedIpConfig);
    // logger.debug(chainName);
    try {
        if (utils.isNotNull(chainConfig.seedIpConfig)) {
            for (let i = 0; i < chainConfig.seedIpConfig.length; i++) {
                //logger.debug(chainConfig.seedIpConfig[i])
                if (chainConfig.seedIpConfig[i].chainName == chainName) {
                    return chainConfig.seedIpConfig[i].peerKeys;
                }
            }
        }

    } catch (e) {
        logger.error("get getChainPeerKey error:", e);
    }
    return [];
}

/**
 * 根据链id获取链已上传的ws的height和hash
 * @returns {Promise<void>}
 */
getSubchainWSHash = async (config, chainName) => {
    try {
        const params = {"chainName": chainName, "height": "0"};
        return await axios.post(config.httpEndpoint + "/v1/chain/get_subchain_ws_hash", params);
    } catch (e) {
        logger.error("getSubchainWSHash error:", e);
    }

    return null;
}

/**
 * 获取表数据
 * @param config
 * @param code
 * @param scope
 * @param table
 * @param limit
 * @param table_key
 * @param lower_bound
 * @param upper_bound
 * @returns {Promise<*>}
 */
getTableInfo = async (config, code, scope, table, limit, table_key, lower_bound, upper_bound) => {
    try {
        const params = {"code": code, "scope": scope, "table": table, "json": true, "key_type": "name"};
        logger.debug(params);
        if (utils.isNotNull(limit)) {
            params.limit = limit;
        }
        if (utils.isNotNull(table_key)) {
            params.table_key = table_key;
        }
        if (utils.isNotNull(lower_bound)) {
            params.lower_bound = lower_bound;
        }
        if (utils.isNotNull(upper_bound)) {
            params.upper_bound = upper_bound;
        }
        let res = await axios.post(config.httpEndpoint + "/v1/chain/get_table_records", params);
        // logger.debug(res);
        return res.data;
    } catch (e) {
        logger.error("get_table_records error:", utils.logNetworkError(e));
    }

    return null;
}

/**
 * 获取全表数据
 * @param config
 * @param code
 * @param scope
 * @param table
 * @returns {Promise<*>}
 */
getTableAllData = async (config, code, scope, table, pk) => {
    let tableObj = {rows: [], more: false};
    let count = 10000; //MAX NUM
    let limit = 1000; //limit
    let finish = false;
    let lower_bound = null;
    var index = 0;
    try {
        while (finish == false) {
            logger.info("table: " + table + " scope:" + scope + " lower_bound(request)：" + lower_bound);
            index++;
            let tableinfo = await getTableInfo(config, code, scope, table, limit, null, lower_bound, null);
            logger.debug("tableinfo:" + table + "):", tableinfo);
            if (utils.isNullList(tableinfo.rows) == false) {
                for (let i = 0; i < tableinfo.rows.length; i++) {
                    if (tableinfo.rows[i][pk] != lower_bound || lower_bound == null) {
                        tableObj.rows.push(tableinfo.rows[i]);
                    }
                }

                if (utils.isNotNull(pk)) {
                    lower_bound = tableinfo.rows[tableinfo.rows.length - 1][pk];
                }

                logger.info("table: " + table + " scope:" + scope + " lower_bound(change)：" + lower_bound);
            }

            //查看是否还有
            finish = true;
            if (utils.isNotNull(tableinfo.more) && tableinfo.more == true) {
                finish = false;
            }
            logger.debug("tableinfo more：" + tableinfo.more);
            if (index * limit >= count) {
                logger.info("table: " + table + " count > " + count + " break now!");
                break;
            }

        }
    } catch (e) {
        logger.error("getTableAllData error:", e);
    }

    logger.debug("getTableAllData(" + table + "):", tableObj);
    return tableObj;

}

/**
 * 返回不同子链对应的非noneproducer的链接点
 * @param chainId
 * @returns {Promise<string>}
 */
getSubchanEndPoint = async (chainName, chainConfig) => {

    // logger.debug(chainConfig.seedIpConfig);
    // logger.debug(chainName);
    try {
        if (utils.isNotNull(chainConfig.seedIpConfig)) {
            for (let i = 0; i < chainConfig.seedIpConfig.length; i++) {
                //logger.debug(chainConfig.seedIpConfig[i])
                if (chainConfig.seedIpConfig[i].chainName == chainName) {
                    return chainConfig.seedIpConfig[i].subchainHttpEndpoint;
                }
            }
        }

    } catch (e) {
        logger.error("getSubchanEndPoint error:", e);
    }
    return "";
}

/**
 * 返回不同子链对应的monitor-server-endpoint的链接点
 * @param chainId
 * @returns {Promise<string>}
 */
getSubchanMonitorService = async (chainName, chainConfig) => {

    // logger.debug(chainConfig.seedIpConfig);
    // logger.debug(chainName);
    try {
        if (utils.isNotNull(chainConfig.seedIpConfig)) {
            for (let i = 0; i < chainConfig.seedIpConfig.length; i++) {
                //logger.debug(chainConfig.seedIpConfig[i])
                if (chainConfig.seedIpConfig[i].chainName == chainName) {
                    return chainConfig.seedIpConfig[i].monitorServerEndpoint;
                }
            }
        }

    } catch (e) {
        logger.error("getSubchanMonitorService error:", e);
    }
    return "";
}

getSubchainConfig = async (chainName, chainConfig) => {

    // logger.debug(chainConfig.seedIpConfig);
    // logger.debug(chainName);
    try {
        if (utils.isNotNull(chainConfig.seedIpConfig)) {
            for (let i = 0; i < chainConfig.seedIpConfig.length; i++) {
                //logger.debug(chainConfig.seedIpConfig[i])
                if (chainConfig.seedIpConfig[i].chainName == chainName) {
                    return chainConfig.seedIpConfig[i];
                }
            }
        }

    } catch (e) {
        logger.error("getSubchainConfig error:", e);
    }
    return "";
}

/**
 * 获取子链近半小时更新的资源信息
 * @param chainName
 * @param chainConfig
 * @returns {Promise<*>}
 */
getSubchainResource = async (chainName, chainConfig) => {

    try {
        const params = {"chain_name": chainName};
        const rs = await axios.post(chainConfig.config.httpEndpoint + "/v1/chain/get_subchain_resource", params);
        return rs.data;
    } catch (e) {
        logger.error("getSubchainResource error:", e);
    }

    return null;
}

/**
 *
 * @param config
 * @returns {Promise<*>}
 */
const getChainBlockDuration = async (config) => {
    try {
        const rs = await axios.post(config.httpEndpoint + "/v1/chain/get_chain_info", {});
        return rs.data.block_interval_ms;
    } catch (e) {
        logger.error("getChainBlockDuration error,", e);
    }

    return null;

}

/**
 * monitor check in
 * @param url
 * @param param
 * @returns {Promise<*>}
 */
monitorCheckIn = async (url, param) => {
    try {
        logger.info("monitorCheckIn param:", qs.stringify(param));
        //var url = "http://172.16.10.5:8078/filedist/checkIn";
        const rs = await axios.post(url + "/filedist/checkIn", qs.stringify(param));
        logger.info("monitorCheckIn result:", rs.data);
        return rs.data;
    } catch (e) {
        logger.error("monitorCheckIn error,", e);
    }
}

/**
 *
 * @param url
 * @param param
 * @returns {Promise<*>}
 */
checkDeployFile = async (url, param) => {
    try {
        logger.debug("checkDeployFile param:", qs.stringify(param));
        const rs = await axios.post(url + "/filedist/checkDeployInfo", qs.stringify(param));
        logger.info("checkDeployFile result:", rs.data);
        return rs.data;
    } catch (e) {
        logger.error("checkDeployFile error,", e);
    }
}

/**
 *
 * @param url
 * @param param
 * @returns {Promise<*>}
 */
finsihDeployFile = async (url, param) => {
    try {
        logger.debug("finishDeployInfo param:", qs.stringify(param));
        const rs = await axios.post(url + "/filedist/finishDeployInfo", qs.stringify(param));
        logger.info("finishDeployInfo result:", rs.data);
        return rs.data;
    } catch (e) {
        logger.error("finishDeployInfo error,", e);
    }
}

/**
 *
 * @returns {Promise<void>}
 */
getSubchainList = async (chainConfig) => {
    let apiArray = [];
    try {
        if (utils.isNotNull(chainConfig.seedIpConfig)) {
            for (let i = 0; i < chainConfig.seedIpConfig.length; i++) {
                //logger.debug(chainConfig.seedIpConfig[i])
                apiArray.push({
                    "chainName":chainConfig.seedIpConfig[i].chainName,
                    "httpEndpoint":chainConfig.seedIpConfig[i].subchainHttpEndpoint
                })
            }
        }
    } catch (e) {
        logger.error("getSubchainList error,", e);
    }

    return apiArray;
}

getSyncBlockChainList = async (chainConfig,isMainChain) => {
    if (isMainChain) {
        return await getSubchainList(chainConfig);
    } else {
        return [{
            "chainName":constant.chainNameConstants.MAIN_CHAIN_NAME,
            "httpEndpoint":chainConfig.config.httpEndpoint
        }];
    }
}


/**
 * 获取目标chain中已同步的块信息
 * @param configTemp
 * @param targetChainHttp
 * @param targetChainName
 * @param searchChainName
 * @returns {Promise<*>}
 */
const getTargetChainBlockNum = async (configTemp,targetChainHttp,targetChainName,searchChainName) => {

    try {
        configTemp.httpEndpoint = targetChainHttp;
        var u3Temp = createU3({...configTemp, sign: true, broadcast: true});
        var subchainBlockNumResult = await u3Temp.getSubchainBlockNum({"chain_name": searchChainName});
        return subchainBlockNumResult;
    } catch (e) {
        logger.error("getTargetChainBlockNum error:", utils.logNetworkError(e))
    }
}

/**
 *
 * @param url
 * @param param
 * @returns {Promise<*>}
 */
addSwitchLog = async (url, param) => {
    try {
        logger.debug("addSwitchLog param:", qs.stringify(param));
        const rs = await axios.post(url + "/filedist/addSwitchLog", qs.stringify(param));
        logger.info("addSwitchLog result:", rs.data);
        return rs.data;
    } catch (e) {
        logger.error("addSwitchLog error,", e);
    }
}






module.exports = {
    getMainChainId,
    getSubChainId,
    getChainInfo,
    getRemoteIpAddress,
    getProducerLists,
    contractInteract,
    getUserBulletin,
    getAccount,
    getTableInfo,
    getTableAllData,
    getChainSeedIP,
    getSubchanEndPoint,
    getSubchainConfig,
    getSubchanMonitorService,
    getSubchainResource,
    getChainBlockDuration,
    monitorCheckIn,
    checkDeployFile,
    finsihDeployFile,
    getChainPeerKey,
    getChainName,
    getSubchainList,
    getSyncBlockChainList,
    getTargetChainBlockNum,
    addSwitchLog
}
