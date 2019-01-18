const http = require('http');
var querystring = require('querystring');
const axios = require('axios')
var logger = require("../config/logConfig").getLogger("NodUltrain");
var IniFile = require('../common/util/iniFile');
var Constants = require('../common/constant/constants');
var ShellCmd = require("../common/util/shellCmd")
var sleep = require("sleep")
var utils = require("../common/util/utils")

/**
 * nod相关交互类
 */
class NodUltrain {
}

NodUltrain.statusCheckTime = 500;

NodUltrain.configFilePath = "/root/.local/share/ultrainio/nodultrain/config/config.ini";

/**
 * 更新nod配置文件
 * @param filepath
 * @param monitorServer
 * @returns {boolean}
 */
NodUltrain.updateConfig = function (seedIp,subchainHttpEndpoint,genesisTime) {
    try {
        var iniFile = new IniFile(this.configFilePath, Constants.encodingConstants.UTF8);
        iniFile.setValue("p2p-peer-address", seedIp+":20122");
        iniFile.setValue("rpos-p2p-peer-address", seedIp+":20123");
        iniFile.setValue("subchainHttpEndpoint", subchainHttpEndpoint);
        //todo 使用真实时间
        iniFile.setValue("genesis-time", genesisTime);
        iniFile.writefile(this.configFilePath, Constants.encodingConstants.UTF8);
        return true;
    } catch (e) {
        logger.error("update nodultrain config file error: ", e)
        return false;
    }
}

/**
 * 关闭nod
 * @returns {Promise<boolean>}
 */
NodUltrain.stop = async function (totalTime) {
    let res = await ShellCmd.execCmd(Constants.cmdConstants.KILL_NODULTRAIN);
    sleep.msleep(this.statusCheckTime);
    //校验端口是否不提供服务
    let searchtime = this.statusCheckTime;
    while (totalTime >= searchtime) {
        let rs = await this.checkAlive();
        if (utils.isNull(rs)) {
            return true;
        }
        sleep.msleep(this.statusCheckTime);
        searchtime += this.statusCheckTime;
    }
    return false;
}

/**
 * 启动nod
 * @returns {Promise<boolean>}
 */
NodUltrain.start = async function (totalTime) {
    await ShellCmd.execCmdFiles(Constants.cmdConstants.START_NODULTRAIN_FILE,Constants.cmdConstants.START_NODULTRAIN_ARG,null);
    sleep.msleep(this.statusCheckTime);
    //校验端口是否不提供服务
    let searchtime = this.statusCheckTime;
    while (totalTime >= searchtime) {
        let rs = await this.checkAlive();
        if (utils.isNotNull(rs)) {
            logger.info("nod is runing..",rs)
            return true;
        }
        sleep.msleep(this.statusCheckTime);
        searchtime += this.statusCheckTime;
    }
    return false;
}

//检查是否已启动
NodUltrain.checkAlive = async function () {

    let path = "http://127.0.0.1:8888/v1/chain/get_chain_info"
    try {
        logger.debug("send http request :"+path);
        const rs = await axios.post(path);
        logger.debug("get get_chain_info  ",rs.data);
        if (rs.status == 200) {
            return rs.data;
        } else {
            logger.error("request node service error code (" + path + ")");
        }
    } catch (e) {
        logger.error("request node service error (" + path + ") :",utils.logNetworkError(e));
    }

    return null;
}

//删除数据
NodUltrain.removeData = async function () {
    try {
        await ShellCmd.execCmd(Constants.cmdConstants.CLEAR_BLOCK_DATA);
        await ShellCmd.execCmd(Constants.cmdConstants.CLEAR_SHARED_MEMORY_DATA);
    } catch (e) {
        logger.error("nod remove data error:",e);
    }

}


module.exports = NodUltrain;