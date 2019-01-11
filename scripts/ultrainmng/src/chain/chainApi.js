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
    logger.debug(result);
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

    logger.debug("getProducerLists result=", result);
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
async function contractInteract(u3, contractName, actionName, params, accountName) {
    try {
        // const keyProvider = [privateKey];
        // const u3 = createU3({...config, keyProvider});

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


/**
 * 通过主链获取子链的userbulletin
 * @returns {Promise<*>}
 */
getUserBulletin = async (u3, chain_name) => {
    try {
        return await u3.getUserBulletin({"chain_name": parseInt(chain_name, 10)});
    } catch (e) {
        logger.error("get user bulletin error :", e);
    }

    return null;

}

/**
 * 根据链id获取种子ip
 * @param chain
 * @returns {Promise<string>}
 */
getChainSeedIP = async (chain) => {
    return "11.11.11.11";
}

/**
 * 根据链id获取链已上传的ws的height和hash
 * @returns {Promise<void>}
 */
getSubchainWSHash = async (config,chainName) => {
    try {
        const params = {"chainName": chainName, "height": "0"};
        return await axios.post(config.httpEndpoint + "/v1/chain/get_subchain_ws_hash", params);
    } catch (e) {
        logger.error("getSubchainWSHash error:",e);
    }

    return null;
}

module.exports = {
    getMainChainId,
    getSubChainId,
    getChainName,
    getRemoteIpAddress,
    getProducerLists,
    contractInteract,
    getUserBulletin
}