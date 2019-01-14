const {U3} = require('u3.js');
const axios = require('axios')
const path = require('path');
const fs = require('fs');
const ini = require('ini');
const {createU3, format} = U3;

/**
 * 日志信息
 */
var logger = require("../config/logConfig").getLogger("ChainConfig");
var logUtil = require("../common/util/logUtil")
var constant = require("../common/constant/constants")
var utils = require("../common/util/utils");
var chainApi = require("./chainApi")
var sleep = require("sleep")


/**
 * 主侧链相关配置
 */
class ChainConfig {

}

//seep节点配置
ChainConfig.seedIpConfig = {};

//配置文件信息
ChainConfig.configPath = path.join(__dirname, "../../config.ini");

//localtest
ChainConfig.localTest = false;

//chain_name
ChainConfig.chainName = "11";
//main chain id
ChainConfig.mainChain
//节点登录的委员会用户信息
ChainConfig.myAccountAsCommittee = "";
//委员会私钥
ChainConfig.mySkAsCommittee = "";


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
 * 主链和子链的u3对象
 */
ChainConfig.u3 = {};
ChainConfig.u3Sub = {};

/**
 * config 配置信息
 * @type {{}}
 */
ChainConfig.configFileData = {};

//配置同步
ChainConfig.syncConfig = async function () {

    try {
        logger.info("start sync config info");
        /**
         * 读取管家程序自己的config文件来读取
         */
        var configIniLocal = ini.parse(fs.readFileSync(this.configPath, constant.encodingConstants.UTF8));
        //logger.debug('configIniLocal=', configIniLocal);
        this.configFileData.local = configIniLocal;

        //读取nodultrain程序中的config.ini
        var configIniTarget = ini.parse(fs.readFileSync(configIniLocal.path, constant.encodingConstants.UTF8));
        //logger.debug('configIniTarget(nodultrain)=', configIniTarget);
        this.configFileData.target = configIniTarget;

        //logger.debug("this.configFileData data:", this.configFileData);

        //获取主链请求的http地址-默认使用
        const ip = await chainApi.getRemoteIpAddress(configIniLocal.url);
        logger.debug('getRemoteIpAddress=', ip);
        this.config.httpEndpoint = `${configIniLocal.prefix}${ip}${configIniLocal.endpoint}`;

        //子链请求地址配置
        this.configSub.httpEndpoint = configIniLocal.subchainHttpEndpoint;

        /**
         * 获取配置中localtest配置
         */
        if (utils.isNotNull(configIniLocal["localtest"])) {
            this.localTest = configIniLocal["localtest"];
        }

        logger.info("env: (localtest:" + this.localTest + ")");

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
        if (this.localTest) {
            if (utils.isNotNull(configIniLocal["mainchainHttpEndpoint"])) {
                logger.debug("local mainchain httpEndpoint is enabled :" + configIniLocal["mainchainHttpEndpoint"]);
                this.config.httpEndpoint = configIniLocal["mainchainHttpEndpoint"];
            }
            // config.httpEndpoint = "http://172.16.10.4:8877";
            if (utils.isNotNull(configIniLocal["chain_name"])) {
                this.chainName = configIniLocal["chain_name"];
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

        logger.info("user info : " + this.myAccountAsCommittee + "");

        try {
            //
            this.config.chainId = await chainApi.getMainChainId(this.config);
            logger.debug("config.chainId=", this.config.chainId);
            this.configSub.chainId = await chainApi.getSubChainId(this.configSub);
            logger.debug("configSub.chainId=", this.configSub.chainId);
        } catch (e) {
            logger.error("target node crashed, check main node or sub node", e)

        }

        logger.debug("init u3 and u3Sub from config");
        this.u3 = createU3({...this.config, sign: true, broadcast: true});
        this.u3Sub = createU3({...this.configSub, sign: true, broadcast: true});

        logger.info("finish sync config info");

        return true;

    } catch
        (e) {
        logger.error("sync chain config error: ", e);
    }

    return false;
}

/**
 * 检查链的配置已经初始化成功了
 */
ChainConfig.isReady = function () {

    //链信息查询
    if (utils.isNull(this.chainName)) {
        logger.error("chainconfig chain info is null");
        return false;
    }

    //校验主子链的id
    if (!utils.isAllNotNull(this.config.chainId, this.configSub.chainId)) {
        logger.error("chainconfig chainId & subChainId  is null");
        return false;
    }

    //用户信息为空
    if (!utils.isAllNotNull(this.myAccountAsCommittee, this.mySkAsCommittee)) {
        logger.error("chainconfig user account is null");
        return false;
    }

    /**
     * u3 object
     */
    if (!utils.isAllNotNull(this.u3, this.u3Sub)) {
        logger.error("chainconfig u3 && u3Sub is not ready");
        return false;
    }

    return true;
}

/**
 * 轮询保证配置已经同步了
 * @returns {Promise<void>}
 */
ChainConfig.waitSyncConfig = async function () {
    await this.syncConfig();
    while (!this.isReady()) {
        sleep.msleep(1000 * 5);
        await this.syncConfig();
        logger.info("config is not ready ,wait to next check...")
    }
}

//打出信息
ChainConfig.printInfo = function () {
    logger.info("====chain config====");
    logger.info("localtest:" + this.localTest)
    logger.info("myAccountAsCommittee:" + this.myAccountAsCommittee)
    logger.info("mySkAsCommittee:" + this.mySkAsCommittee)

    logger.info("====chain config====");
}

//同步seedip config
ChainConfig.syncSeedIpConfig = function () {
    try {
        var data = fs.readFileSync(path.join(__dirname, "../../seedconfig.json"), constant.encodingConstants.UTF8);
        if (utils.isNotNull(data)) {
            this.seedIpConfig = JSON.parse(data);
            //logger.debug("seedIpConfig:", this.seedIpConfig);
        }
    } catch (e) {
        logger.error("syncSeedIpConfig error", e);
    }

}

ChainConfig.syncSeedIpConfig();


module.exports = ChainConfig