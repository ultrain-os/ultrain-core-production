const http = require('http');
var querystring = require('querystring');
const axios = require('axios')
var logger = require("../config/logConfig").getLogger("WorldState");
var IniFile = require('../common/util/iniFile');
var Constants = require('../common/constant/constants');
var utils = require("../common/util/utils")
var wsResUtil = require("./wsResUtil")
var ShellCmd = require("../common/util/shellCmd")
var sleep = require('sleep')
const path = require('path');

/**
 * 世界状态通信对象
 */
class WorldState {
}

//wss提供的请求地址版本
WorldState.port = 7777
WorldState.hostname = "127.0.0.1"
// WSS.port = 8888
// WSS.hostname = "172.16.10.5"
WorldState.http = "http"
WorldState.version = "v1"

//检查世界状态是否可用时间间隔
WorldState.statusCheckTime = 500;

//配置文件目录与文件名
WorldState.configFilePath = "/root/.local/share/ultrainio/wssultrain/config/"
WorldState.configFileName = "config.ini"

/**
 * 获取 http请求时的options
 * @param path
 * @returns {{path: string, hostname: string, method: string, port: number}}
 */
WorldState.getHttpRequestPath = function (path) {
    return this.http + "://" + this.hostname + ":" + this.port + "/" + this.version + path;
}

//实时状态
WorldState.status = null;

/**
 * 同步世界状态信息
 * @returns {Promise<void>}
 */
WorldState.syncStatus = async function () {
    let data = await this.getSubchainWsInfo();
    if (utils.isNotNull(data)) {
        //比较块高
        if (utils.isNull(this.status) || data.block_height > this.status.block_height) {
            this.status = data;
        }
    }
}

/**
 * 调用url获取请求
 * @param path
 * @param params
 * @returns {Promise<*>}
 */
WorldState.requestData = async function (path, params) {
    try {
        logger.debug("send http request :" + this.getHttpRequestPath(path) + " params:" + params);
        const rs = await axios.post(this.getHttpRequestPath(path), params);
        logger.debug("get http request :" + this.getHttpRequestPath(path) + " params:" + params + " result :", rs.data);
        if (rs.status == 200 || rs.status == 201) {
            return rs.data;
        } else {
            logger.error("request wss service error code (" + this.getHttpRequestPath(path) + ")", rs);
        }
    } catch (e) {
        logger.error("request wss service error (" + this.getHttpRequestPath(path) + ") :", utils.logNetworkError(e));
    }

    return null;
}

/**
 * 同步世界状态
 * @param hash
 * @param height
 * @returns {Promise<*>}
 */
WorldState.syncWorldState = async function (hash, height, file_size, chain_id) {
    let param = {"chain_id": chain_id, "block_height": height, "hash_string": hash, "file_size": file_size};
    logger.info("syncWorldState param:", param)
    let res = await this.requestData("/wss/require_ws", param);
    if (utils.isNotNull(res)) {
        return true;
    }

    return false;
}

/**
 * 请求同步世界状态
 * @returns {Promise<*>}
 */
WorldState.requestWSState = async function () {
    return await this.requestData("/wss/ws_status", "\"ws\"");
}

WorldState.setValidWs = async function(vaildNum) {
    let res =  await this.requestData("/wss/set_vaild_ws", "\""+vaildNum+"\"");
    logger.info("set wss vaild num:",res);
    return res;
}

/**
 * 轮询同步时间状态
 * @param code
 * @param timeInterval 调用间隔时间
 * @param totalTime 总时间
 * @returns {Promise<boolean>}
 */
