const {U3} = require('u3.js');
const fs = require('fs');
var logger = require("../config/logConfig").getLogger("Monitor");
var chainConfig = require("./chainConfig")
var chainApi = require("./chainApi")
var cacheKeyConstants = require("../common/constant/constants").cacheKeyConstants
var filenameConstants = require("../common/constant/constants").filenameConstants
var iniConstants = require("../common/constant/constants").iniConstants
var algorithmConstants = require("../common/constant/constants").algorithmConstants
var utils = require("../common/util/utils")
var CacheObj = require("../common/cache/cacheObj");
var hashUtil = require("../common/util/hashUtil")


var hashCache = new CacheObj(false, null);
/**
 * hash缓存过期时间（1分钟）
 * @type {number}
 */
var HASH_EXPIRE_TIME_MS = 1000 * 60;

/**
 *
 * @returns {string}
 */
async function getNodVersion() {
    let hashFile = hashCache.get(cacheKeyConstants.NOD_FILE_KEY);
    if (utils.isNull(hashFile)) {
        logger.info("cache not hit :"+cacheKeyConstants.NOD_FILE_KEY);
        let nodFilePath = chainConfig.configFileData.local.nodpath + "/" + filenameConstants.NOD_EXE_FILE;
        hashFile =  hashUtil.calcHash(nodFilePath,algorithmConstants.SHA1);
        if (utils.isNotNull(hashFile)) {
            hashCache.put(cacheKeyConstants.NOD_FILE_KEY,hashFile,HASH_EXPIRE_TIME_MS);
            logger.info("cache info :",hashCache.getAll());
        }
    } else {
        logger.info("cache hit :"+cacheKeyConstants.NOD_FILE_KEY,hashFile);
    }
    return hashFile;
}

/**
 *
 * @returns {string}
 */
async function getMngVersion() {
    let hashFile = hashCache.get(cacheKeyConstants.MNG_FILE_KEY);
    if (utils.isNull(hashFile)) {
        logger.info("cache not hit :"+cacheKeyConstants.MNG_FILE_KEY);
        let nodFilePath = chainConfig.configFileData.local.mngpath + "/" + filenameConstants.MNG_FILE;
        hashFile =  hashUtil.calcHash(nodFilePath,algorithmConstants.SHA1);
        if (utils.isNotNull(hashFile)) {
            hashCache.put(cacheKeyConstants.MNG_FILE_KEY,hashFile,HASH_EXPIRE_TIME_MS);
        }
    } else {
        logger.info("cache hit :"+cacheKeyConstants.MNG_FILE_KEY,hashFile);
    }
    return hashFile;
}

/**
 *
 * @returns {*}
 */
function getMonitorUrl() {
    return chainConfig.configFileData.target[iniConstants.MONITOR_SERVER_ENDPOINT];
}

/**
 *
 * @returns {boolean}
 */
function checkNeedSync() {
    if (chainConfig.configFileData.local[iniConstants.MONITOR] == false) {
        return false;
    }

    return true;
}


/**
 * check in
 * @returns {Promise<void>}
 */
async function checkIn() {


    if (checkNeedSync() == false) {
        return
    }

    logger.info("monitor checkin start");
    logger.info("monitor check in(http:" + getMonitorUrl());
    let publicip = await utils.getPublicIp();
    logger.debug("extern ip:" + publicip);
    var user = "unkown";
    if (utils.isNotNull(chainConfig.myAccountAsCommittee)) {
        user = chainConfig.myAccountAsCommittee;
    }
    var nodFileHash = await getNodVersion();
    if (utils.isNull(nodFileHash)) {
        logger.error("nod file hash error");
        return;
    }

    var mngFileHash = await getMngVersion();
    if (utils.isNull(mngFileHash)) {
        logger.error("mng file hash error");
        return;
    }
    var param = {
        "chainId": chainConfig.chainName,
        "ipLocal": utils.getLocalIPAdress(),
        "ipPublic": publicip,
        "user": user,
        "nodVersion": nodFileHash,
        "mngVersion": mngFileHash
    };

    await chainApi.monitorCheckIn(getMonitorUrl(), param);

    logger.info("monitor checkin end");
}


module.exports = {
    checkIn
}
