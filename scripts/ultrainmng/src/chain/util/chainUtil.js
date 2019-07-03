var logger = require("../../config/logConfig").getLogger("ChainUtil")
var utils = require('../../common/util/utils')
const fs = require('fs');
var constants = require('../../common/constant/constants')
var rand = require('random-lib')
var url = require('url');

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


/**
 * 通过块信息获取转账的交易列表
 * @param blockInfo
 * @param chainInfo
 * @returns {Array}
 */
function getTransFromBlockHeader(blockInfo,chainName) {
    let trans = [];
    try {
        let transList = blockInfo.transactions;
        if (transList.length > 0) {
            for (let i=0;i<transList.length;i++) {
                let tranInfo = transList[i];
                if (tranInfo.status == "executed") {
                    try {
                        logger.debug("traninfo trx :", tranInfo.trx);
                        logger.debug("traninfo trx actions :", tranInfo.trx.transaction.actions);
                        let actions = tranInfo.trx.transaction.actions;
                        for (let t = 0; t < actions.length; t++) {
                            let action = actions[t];
                            if (action.data.to == constants.contractConstants.UTRIO_BANK && action.data.memo == chainName) {
                                logger.debug("find useful action:", action);
                                logger.debug("find useful tran:", tranInfo);
                                trans.push(tranInfo);
                                break;
                            }
                        }
                    } catch (e) {
                        logger.error("getTransFromBlockHeader error",e);
                    }
                }
            }
        }


    } catch (e) {
        logger.error("getTransFromBlockHeader error",e);
    }

    return trans;
}


/**
 * 通过块信息获取转账的交易列表
 * @param blockInfo
 * @param chainInfo
 * @returns {Array}
 */
function getSyncUserTransFromBlockHeader(blockInfo,chainName,user) {
    let trans = [];
    try {
        let transList = blockInfo.transactions;
        if (transList.length > 0) {
            for (let i=0;i<transList.length;i++) {
                let tranInfo = transList[i];
                try {
                    if (tranInfo.status == "executed") {
                        logger.debug("traninfo trx :", tranInfo.trx);
                            logger.debug("traninfo trx actions :", tranInfo.trx.transaction.actions);
                            let actions = tranInfo.trx.transaction.actions;
                            for (let t = 0; t < actions.length; t++) {
                                let action = actions[t];
                                if (action.name == "empoweruser" && action.data.chain_name == chainName && action.data.user == user) {
                                    logger.debug("[Sync User]find useful action:", action);
                                    logger.debug("find useful tran:", tranInfo);
                                    trans.push(tranInfo);
                                    break;
                                }
                            }
                    }
                } catch (e) {
                    logger.error("getSyncUserTransFromBlockHeader error",e);
                }
            }
        }


    } catch (e) {
        logger.error("getSyncUserTransFromBlockHeader error",e);
    }

    return trans;
}

/**
 * 通过资源你信息获取转账的交易列表
 * @param blockInfo
 * @param chainInfo
 * @returns {Array}
 */
function getSyncResTransFromBlockHeader(blockInfo,chainName,user) {
    let trans = [];
    try {
        let transList = blockInfo.transactions;
        if (transList.length > 0) {
            for (let i=0;i<transList.length;i++) {
                try {
                    let tranInfo = transList[i];
                    if (tranInfo.status == "executed") {
                        logger.debug("traninfo trx :", tranInfo.trx);
                        logger.debug("traninfo trx actions :", tranInfo.trx.transaction.actions);
                        let actions = tranInfo.trx.transaction.actions;
                        for (let t = 0; t < actions.length; t++) {
                            let action = actions[t];
                            if (action.name == "resourcelease" && action.data.location == chainName && action.data.receiver == user) {
                                logger.debug("find useful action:", action);
                                logger.debug("find useful tran:", tranInfo);
                                trans.push(tranInfo);
                                break;
                            }
                        }
                    }
                } catch (e) {
                    logger.error("getSyncResTransFromBlockHeader error",e);
                }
            }
        }


    } catch (e) {
        logger.error("getSyncResTransFromBlockHeader error",e);
    }

    return trans;
}

/**
 *
 * @param blockInfo
 * @param chainName
 * @returns {Array}
 */
