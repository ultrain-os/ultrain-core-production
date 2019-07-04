const http = require('http');
var querystring = require('querystring');
const axios = require('axios')
var logger = require("../config/logConfig").getLogger("NodUltrain");
var IniFile = require('../common/util/iniFile');
var Constants = require('../common/constant/constants');
var iniConstants = require('../common/constant/constants').iniConstants;
var ShellCmd = require("../common/util/shellCmd")
var sleep = require("sleep")
var utils = require("../common/util/utils")
const path = require('path');
var hashUtil = require("../common/util/hashUtil")
var algorithmConstants = require("../common/constant/constants").algorithmConstants
var process = require('child_process');
const fs = require('fs');
var os = require('os');

/**
 * nod相关交互类
 */
class NodUltrain {
}

NodUltrain.statusCheckTime = 500;

NodUltrain.configFilePath = utils.formatHomePath("~/.local/share/ultrainio/nodultrain/config/config.ini");

NodUltrain.checkConfigInfo = function(updateArr) {
    try {
        let ready = true;
        var iniFile = new IniFile(this.configFilePath, Constants.encodingConstants.UTF8);
        logger.info("add nod config, updateArr:",updateArr);
        if (updateArr.length > 0) {
            for (let t = 0;t<updateArr.length;t++) {
                let kv = updateArr[t];
                logger.info("check add nod confg info:",kv);
                let kvArray = kv.split("=");
                if (kvArray.length == 2) {
                    let key = kvArray[0];
                    let value = kvArray[1];
                    if (iniFile.checkKVExist(key,value) == false) {
                        return false;
                    }
                }
            }
        } else {
            logger.error("add nod config, updateArr is null:",updateArr);
        }

        return ready;

    } catch (e) {
        logger.error("getConfigInfo error,",e);
    }
}

/**
 *
 * @param updateArr
 */
NodUltrain.updateConfigInfo = function(updateArr) {
    try {
        let updateFlag = false;
        var iniFile = new IniFile(this.configFilePath, Constants.encodingConstants.UTF8);
        if (updateArr.length > 0) {
            for (let t = 0;t<updateArr.length;t++) {
                let kv = updateArr[t];
                logger.info("check add nod confg info:",kv);
                let kvArray = kv.split("=");
                if (kvArray.length == 2) {
                    let key = kvArray[0].trim();
                    let value = kvArray[1].trim();
                    if (iniFile.checkKVExist(key,value) == false) {
                        logger.info("kv("+key+":"+value+") is not exist,need add");
                        iniFile.setValue(key,value);
                        updateFlag = true;
                    } else {
                        logger.info("kv("+key+":"+value+") is exist,need not add");
                    }
                }
            }

            if (updateFlag == true) {
                iniFile.writefile(this.configFilePath, Constants.encodingConstants.UTF8);
                return true;
            }
        }


    } catch (e) {
        logger.error("addConfigInfo error,",e);

    }

    return false;
}
/**
 *
 * @param data
 * @returns {boolean}
 */
NodUltrain.addConfigInfo = function(updateArr) {
    try {
        let updateFlag = false;
        var iniFile = new IniFile(this.configFilePath, Constants.encodingConstants.UTF8);
        if (updateArr.length > 0) {
            for (let t = 0;t<updateArr.length;t++) {
                let kv = updateArr[t];
                logger.info("check add nod confg info:",kv);
                let kvArray = kv.split("=");
                if (kvArray.length == 2) {
                    let key = kvArray[0];
                    let value = kvArray[1];
                    if (iniFile.checkKVExist(key,value) == false) {
                        logger.info("kv("+key+":"+value+") is not exist,need add");
                        iniFile.addKeyValue(key,value);
                        updateFlag = true;
                    } else {
                        logger.info("kv("+key+":"+value+") is exist,need not add");
                    }
                }
            }

            if (updateFlag == true) {
                iniFile.writefile(this.configFilePath, Constants.encodingConstants.UTF8);
                return true;
            }
        }


    } catch (e) {
        logger.error("addConfigInfo error,",e);

    }

    return false;
}

/**
 * 更新nod配置文件
 * @param filepath
 * @param monitorServer
 * @returns {boolean}
 */