WorldState.pollingkWSState = async function (timeInterval, totalTime) {

    let resultFlag = false;
    if (totalTime <= 0 || timeInterval <= 0 || timeInterval > totalTime) {
        return result;
    }

    /**
     * 检查状态
     */
    let searchTime = 0;
    let errorTime = 0;
    while (searchTime <= totalTime) {
        logger.info("pollingkWSState status(searchTime: "+searchTime+",totalTime:"+totalTime+"),result:"+resultFlag);
        let result = await WorldState.requestWSState();
        logger.info("pollingkWSState result:", result);
        if (wsResUtil.isSuccess(result)) {
            logger.info("sync worldstate success");
            resultFlag = true;
            break;
        } else {
            if (wsResUtil.isOngoing(result)) {
                logger.info("sync worldstate is ongoing");
                errorTime = 0;
            } else if (wsResUtil.isError(result)) {
                logger.error("sync worldstate is error:");
                errorTime++;
                if (errorTime >= 5) {
                    resultFlag = false;
                    break;
                }
            }
        }
        /**
         * 等待
         */
        sleep.msleep(timeInterval);
        searchTime += timeInterval;
    }
    return resultFlag;
}

/**
 * 轮询检查块状态
 * @param code
 * @param timeInterval 调用间隔时间
 * @param totalTime 总时间
 * @returns {Promise<boolean>}
 */
WorldState.pollingBlockState = async function (timeInterval, totalTime) {

    let result = false;
    if (totalTime <= 0 || timeInterval <= 0 || timeInterval > totalTime) {
        return false;
    }

    /**
     * 检查状态
     */
    let searchTime = 0;
    while (searchTime <= totalTime) {
        let res = await WorldState.requestBlockState();
        if (wsResUtil.isSuccess(res)) {
            logger.info("sync  block success");
            result = true;
        } else {
            if (wsResUtil.isOngoing(res)) {
                logger.info("sync block is ongoing");
            } else if (wsResUtil.isError(res)) {
                logger.error("sync block is error:", res)
            }
        }
        /**
         * 等待
         */
        if (result) {
            break;
        }
        sleep.msleep(timeInterval);
        searchTime += timeInterval;
    }
    return result;
}

/**
 * 同步块信息
 * @param begin
 * @param end
 * @returns {Promise<*>}
 */
WorldState.syncBlocks = async function (chainid, blockHeight) {
    return await this.requestData("/wss/require_block", "[\"" + chainid + "\"," + blockHeight + "]")
}

/**
 * 请求同步块状态
 * @returns {Promise<*>}
 */
WorldState.requestBlockState = async function () {
    return await this.requestData("/wss/ws_status", "\"block\"");
}


/**
 * 检查ws是否是正常状态
 * @returns {Promise<*>}
 */
WorldState.checkAlive = async function () {

    let resut = null;
    try {
        //resut = await this.requestData("/wss/latest_wsinfo", null);
        resut = await this.requestData("/wss/ws_status", "\"\"");
        logger.info("ws result:", resut);
        return resut;
    } catch (e) {
        logger.error("request wss data error:", utils.logNetworkError(e));
        return null;
    }
}

/**
 * 获取子链最新的世界状态
 * @returns {Promise<*>}
 *
 * {"chain_id":"0000000000000000000000000000000000000000000000000000000000000000","block_height":0,"hash_string":"","file_size":0}
 */
WorldState.getSubchainWsInfo = async function () {
    let resut = null;
    try {
        resut = await this.requestData("/wss/latest_wsinfo", null);
        logger.debug("result:", resut);
        return resut;
    } catch (e) {
        logger.error("request wss data error:", utils.logNetworkError(e));
        return null;
    }
}

/**
 * 更新世界状态配置文件
 * @param chainId
 * @param seedIp
 * @returns {boolean}
 */
