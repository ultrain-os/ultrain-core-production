const http = require('http');
var querystring = require('querystring');
const axios = require('axios')
var logger = require("../config/logConfig");
var IniFile = require('../common/util/iniFile');
var Constants = require('../common/constant/constants');

/**
 * 世界状态通信对象
 */
class WSS {
}

//wss提供的请求地址版本
WSS.port = 7777
WSS.hostname = "127.0.0.1"
// WSS.port = 8888
// WSS.hostname = "172.16.10.5"
WSS.http = "http"
WSS.version = "v1"

//配置文件目录与文件名
WSS.configFilePath = "/root/.local/share/ultrainio/wssultrain/config/"
WSS.configFileName = "config.ini"

/**
 * 获取 http请求时的options
 * @param path
 * @returns {{path: string, hostname: string, method: string, port: number}}
 */
WSS.getHttpRequestPath = function (path) {
    return this.http + "://" + this.hostname + ":" + this.port + "/" + this.version + path;
}

/**
 * 调用url获取请求
 * @param path
 * @param params
 * @returns {Promise<*>}
 */
WSS.requestData = async function (path, params) {
    try {
        const rs = await axios.post(this.getHttpRequestPath(path), params);
        if (rs.status == 200) {
            return rs.data;
        } else {
            logger.error("request wss service error:" + rs);
            console.log(rs);
            return null;
        }
    } catch (e) {
        logger.error("request wss service error:", e);
    }
}

/**
 * 同步世界状态
 * @param hash
 * @param height
 * @returns {Promise<*>}
 */
WSS.syncWorldstate = async function (hash, height) {
    return await this.requestData("/wss/require_ws", {hash: hash, height: height});
}

/**
 * 轮询同步时间状态
 * @returns {Promise<*>}
 */
WSS.pollingWSState = async function () {
    return await this.requestData("/wss/ws_status", "[\"ws\"]");
}

/**
 * 同步块信息
 * @param begin
 * @param end
 * @returns {Promise<*>}
 */
WSS.syncBlocks = async function (begin, end) {
    return await this.requestData("/wss/require_block", {begin: begin, end: end})
}

/**
 * 轮询同步块状态
 * @returns {Promise<*>}
 */
WSS.pollingBlockState = async function () {
    return await this.requestData("/wss/ws_status", "[\"block\"]");
}


/**
 * 检查ws是否是正常状态
 * @returns {Promise<*>}
 */
WSS.checkAlive = async function () {
    let res =  await this.requestData("/wss/ws_status", "[\"\",1]");
    logger.error("ws status:"+res);
}

/**
 * 更新世界状态配置文件
 * @param chainId
 * @param seedIp
 * @returns {boolean}
 */
WSS.updateConfig = function (chainId, seedIp) {
    try {
        var iniFile = new IniFile(this.configFilePath + this.configFileName, Constants.encodingConstants.UTF8);
        iniFile.setValue("chainId", chainId);
        iniFile.setValue("seedIP", seedIp);
        iniFile.setValue("http-server-address","127.0.0.1:"+this.port);
        iniFile.writefile(this.configFilePath + this.configFileName, Constants.encodingConstants.UTF8);
        return true;
    } catch (e) {
        logger.error("update wss config file error: ", e)
        return false;
    }
}


module.exports = WSS;