NodUltrain.updateConfig = function (seedIp,subchainHttpEndpoint,genesisTime,genesisPK,monitorServcer,chainPeerInfo,chainName) {
    try {
        var iniFile = new IniFile(this.configFilePath, Constants.encodingConstants.UTF8);

        // if (utils.isNotNull(monitorServcer)) {
        //     iniFile.setValue(iniConstants.MONITOR_SERVER_ENDPOINT, monitorServcer);
        // }

        if (utils.isNotNull(iniFile.getValue(iniConstants.UDP_SEED))) {
            logger.info("nod has udp config,set udp seed");
            iniFile.removeKey(iniConstants.UDP_SEED);
            for (var i=0;i<seedIp.length;i++) {
                iniFile.addKeyValue(iniConstants.UDP_SEED, seedIp[i]);
            }
        }

        if (utils.isNotNull(iniFile.getValue(iniConstants.P2P_PEER_ADDRESS))) {
            logger.info("nod has p2p-peer-address,set up tcp config");
            iniFile.removeKey(iniConstants.P2P_PEER_ADDRESS)
            iniFile.removeKey(iniConstants.RPOS_P2P_PEER_ADDRESS)
            for (var i=0;i<seedIp.length;i++) {
                iniFile.addKeyValue(iniConstants.P2P_PEER_ADDRESS, seedIp[i]+":20122");
                iniFile.addKeyValue(iniConstants.RPOS_P2P_PEER_ADDRESS, seedIp[i]+":20123");
            }
        }

        //更新peer info
        iniFile.removeKey(iniConstants.PEER_KEY);
        for (var i=0;i<chainPeerInfo.length;i++) {
            iniFile.addKeyValue(iniConstants.PEER_KEY, chainPeerInfo[i]);
        }

        //更新chainname
        iniFile.setValue(iniConstants.CHAIN_NAME,chainName);

        //更新genesis——pk
        iniFile.removeKey(iniConstants.GENESIS_PK);
        logger.error("genesis-pk is:",genesisPK);
        if (utils.isNotNull(genesisPK)) {
            iniFile.addKeyValue(iniConstants.GENESIS_PK,genesisPK);
        }

        //iniFile.setValue(iniConstants.SUBCHAIN_HTTP_ENDPOINT, subchainHttpEndpoint);
        iniFile.setValue(iniConstants.GENESIS_TIME, genesisTime);
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
NodUltrain.stop = async function (totalTime,port) {
    let res = await ShellCmd.execCmd(Constants.cmdConstants.KILL_NODULTRAIN);
    sleep.msleep(this.statusCheckTime);
    //校验端口是否不提供服务
    let searchtime = this.statusCheckTime;
    while (totalTime >= searchtime) {
        let rs = await this.checkAlive(port);
        if (utils.isNull(rs)) {
            return true;
        }
        sleep.msleep(this.statusCheckTime);
        searchtime += this.statusCheckTime;
    }
    return false;
}


/**
 *
 * @param totalTime
 * @param cmdFlag true代表命令执行，false代表文件执行
 * @param nodPath nod执行程序的目录
 * @returns {Promise<boolean>}
 */
NodUltrain.start = async function (totalTime,nodPath,wssinfo,local,port) {

    var shPath= path.join(__dirname, "../../tool/_runultrain.sh");
    if (local == false) {
        shPath = Constants.cmdConstants.START_NODULTRAIN_FILE;
    }
    logger.info("start nod by shell file..",shPath)
    logger.info("start nod by shell file..",nodPath)
    logger.info("start nod by shell file..",wssinfo)
    let args= [];
    args.push(nodPath);
    args.push(wssinfo);
    await ShellCmd.execCmdFiles(shPath, args, null);
    sleep.msleep(this.statusCheckTime);
    //校验端口是否不提供服务
    let searchtime = this.statusCheckTime;
    while (totalTime >= searchtime) {
        let rs = await this.checkAlive(port);
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
NodUltrain.checkAlive = async function (port) {

    let path = "http://127.0.0.1:"+port+"/v1/chain_info/get_chain_info"
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

/**
 * 更新执行程序
 * @returns {Promise<void>}
 */
NodUltrain.updateExeFile = async function (localpath,targetpath,hashFile,port) {
    let result = false;
    try {
        result = this.stop(1200000,port);
        if (result == false) {
            logger.error("stop nod error,can't update file");
            return false;
        } else {
            let cmd = "cp " + localpath + " " + targetpath;
            sleep.msleep(2000);
            logger.error("stop nod success,start to update file: " + cmd);
            await ShellCmd.execCmd(cmd);
            sleep.msleep(5000);
            let i = 0;
            while (i <=10) {
                let sha = hashUtil.calcHash(targetpath,algorithmConstants.SHA1);
                if (sha == hashFile) {
                    logger.info("target file("+targetpath+") hash("+sha+") ==" + hashFile);
                    return true;
                }
                await ShellCmd.execCmd(cmd);
                sleep.msleep(2000);
                i++;
            }

        }
    } catch (e) {
        logger.error("updateExeFile error:",e);
    }

    return false;
}

NodUltrain.getNewestLog = function(logDir,callback) {
    try {

        let files=fs.readdirSync(logDir);

        os.hostname()
        let command = "ls -lt "+logDir+" | grep -v mng | grep -v wss | grep -v ws | grep -v total | grep "+os.hostname()+" | head -n 1 | awk '{print $NF}'";

        process.exec(command, function (error, stdout, stderr, finish) {
            if (error) {
                logger.error("getNewestLog error:",error);
            } else {
                let fileName = stdout;
                let filePath = logDir+"/"+fileName;
                logger.info("get nod filenpath:",filePath);
                let cmd = "tail -n 30 "+filePath;

                process.exec(cmd, function (error, stdout, stderr, finish) {
                    if (error) {
                        logger.error("getNewestLog error:",error);
                    } else {
                        logger.debug("nod log :",stdout);
                        callback(stdout);
                    }
                });

            }
        });
    } catch (e) {
        logger.error("getNewestLog errpr",e);
    }
}


module.exports = NodUltrain;