WorldState.updateConfig = function (chainId, seedIp) {

    try {
        if (utils.isNotNull(chainId, seedIp)) {
            var iniFile = new IniFile(this.configFilePath + this.configFileName, Constants.encodingConstants.UTF8);
            iniFile.setValue("chainId", chainId);
            //iniFile.setValue("p2p-peer-address", seedIp + ":7272");

            iniFile.removeKey("p2p-peer-address");
            for (var i = 0; i< seedIp.length;i++) {
                iniFile.addKeyValue("p2p-peer-address", seedIp[i] + ":7272");
            }

            iniFile.setValue("http-server-address", "127.0.0.1:" + this.port);
            iniFile.writefile(this.configFilePath + this.configFileName, Constants.encodingConstants.UTF8);
            return true;
        } else {
            logger.error("update wss config file error,chainId or seedIp is null");
        }
    } catch (e) {
        logger.error("update wss config file error: ", e)
    }
    return false;
}

/**
 * 结束程序
 * @returns {Promise<void>}
 */
WorldState.stop = async function (totalTime) {
    let res = await ShellCmd.execCmd(Constants.cmdConstants.KILL_WORLDSTATE);
    sleep.msleep(this.statusCheckTime);
    //校验端口是否不提供服务
    let searchtime = this.statusCheckTime;
    while (totalTime >= searchtime) {
        if (utils.isNull(await this.checkAlive()) == true) {
            return true;
        }
        sleep.msleep(this.statusCheckTime);
        searchtime += this.statusCheckTime;
    }
    return false;
    //return true;
}

/**
 * 启动
 * @type {WorldState}
 */
WorldState.start = async function (chainId, seedIp, totalTime, wsspath,local) {

    let result = false;
    let args= [];
    /**
     * 更新配置信息（端口号，链id，种子ip
     */
    if (!this.updateConfig(chainId, seedIp)) {
        //return result;
    }

    var shPath= path.join(__dirname, "../../tool/_runworldstate.sh");
    if (local == false) {
        shPath = Constants.cmdConstants.START_WORLDSTATE_FILE;
    }
    logger.info("start ws by shell file:",shPath);
    logger.info("start ws by shell file:",wsspath);
    args.push(wsspath)
    await ShellCmd.execCmdFiles(shPath, args, null);
    //await ShellCmd.execCmdFiles(Constants.cmdConstants.START_WORLDSTATE_FILE, Constants.cmdConstants.START_NODULTRAIN_ARG, null);

    //utils.sleep(this.statusCheckTime);
    let searchtime = this.statusCheckTime;
    while (totalTime >= searchtime) {
        if (utils.isNull(await this.checkAlive()) == false) {
            result = true;
            break;
        }
        sleep.msleep(this.statusCheckTime);
        searchtime += this.statusCheckTime;
    }
    return result;
}


/**
 * 启动
 * @type {WorldState}
 */
WorldState.startWithoutUpdate = async function (totalTime, wsspath,local) {

    let result = false;
    let args= [];

    var shPath= path.join(__dirname, "../../tool/_runworldstate.sh");
    if (local == false) {
        shPath = Constants.cmdConstants.START_WORLDSTATE_FILE;
    }
    logger.info("start ws by shell file:",shPath);
    logger.info("start ws by shell file:",wsspath);
    args.push(wsspath)
    await ShellCmd.execCmdFiles(shPath, args, null);
    //await ShellCmd.execCmdFiles(Constants.cmdConstants.START_WORLDSTATE_FILE, Constants.cmdConstants.START_NODULTRAIN_ARG, null);

    //utils.sleep(this.statusCheckTime);
    let searchtime = this.statusCheckTime;
    while (totalTime >= searchtime) {
        if (utils.isNull(await this.checkAlive()) == false) {
            result = true;
            break;
        }
        sleep.msleep(this.statusCheckTime);
        searchtime += this.statusCheckTime;
    }
    return result;
}


/**
 * 清除DB数据
 * @returns {Promise<void>}
 */
WorldState.clearDB = async function () {
    try {
        await ShellCmd.execCmd(Constants.cmdConstants.CLEAR_WORLD_STATE_FILE);
    } catch (e) {
        logger.error("worldstate remove data error:", e);
    }
}


module.exports = WorldState;
