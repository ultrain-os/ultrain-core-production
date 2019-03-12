const {U3} = require('u3.js');
const fs = require('fs');
var logger = require("../config/logConfig").getLogger("Monitor");
var chainConfig = require("./chainConfig")
var chainApi = require("./chainApi")
var constants = require("../common/constant/constants")
var cacheKeyConstants = require("../common/constant/constants").cacheKeyConstants
var filenameConstants = require("../common/constant/constants").filenameConstants
var iniConstants = require("../common/constant/constants").iniConstants
var pathConstants = require("../common/constant/constants").pathConstants
var algorithmConstants = require("../common/constant/constants").algorithmConstants
var statusConstants = require("../common/constant/constants").statusConstants
var utils = require("../common/util/utils")
var CacheObj = require("../common/cache/cacheObj");
var hashUtil = require("../common/util/hashUtil")
var download = require('download-to-file')
var ShellCmd = require("../common/util/shellCmd")
var NodUltrain = require("../nodultrain/nodultrain")
var sleep = require("sleep")
var process = require('child_process');


var hashCache = new CacheObj(false, null);
/**
 * hash缓存过期时间（1分钟）
 * @type {number}
 */
var HASH_EXPIRE_TIME_MS = 1000 * 60;

var deploySyncFlag = false;

var seedCount = 10;

var enableRestart = 1;

/**
 *
 * @returns {string}
 */
