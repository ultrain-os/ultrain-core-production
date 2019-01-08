const {U3} = require('u3.js');
const axios = require('axios')
const path = require('path');
const fs = require('fs');
const ini = require('ini');
const {createU3, format} = U3;

/**
 * 日志信息
 */
var logger = require("../config/logConfig");
var logUtil = require("../common/util/logUtil")
var constant = require("../common/constant/constants")
var utils = require("../common/util/utils");
var chainApi = require("./chainApi")

/**
 * 主侧链相关配置
 */
class ChainConfig {

}

//配置文件信息
ChainConfig.configPath = path.join(__dirname, "../../config.ini");

//localtest
ChainConfig.localtest = false;

//chain_name
ChainConfig.chain_name = "";
//节点登录的委员会用户信息
ChainConfig.myAccountAsCommittee="";
//委员会私钥
ChainConfig.mySkAsCommittee="";

//主链配置
ChainConfig.config = {
    httpEndpoint: "",
    httpEndpoint_history: "",
    keyProvider: [], // WIF string or array of keys..

    chainId: '', // 32 byte (64 char) hex string
    // chainId: '2616bfbc21e11d60d10cb798f00893c2befba10e2338b7277bb3865d2e658f58', // 32 byte (64 char) hex string
    expireInSeconds: 60,
    broadcast: true,
    sign: true,

    debug: false,
    verbose: false,
    logger: {
        log: logUtil.log,
        error: logUtil.error,
        debug: logUtil.debug
    },
    binaryen: require('binaryen'),
    symbol: 'UGAS'

};

//子链配置
ChainConfig.configSub = {
    httpEndpoint: '',
    httpEndpoint_history: '',
    keyProvider: [], // WIF string or array of keys..

    chainId: "", // 32 byte (64 char) hex string
    // chainId: '2616bfbc21e11d60d10cb798f00893c2befba10e2338b7277bb3865d2e658f58', // 32 byte (64 char) hex string
    expireInSeconds: 60,
    broadcast: true,
    sign: true,

    debug: false,
    verbose: false,
    logger: {
        log: logUtil.log,
        error: logUtil.error,
        debug: logUtil.debug
    },
    binaryen: require('binaryen'),
    symbol: 'UGAS'

};

/**
 * 根据官网数据获取主网ip地址
 * @returns {Promise<*>}
 */
async function getRemoteIpAddress(url) {
    const rs = await axios.get(url);
    return rs.data;
}


//配置同步
ChainConfig.syncConfig = async function () {

    try {
        logger.debug("start to init config file");
        /**
         * 读取管家程序自己的config文件来读取
         */
        var configIniLocal = ini.parse(fs.readFileSync(this.configPath, constant.encodingConstants.UTF8));
        logger.debug('configIniLocal=', configIniLocal);

        //读取nodultrain程序中的config.ini
        var configIniTarget = ini.parse(fs.readFileSync(configIniLocal.path, constant.encodingConstants.UTF8));
        logger.debug('configIniTarget(nodultrain)=', configIniTarget);

        //获取主链请求的http地址-默认使用
        const ip = await getRemoteIpAddress(configIniLocal.url);
        logger.debug('getRemoteIpAddress=', ip);
        this.config.httpEndpoint = `${configIniLocal.prefix}${ip}${configIniLocal.endpoint}`;

        //子链请求地址配置
        this.configSub.httpEndpoint = configIniLocal.subchainHttpEndpoint;

        /**
         * 获取配置中localtest配置
         */
        if (utils.isNotNull(configIniLocal["localtest"])) {
            this.localtest = configIniLocal["localtest"];
        }

        logger.debug("env: (localtest:" + this.localtest + ")");

        /**
         * 通过nodultrain的配置信息获取主子链的用户信息
         */
        this.myAccountAsCommittee = configIniTarget["my-account-as-committee"];
        this.mySkAsCommittee = configIniTarget["my-sk-as-committee"];
        this.config.keyProvider = [configIniTarget["my-sk-as-account"]];
        this.configSub.keyProvider = [configIniTarget["my-sk-as-account"]];

        /**
         * 如果localtest为true，表明当前是本地测试状态，更新主链url等信息
         */
        if (this.localtest) {
            if (utils.isNotNull(configIniLocal["mainchainHttpEndpoint"])) {
                logger.debug("local mainchain httpEndpoint is enabled :" + configIniLocal["mainchainHttpEndpoint"]);
                this.config.httpEndpoint = configIniLocal["mainchainHttpEndpoint"];
            }
            // config.httpEndpoint = "http://172.16.10.4:8877";
            if (utils.isNotNull(configIniLocal["chain_name"])) {
                this.chain_name = configIniLocal["chain_name"];
            }
            /**
             * 账号信息需要同时否不为空才进行替换
             */
            if (utils.isAllNotNull(configIniLocal["my-account-as-committee"], configIniLocal["my-sk-as-committee"], configIniLocal["my-sk-as-account"])) {
                logger.debug("local account config is enabled :" + configIniLocal["my-account-as-committee"]);
                this.myAccountAsCommittee = configIniLocal["my-account-as-committee"];
                this.mySkAsCommittee = configIniLocal["my-sk-as-committee"];
                this.config.keyProvider = [configIniLocal["my-sk-as-account"]];
                this.configSub.keyProvider = [configIniLocal["my-sk-as-account"]];
            }
        }

        try {
            //
            this.config.chainId = await chainApi.getMainChainId(this.config);
            logger.debug("config.chainId=", this.config.chainId);
            this.configSub.chainId = await chainApi.getSubChainId(this.configSub);
            logger.debug("configSub.chainId=", this.configSub.chainId);
        } catch (e) {
            logger.error("target node crashed, check main node or sub node", e)

        }

        logger.debug("finish init config file");

    } catch
        (e) {
        logger.error("sync chain config error: ", e);
    }
}

//打出信息
ChainConfig.printInfo = function () {
    logger.info("====chain config====");
    logger.info("localtest:" + this.localtest)
    logger.info("myAccountAsCommittee:" + this.myAccountAsCommittee)
    logger.info("mySkAsCommittee:" + this.mySkAsCommittee)

    logger.info("====chain config====");


}


module.exports = ChainConfig