function getMoveProdRTransFromBlockHeader(blockInfo,chainName) {
    let trans = [];
    try {
        let transList = blockInfo.transactions;
        if (transList.length > 0) {
            for (let i=0;i<transList.length;i++) {
                try {
                    let tranInfo = transList[i];
                    if (tranInfo.status == "executed") {
                        logger.debug("traninfo trx :", tranInfo.trx);
                        logger.debug("traninfo trx actions :", tranInfo.trx.transaction.actions);
                        let actions = tranInfo.trx.transaction.actions;
                        for (let t = 0; t < actions.length; t++) {
                            let action = actions[t];
                            if (action.name == "moveprod" && (action.data.to_chain == chainName || action.data.from_chain == chainName)) {
                                //logger.info("find useful action:", action);
                                logger.info("find useful tran:", tranInfo);
                                trans.push(tranInfo);
                                break;
                            }
                        }
                    }
                } catch (e) {
                    logger.error("getMoveProdRTransFromBlockHeader error",e);
                }
            }
        }


    } catch (e) {
        logger.error("getMoveProdRTransFromBlockHeader error",e);
    }

    return trans;
}


function transferTrxReceiptBytesToArray(bytes) {

    let array = [];
    try {
        if (bytes.length % 2 == 0) {
            let count = bytes.length / 2;
            let i = 0;
            while (i<count) {
                let str = bytes.substr(i*2,2);
                var num=parseInt(str,16);
                array.push(num);
                i++;
            }
        }

    } catch (e) {
        logger.error("transferTrxReceiptBytesToArray error,",e);
    }

    return array;

}

function transferFreeMemToArray(memStr) {

    let array = [];
    var ValidChars = "0123456789";
    try {
        let number = "";
        for (let i=0;i<memStr.length;i++) {
            let n = memStr[i].trim();
            if (ValidChars.indexOf(n) == -1 || n.length <=0) {
                if (number.length > 0) {
                    array.push(number);
                }
                number = "";
            } else {
                number = number +n;
            }
        }

    } catch (e) {
        logger.error("transferFreeMemToArray error:",e);
    }

    return array;
}

/**
 * 随机概率
 * @param setRatio
 * @returns {boolean}
 */
function randomRato(setRatio) {
    if (setRatio >= 1) {
        return true
    }
    let result = rand.intsSync({min: 1, max: 100, num: 1});
    if (result <= setRatio * 100) {
        return true;
    }

    return false;
}

/**
 *
 * @param url
 * @returns {{proxy: string, path: string, port: number, host: string}}
 */
function parseUrlInfo(downloadurl) {
    let parseInfo = {
        proxy : "http",
        host :"",
        port:80,
        path:""
    }

    try {
        parseInfo.path = url.parse(downloadurl).pathname;
        let host = url.parse(downloadurl).host;
        if (host.indexOf(":") != -1) {
            var array=host.split(":");
            parseInfo.host=array[0];
            parseInfo.port = array[1];
        } else {
            parseInfo.host = host;
        }

        if (downloadurl.indexOf("https") != -1) {
            parseInfo.proxy = "https";
        }
    } catch (e) {
        logger.error("parseUrlInfo error:",e);
    }

    return parseInfo;
}

/**
 * 从ws表里获取一个与块高最接近的世界状态文件信息
 */
function getNearestWsInfoByBlockHeight(data, blockHeight) {

    let result = {};
    try {

        if (utils.isNull(data)) {
            return null;
        }

        let rows = data.rows;

        if (utils.isNull(rows) || utils.isNullList(rows)) {
            return null;
        }

        logger.info("getNearestWsInfoByBlockHeight rows:",data.rows);

        for (var i = rows.length - 1; i >= 0; i--) {
            let obj = rows[i];
            if (utils.isNullList(obj.hash_v) == false) {
                logger.debug("getNearestWsInfoByBlockHeight obj:",obj);
                for (var t = 0; t < obj.hash_v.length; t++) {
                    if (obj.hash_v[t].valid == 1 && obj.block_num <= blockHeight) {
                        result.block_num = obj.block_num;
                        result.hash = obj.hash_v[t].hash;
                        result.file_size = obj.hash_v[t].file_size;
                        return result;
                    }
                }
            }

        }

    } catch (e) {
        logger.error("getNearestWsInfoByBlockHeight error:", e);
    }

    return null;
}


module.exports = {
    formatGensisTime,
    getOwnerPkByAccount,
    checkFileExist,
    calcBlockDuration,
    isResourceChanged,
    calcSubchainIntevalBlockHeight,
    getTransFromBlockHeader,
    transferTrxReceiptBytesToArray,
    getSyncUserTransFromBlockHeader,
    getSyncResTransFromBlockHeader,
    transferFreeMemToArray,
    getMoveProdRTransFromBlockHeader,
    randomRato,
    parseUrlInfo,
    getNearestWsInfoByBlockHeight,
}
