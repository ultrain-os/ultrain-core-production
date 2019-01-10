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
WorldState.statusCheckTime=500;

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

/**
 * 调用url获取请求
 * @param path
 * @param params
 * @returns {Promise<*>}
 */
WorldState.requestData = async function (path, params) {
    try {
        logger.debug("send http request :"+this.getHttpRequestPath(path)+" params:"+params);
        const rs = await axios.post(this.getHttpRequestPath(path), params);
        logger.debug("get http request :"+this.getHttpRequestPath(path)+" params:"+params +" result :",rs.data);
        if (rs.status == 200 || rs.status == 201) {
            return rs.data;
        } else {
            logger.error("request wss service error code (" + this.getHttpRequestPath(path) + ")", rs);
        }
    } catch (e) {
        logger.error("request wss service error (" + this.getHttpRequestPath(path) + ") :",utils.logNetworkError(e));
    }

    return null;
}

/**
 * 同步世界状态
 * @param hash
 * @param height
 * @returns {Promise<*>}
 */
WorldState.syncWorldState = async function (hash, height) {
    console.log("syncWorldState")
    let res = await this.requestData("/wss/require_ws", "[" + height + ",\"" + hash + "\"]");
    if (utils.isNotNull(res)) {
        return true;
    }

    return false;
}

/**
 * 请求同步世界状态
 * @returns {Promise<*>}
 */
WorldState.requestWSState = async function (code) {
    return await this.requestData("/wss/ws_status", "[\"ws\"," + code + "]");
}

/**
 * 轮询同步时间状态
 * @param code
 * @param timeInterval 调用间隔时间
 * @param totalTime 总时间
 * @returns {Promise<boolean>}
 */
WorldState.pollingkWSState = async function (code, timeInterval, totalTime) {

    let result = false;
    if (totalTime <= 0 || timeInterval <= 0 || timeInterval > totalTime) {
        return result;
    }

    /**
     * 检查状态
     */
    let searchTime = 0;
    while (searchTime <= totalTime) {
        /**
         * todo delete mock用
         */
        if (totalTime-searchTime <= timeInterval) {
            code = 0;
        }
        result = await WorldState.requestWSState(code);
        if (wsResUtil.isSuccess(result)) {
            logger.info("sync worldstate success");
            result = true;
            break;
        } else {
            if (wsResUtil.isOngoing(result)) {
                logger.info("sync worldstate is ongoing");
            } else if (wsResUtil.isError(result)) {
                logger.error("sync worldstate is error:")
            }
        }
        /**
         * 等待
         */
        sleep.msleep(timeInterval);
        searchTime +=timeInterval;
    }
    return result;
}

/**
 * 轮询检查块状态
 * @param code
 * @param timeInterval 调用间隔时间
 * @param totalTime 总时间
 * @returns {Promise<boolean>}
 */
WorldState.pollingBlockState = async function (code, timeInterval, totalTime) {

    let result = false;
    if (totalTime <= 0 || timeInterval <= 0 || timeInterval > totalTime) {
        return false;
    }

    /**
     * 检查状态
     */
    let searchTime = 0;
    while (searchTime <= totalTime) {
        /**
         * todo delete mock用
         */
        if (totalTime-searchTime <= timeInterval) {
            code = 0;
        }
        let res = await WorldState.requestBlockState(code);
        if (wsResUtil.isSuccess(res)) {
            logger.info("sync  block success");
            result = true;
        } else {
            if (wsResUtil.isOngoing(res)) {
                logger.info("sync block is ongoing");
            } else if (wsResUtil.isError(res)) {
                logger.error("sync block is error:",res)
            }
        }
        /**
         * 等待
         */
        if (result) {
            break;
        }
        sleep.msleep(timeInterval);
        searchTime +=timeInterval;
    }
    return result;
}

/**
 * 同步块信息
 * @param begin
 * @param end
 * @returns {Promise<*>}
 */
WorldState.syncBlocks = async function () {
    return await this.requestData("/wss/require_block", "[0,0]")
}

/**
 * 请求同步块状态
 * @returns {Promise<*>}
 */
WorldState.requestBlockState = async function (code) {
    return await this.requestData("/wss/ws_status", "[\"block\","+code+"]");
}


/**
 * 检查ws是否是正常状态
 * @returns {Promise<*>}
 */
WorldState.checkAlive = async function () {
    return wsResUtil.isSuccess(await this.requestData("/wss/ws_status", "[\"\",0]"));
}

/**
 * 获取子链最新的世界状态
 * @returns {Promise<*>}
 *
 * {"chain_id":"0000000000000000000000000000000000000000000000000000000000000000","block_height":0,"hash_string":"","file_size":0}
 */
WorldState.getSubchainWsInfo = async function () {
    return await this.requestData("/wss/latest_wsinfo", "[\"\",0]")
}

/**
 * 更新世界状态配置文件
 * @param chainId
 * @param seedIp
 * @returns {boolean}
 */
WorldState.updateConfig = function (chainId, seedIp) {
    try {
        var iniFile = new IniFile(this.configFilePath + this.configFileName, Constants.encodingConstants.UTF8);
        iniFile.setValue("chainId", chainId);
        iniFile.setValue("seedIP", seedIp);
        iniFile.setValue("http-server-address", "127.0.0.1:" + this.port);
        iniFile.writefile(this.configFilePath + this.configFileName, Constants.encodingConstants.UTF8);
        return true;
    } catch (e) {
        logger.error("update wss config file error: ", e)
        return false;
    }
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
        if (await this.checkAlive() == false) {
            return true;
        }
        sleep.msleep(this.statusCheckTime);
        searchtime+=this.statusCheckTime;
    }
    return false;
    //return true;
}

/**
 * 启动
 * @type {WorldState}
 */
WorldState.start = async function (chainId, seedIp,totalTime) {

    let result = false;
    /**
     * 更新配置信息（端口号，链id，种子ip
     */
    if (!this.updateConfig(chainId, seedIp)) {
        return result;
    }
    res = await ShellCmd.execCmd(Constants.cmdConstants.START_WORLDSTATE);
    if (res) {
        console.debug("start WorldState success");
        //utils.sleep(this.statusCheckTime);
        let searchtime = this.statusCheckTime;
        while (totalTime >= searchtime) {
             if (await this.checkAlive()) {
                 result = true;
                 break;
             }
            sleep.msleep(this.statusCheckTime);
            searchtime+=this.statusCheckTime;
        }
    } else {
        console.debug("start WorldState error");
    }
    return result;
}


module.exports = WorldState;