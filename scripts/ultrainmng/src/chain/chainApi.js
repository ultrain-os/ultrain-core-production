const {U3} = require('u3.js');
const {createU3, format} = U3;

/**
 * 链相关操作的api
 */
var logger = require("../config/logConfig");
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
const getChainName = async function initChainName(user) {

    let result = await u3.getProducerInfo({"owner": user});
    logger.debug(result);
    return result.location;

}

module.exports = {
    getMainChainId,
    getSubChainId,
    getChainName,
}