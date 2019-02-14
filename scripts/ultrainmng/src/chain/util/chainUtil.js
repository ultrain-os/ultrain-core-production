var logger = require("../../config/logConfig").getLogger("ChainUtil")
var utils = require('../../common/util/utils')
const fs = require('fs');

/**
 * 创世时间格式化
 * @param time
 * @returns {*|void|string|never}
 */
function formatGensisTime(time) {
    //return time.replace("T"," ");
    return time;
}

/**
 *
 * @param account
 * @param permName
 * @returns {*}
 */
function getOwnerPkByAccount(account, permName) {

    try {

        let permissions = account.permissions;
        if (permissions.length > 0) {
            for (let i = 0; i < account.permissions.length; i++) {
                var permission = account.permissions[i];
                if (permission.perm_name == permName) {
                    logger.debug("account.permissions[i].required_auth", account.permissions[i]);
                    logger.debug("account.permissions[i].required_auth", account.permissions[i].required_auth);
                    return account.permissions[i].required_auth.keys[0].key;
                }
            }
        }


        if (utils.isNull(account)) {
            return null;
        }


    } catch (e) {
        logger.error("getOwnerPkByAccount error:", e);
    }

    return null;

}

/**
 * filepath check exist
 * @param filepath
 * @returns {boolean}
 */
function checkFileExist(filepath) {
    if (fs.existsSync(filepath)) {
        logger.info("filepath exists:", filepath)
        return true;
    }
    logger.error("filepath not exist:", filepath);
    return false;
}

/**
 *
 * @param startBlockNum
 * @param endBlockNum
 * @param blockDuration(unit:second)
 */
function calcBlockDuration(startBlockNum, endBlockNum, blockDuration) {

    if (endBlockNum <= startBlockNum) {
        return 0;
    }

    try {

        let deltaBlockNum = endBlockNum - startBlockNum;
        logger.debug("deltaBlockNum: ", deltaBlockNum);

        let time = deltaBlockNum * blockDuration;
        logger.debug("deltaTime: ", time);

        return time;

    } catch (e) {
        logger.error("calcBlockDuration error,", e)
    }

    return 0;
}

/**
 *
 * @param mainResObj
 * @param subResObj
 * @param chainConfig
 * @returns {boolean}
 */
function isResourceChanged(mainResObj, subResObj, chainConfig) {

    if (mainResObj.lease_num > subResObj.lease_num ||
        calcBlockDuration(mainResObj.start_block_height, mainResObj.end_block_height, chainConfig.mainChainBlockDuration) > calcBlockDuration(subResObj.start_block_height, subResObj.end_block_height, chainConfig.subChainBlockDuration)) {
        return true;
    }

    return false;

}

/**
 *
 * @param mainchainStartBlockNum
 * @param mainchainEndBlockNum
 * @param mainChainBlockDuration
 * @param subchainBlockDuration
 * @returns {number}
 */
function calcSubchainIntevalBlockHeight(mainchainStartBlockNum, mainchainEndBlockNum, mainChainBlockDuration, subchainBlockDuration) {

    if (mainchainEndBlockNum <= mainchainStartBlockNum) {
        return 0;
    }

    return Math.ceil((mainchainEndBlockNum - mainchainStartBlockNum ) * mainChainBlockDuration / subchainBlockDuration);
}

// console.log("block:",calcSubchainIntevalBlockHeight(1,101,10,21));

// let data = "1971-01-24T14:06:00";
//
// logger.info(formatGensisTime(data));

module.exports = {
    formatGensisTime,
    getOwnerPkByAccount,
    checkFileExist,
    calcBlockDuration,
    isResourceChanged,
    calcSubchainIntevalBlockHeight
}