async function getNodVersion() {
    let hashFile = hashCache.get(cacheKeyConstants.NOD_FILE_KEY);
    if (utils.isNull(hashFile)) {
        logger.info("cache not hit :" + cacheKeyConstants.NOD_FILE_KEY);
        let nodFilePath = chainConfig.configFileData.local.nodpath + "/" + filenameConstants.NOD_EXE_FILE;
        hashFile = hashUtil.calcHash(nodFilePath, algorithmConstants.SHA1);
        if (utils.isNotNull(hashFile)) {
            hashCache.put(cacheKeyConstants.NOD_FILE_KEY, hashFile, HASH_EXPIRE_TIME_MS);
            logger.info("cache info :", hashCache.getAll());
        }
    } else {
        logger.info("cache hit :" + cacheKeyConstants.NOD_FILE_KEY, hashFile);
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
        logger.info("cache not hit :" + cacheKeyConstants.MNG_FILE_KEY);
        let nodFilePath = chainConfig.configFileData.local.mngpath + "/" + filenameConstants.MNG_FILE;
        hashFile = hashUtil.calcHash(nodFilePath, algorithmConstants.SHA1);
        if (utils.isNotNull(hashFile)) {
            hashCache.put(cacheKeyConstants.MNG_FILE_KEY, hashFile, HASH_EXPIRE_TIME_MS);
        }
    } else {
        logger.info("cache hit :" + cacheKeyConstants.MNG_FILE_KEY, hashFile);
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
 *
 * @returns {Promise<{mngVersion, ipLocal: (*|string), chainId: string, ipPublic: *, nodVersion, user: string, isProducer: number}>}
 */
async function buildParam() {

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

    var isProducer = 1;
    if (chainConfig.isNoneProducer()) {
        isProducer = 0;
    }

    var extMsg = {
        "enableRestart": enableRestart
    }
    var param = {
        "chainId": chainConfig.localChainName,
        "ipLocal": utils.getLocalIPAdress(),
        "ipPublic": publicip,
        "user": user,
        "nodVersion": nodFileHash,
        "mngVersion": mngFileHash,
        "isProducer": isProducer,
        "time": new Date().getTime(),
        "ext" : JSON.stringify(extMsg),
    };

    return param;
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

    let param = await buildParam();

    param.sign = generateSign(param.time,generateSignParam(param));

    await chainApi.monitorCheckIn(getMonitorUrl(), param);

    logger.info("monitor checkin end");

    await getDeployFile();

    //await syncSeedInfo();
}

/**
 *
 * @returns {Promise<void>}
 */
async function syncSeedInfo() {

    if (checkNeedSync() == false) {
        return
    }

    logger.info("syncSeedInfo start,seedCount:"+seedCount);

    if (seedCount < 10) {
        seedCount++;
        return;
    }

    //reset
    seedCount = 1;


    let deployInfo = await chainApi.getSeedInfo(getMonitorUrl(), {});
    if (chainApi.verifySign(deployInfo) == true) {
        let data = deployInfo.data;

        if (utils.isNullList(data)) {
            logger.error("seed info is null, need not update");
        }

        if (JSON.stringify(data) == JSON.stringify(chainConfig.seedIpConfig)) {
            logger.info("seed info is equal, need not update");
            return;
        }

        var filepath = pathConstants.MNG_CONFIG+"seedconfig.json";
        chainConfig.seedIpConfig = data;
        fs.writeFile(filepath, JSON.stringify(data), {flag: 'w'}, function (err) {
            if(err) {
                logger.error("write seed config file error:");
            } else {
                logger.info("write seed config file success:");
            }


        });
    }  else {
        logger.error("syncSeedInfo sign error");
    }


}

/**
 * deploy filehh
 * @returns {Promise<void>}
 */
async function getDeployFile() {

    if (checkNeedSync() == false) {
        return
    }

    // if (chain.isChainChanging() == true) {
    //     return;
    // }

    if (deploySyncFlag == true) {
        return;
    }


    logger.info("getDeployFile start");
    let param = await buildParam();
    param.sign = generateSign(param.time,generateSignParam(param));
    let deployInfo = await chainApi.checkDeployFile(getMonitorUrl(), param);

    logger.info("get deploy info:", deployInfo);

    if (chainApi.verifySign(deployInfo) == false) {
        logger.error("get deploy(check sign) error:");
    }

    /**
     *
     */
    if (utils.isNotNull(deployInfo.data)) {
        logger.info("receive deploy info:", deployInfo.data);
        //TODO check sign
        if (signCheck(deployInfo.data) == true) {
            //文件部署
            if (deployInfo.data.type == 0) {
                await fileDeploy(deployInfo.data.deployBatch);
            }

            //命令部署
            if (deployInfo.data.type == 1) {
                await cmdDeploy(deployInfo.data.deployBatch);
            }

        } else {
            logger.error("invalid file update,undo");
        }
    }
}

async function cmdDeploy(deployBatch) {
    deploySyncFlag = true;
    try {
        var deployCmd = JSON.parse(deployBatch.deployInfo);
        logger.debug("deployCmd:", deployCmd);

        logger.info("start to deploy cmd (" + deployCmd.name +  ")");
        logger.info("start to exe cmd (" + deployCmd.content +  ")");

        //系统命令
        let systemCmd = false;

        //设置重启-1
        if (deployCmd.content == constants.cmdConstants.ENABLE_RESTART) {
            enableRestart = 1;
            systemCmd = true;
        }

        //设置重启-0
        if (deployCmd.content == constants.cmdConstants.DISABLE_RESTART) {
            enableRestart = 0;
            systemCmd = true;
        }

        if (systemCmd == true) {
            let param = await buildParam();
            param.batchId = deployBatch.id;
            param.status = statusConstants.SUCCESS;
            param.sign = generateSign(param.time,generateSignParamWithStatus(param));
            param.ext = "success";
            await chainApi.finsihDeployFile(getMonitorUrl(), param);
            enableDeploy();
            return;
        }

        //其它命令
        process.exec(deployCmd.content, async function (error, stdout, stderr, finish) {

            let param = await buildParam();
            param.batchId = deployBatch.id;
            logger.info("finsih deploy param:",param);
            if (error !== null) {
                logger.error('exec error: ' + error);
                param.status = statusConstants.EXCEPTION;
                param.ext = error.toString();
                await chainApi.finsihDeployFile(getMonitorUrl(), param);
                param.sign = generateSign(param.time,generateSignParamWithStatus(param));
                enableDeploy();
            } else {
                logger.info("exec success :",stdout);
                param.status = statusConstants.SUCCESS;
                if (utils.isNotNull(stdout) && stdout != undefined) {
                    param.ext = stdout.toString();
                }
                param.sign = generateSign(param.time,generateSignParamWithStatus(param));
                await chainApi.finsihDeployFile(getMonitorUrl(), param);
                enableDeploy();
            }

        });

    } catch (e) {
        logger.error("cmd deplpy error,",e);
        deploySyncFlag = false;
    }


}

/**
 *
 * @returns {boolean}
 */
function signCheck() {
    return true;
}

/**
 *
 * @param filename
 * @returns {Promise<string>}
 */
async function getLocalHash(filename) {

    //nod 本地 hash
    if (filenameConstants.NOD_EXE_FILE == filename) {
        return await getNodVersion();
    }

    if (filenameConstants.MNG_FILE == filename) {
        return await getMngVersion();
    }

    return "";
}



function getTargetPath(filename) {
    //nod 本地 hash
    if (filenameConstants.NOD_EXE_FILE == filename) {
        return chainConfig.configFileData.local.nodpath + "/" + filename;
    }

    if (filenameConstants.MNG_FILE == filename) {
        return chainConfig.configFileData.local.mngpath + "/" + filename;
    }

    return "";
}

async function fileDeploy(deployBatch) {
    deploySyncFlag = true;

    try {
        var deployFile = JSON.parse(deployBatch.deployInfo);
        logger.debug("deployFile:", deployFile);

        logger.info("start to deploy file (" + deployFile.filename + ")");

        let localHash = await getLocalHash(deployFile.filename);
        if (localHash == deployFile.hash) {
            logger.info("localfile hash(" + localHash + ") is equal deploy hash(" + deployFile.hash + ") ,need not download");
            deploySyncFlag = false;

            let param = await buildParam();
            param.status = statusConstants.SUCCESS;
            param.batchId = deployBatch.id;
            logger.info("finsih deploy param:",param);
            param.data = generateSignParamWithStatus(param);
            await chainApi.finsihDeployFile(getMonitorUrl(), param);
            return;
        } else {
            logger.info("localfile hash(" + localHash + ") is not  equal deploy hash(" + deployFile.hash + ") ,need download");
        }

        let localpath = getDownloadFilePath(deployFile.filename, deployFile.hash);

        if (fs.existsSync(localpath)) {
            logger.info("file has already downloaded:" + localpath);

            if (filenameConstants.NOD_EXE_FILE == deployFile.filename) {
                await fileProcessNod(deployFile, localpath);
            }

            if (filenameConstants.MNG_FILE == deployFile.filename) {
                await fileProcessMng(deployFile, localpath);
            }

        } else {
            logger.info("file is not ready, need to downloaded:" + localpath);
            download(deployFile.url, localpath, function (err, filepath) {
                if (err) {
                    logger.error("download file error:" + deployFile.url, err);
                    deploySyncFlag = false;
                } else {
                    logger.info("download successfully:" + deployFile.url);
                    console.log('Download finished:', filepath);
                    deploySyncFlag = false;
                }
            })
        }

    } catch (e) {
        logger.error("fileDeploy error", e);
        deploySyncFlag = false;
    }
}

/**
 *
 * @param deployFile
 * @param localpath
 * @returns {Promise<void>}
 */
async function fileProcessMng(deployFile, localpath) {

    try {
        let hash = hashUtil.calcHash(localpath, algorithmConstants.SHA1);
        if (hash == deployFile.hash) {
            logger.info("download file(" + hash + ") equals server info(" + deployFile.hash + ")");
            logger.info("start to update mng file");
            let targetPath = getTargetPath(deployFile.filename);
            logger.info("need to update target path :" + targetPath);

            var cmd = "cp " + localpath + " " + targetPath + " -f";
            process.exec(cmd, async function (error, stdout, stderr, finish) {
                if (error !== null) {
                    logger.error('exec error: ' + error);
                    enableDeploy();
                } else {
                    logger.info("exec success :",cmd);
                    cmd = "pm2 restart sideChainService";
                    process.exec(cmd, async function (error, stdout, stderr, finish,cmd) {
                        if (error !== null) {
                            logger.error('exec error: ' + error);
                            enableDeploy();
                        } else {
                            logger.info("exec success :",cmd);
                            enableDeploy();
                        }
                    });

                }

            });
        } else {
            enableDeploy()
        }
    } catch (e) {
        logger.error("fileProcessMng error,",e);
        enableDeploy()
    }

}

/**
 *
 * @param deployFile
 * @param localpath
 * @returns {Promise<void>}
 */
async function fileProcessNod(deployFile, localpath) {
    try {
        let hash = hashUtil.calcHash(localpath, algorithmConstants.SHA1);
        if (hash == deployFile.hash) {
            logger.info("download file(" + hash + ") equals server info(" + deployFile.hash + ")");
            logger.info("start to stop nod before update file");
            sleep.msleep(1000);
            result = await NodUltrain.stop(1200000);
            if (result == true) {
                logger.info("nod stop successfully,start to update file");
                let targetPath = getTargetPath(deployFile.filename);
                logger.info("need to update target path :" + targetPath);
                result = await NodUltrain.stop(1200000);

                if (result == true) {
                    let cmd = "cp " + localpath + " " + targetPath + " -f";
                    process.exec(cmd, async function (error, stdout, stderr, finish) {
                        if (error !== null) {
                            logger.error('exec error: ' + error);
                            enableDeploy();
                        } else {
                            hashCache.clear();
                            logger.info("exe success: " + cmd);
                            cmd = "chmod a+x " + targetPath;
                            process.exec(cmd, async function (error, stdout, stderr, finish) {
                                if (error !== null) {
                                    logger.error('exec error: ' + error);
                                    enableDeploy();
                                } else {
                                    logger.info("exccmd success:" + cmd);
                                    result = await NodUltrain.start(600000, chainConfig.configFileData.local.nodpath, " ", chainConfig.localTest);
                                    if (result == true) {
                                        logger.info("nod start success")
                                    } else {
                                        logger.error("node start error");
                                    }
                                    enableDeploy();
                                }
                            });
                        }
                    });
                }
            } else {
                logger.error("nod stop failed,can't update file");
                enableDeploy()
            }

        } else {
            logger.error("download file(" + hash + ") not equals server info(" + deployFile.hash + "),need remove download file");
            await ShellCmd.execCmd("rm " + localpath);
            sleep.msleep(3000);
            enableDeploy()
        }

    } catch (e) {
        logger.error("fileProcessNod error:", e);
        enableDeploy()
    }


}

/**
 *
 * @param type
 * @param filename
 * @param hash
 * @returns {string}
 */
function getDownloadFilePath(filename, hash) {
    let filePath = pathConstants.FILE_DOWNLOAD_PATH + hash + "/" + filename;
    return filePath;
}

function isDeploying() {
    return deploySyncFlag;
}

function disableDeploy() {
    deploySyncFlag = true;
}

function enableDeploy() {
    deploySyncFlag = false;
}

function generateSignParam(param) {
    let data = "";
    try {
        data = "chainId="+param.chainId+"&user="+param.user+"&nodVersion="+param.nodVersion+"&mngVersion="+param.mngVersion+"&isProducer="+param.isProducer;
    } catch (e) {
        logger.error("generate sign error,",e);
    }

    return data;
}


function generateSignParamWithStatus(param) {
    let data = "";
    try {
        data = generateSignParam(param);
        data = data+"&status="+param.status;
    } catch (e) {
        logger.error("generate sign error,",e);
    }

    return data;
}

function generateSign(time,data) {
    let result = data+"&time="+time+"&key="+constants.PRIVATE_KEY;
    logger.info("result:"+result);
    let sign = hashUtil.calcMd5(result);
    return sign;
}

/**
 *
 * @returns {boolean}
 */
function needCheckNod() {
    if (enableRestart == 1) {
        return true;
    }

    return false;
}

module.exports = {
    checkIn,
    isDeploying,
    disableDeploy,
    enableDeploy,
    buildParam,
    getMonitorUrl,
    generateSignParamWithStatus,
    generateSign,
    needCheckNod,
}
