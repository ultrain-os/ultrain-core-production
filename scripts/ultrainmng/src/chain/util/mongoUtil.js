var logger = require("../../config/logConfig").getLogger("MongoUtil")
var utils = require('../../common/util/utils')
var process = require('child_process');
const fs = require('fs');
var sleep = require("sleep");
var constant = require("../../common/constant/constants")

/**
 *
 * @returns {Promise<void>}
 */
async function getLocalMongoMaxBlock(maxTime,mongoPath,mongoDBPath) {

    let blockInfo = {
        code: -1,
        msg : "error",
        block_num : -1,
    };
    let  finishFlag = false;

    let starttime =0;

    try {

        logger.info("mongoPath :",mongoPath);
        logger.info("mongoDBPath :",mongoDBPath);

        let logfilePath = "/tmp/mongo-repair-"+new Date().getTime()+".log";
        let cmd = "python ~/mongo_process.py "+mongoPath+"/bin "+mongoDBPath+" "+logfilePath;
        logger.info("mongo getLocalMongoMaxBlock cmd:",cmd);

        /**
         * 发送执行
         */
        process.exec(cmd, async function (error, stdout, stderr, finish) {
            if (error) {
                logger.error("echoCmd(" + cmd + ") error:", error);
            } else {
                logger.info("echoCmd(" + cmd + ") success:");
            }
        });

        /**
         * 循环遍历文件是否ready
         */
        let nullCount =0;
        while (starttime < maxTime && finishFlag == false) {
            logger.info("getLocalMongoMaxBlock check, now("+starttime+") total("+maxTime+")");
            sleep.msleep(1000);
            starttime = starttime + 1000;

            if (fs.existsSync(logfilePath)) {
                var logdata = fs.readFileSync(logfilePath, constant.encodingConstants.UTF8);
                logger.info("logdata:",logdata);
                if (utils.isNotNull(logdata)) {
                    try {
                        let obj = JSON.parse(logdata);
                        logger.info("mongo_process porcess obj",obj);
                        logger.info("mongo_process porcess obj code",obj.code);
                        logger.info("mongo_process porcess obj block_num",obj.block_num);
                        if (utils.isNotNull(obj.block_num)) {
                            blockInfo.code = obj.code;
                            blockInfo.block_num = obj.block_num;
                            blockInfo.msg = "success";
                        } else {
                            logger.error("code or block_num is null:",logfilePath);
                            blockInfo.msg = "json parse error:"+logfilePath;
                        }
                    } catch (e) {
                        logger.error("parse json error:",e);
                        blockInfo.msg = "json parse error:"+logfilePath;
                    }
                    finishFlag = true;
                } else {
                    blockInfo.msg = "log data is null:"+logfilePath;
                    nullCount++;
                    if (nullCount >= 10) {
                        finishFlag = true;
                    }
                }



            } else {
                blockInfo.msg = "logfile not exists:"+logfilePath;
            }
        }

    } catch (e) {
        logger.error("getLocalMongoMaxBlock error:",e);
    }

    return blockInfo;

}


module.exports = {
    getLocalMongoMaxBlock,

}
