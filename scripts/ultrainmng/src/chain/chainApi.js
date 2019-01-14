const {U3} = require('u3.js');
const {createU3, format} = U3;
const axios = require('axios')


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

    var u3 = createU3({...config, sign: true, broadcast: true});
    var blockInfo = await u3.getBlockInfo("1");
    //logger.debug("block info  blockInfo.action_mroot=", blockInfo.action_mroot);
    return blockInfo.action_mroot;
}

/**
 * 获取子网子链Chain Id
 * @param configSub
 * @returns {Promise<*>}
 */
const getSubChainId = async (configSub) => {

    var u3Sub = createU3({...configSub, sign: true, broadcast: true});
    var blockInfo = await u3Sub.getBlockInfo("1");
    //logger.debug("block info  blockInfo.action_mroot=", blockInfo.action_mroot);
    return blockInfo.action_mroot;
}

/**
 * 根据user 获取chain name
 * @param user
 * @returns {Promise<*|number|Location|string|WorkerLocation>}
 */
const getChainName = async function initChainName(u3, user) {

    let result = await u3.getProducerInfo({"owner": user});
    //logger.debug(result);
    return result.location;

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
        logger.error('contractInteract error :', err);
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
    return "";
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
        const params = {"code": code, "scope": scope, "table": table, "json": true};
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
        logger.error("get_table_records error:", e);
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
getTableAllData = async (config, code, scope, table) => {
    let tableObj = {rows: [], more: false};
    let count = 10000; //MAX NUM
    let limit = 100; //limit
    let finish = false;
    let lower_bound = null;
    try {
        while (finish == false) {
            let tableinfo = await getTableInfo(config, code, scope, table, limit, null, lower_bound, null);
            logger.debug("tableinfo:", tableinfo);
            if (utils.isNullList(tableinfo.rows) == false) {
                for (let i = 0; i < tableinfo.rows.length; i++) {
                    if (tableinfo.rows[i].owner != lower_bound) {
                        tableObj.rows.push(tableinfo.rows[i]);
                    }
                }
                lower_bound = tableinfo.rows[tableinfo.rows.length - 1].owner;
            }
            logger.debug("lower_bound：" + lower_bound);
            //查看是否还有
            finish = true;
            if (utils.isNotNull(tableinfo.more) && tableinfo.more == true) {
                finish = false;
            }
            logger.debug("tableinfo more：" + tableinfo.more);
            //sleep.msleep(1000);
        }
    } catch (e) {
        logger.error("getTableAllData error:", e);
    }

    return tableObj;

}


module.exports = {
    getMainChainId,
    getSubChainId,
    getChainName,
    getRemoteIpAddress,
    getProducerLists,
    contractInteract,
    getUserBulletin,
    getAccount,
    getTableInfo,
    getTableAllData,
    getChainSeedIP
}