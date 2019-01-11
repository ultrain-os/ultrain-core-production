var chain = require("./chain/chain")
var chainConfig = require("./chain/chainConfig")
var logger = require("./config/logConfig").getLogger("SideChainService");
const schedule = require('node-schedule');

//定时配置
var chainJobSchedule = "*/10 * * * * *"
var blockJobSchedule = "*/10 * * * * *"
var userJobSchedule = "*/10 * * * * *"

/**
 * 管家程序入口
 * @returns {Promise<void>}
 */
async function startEntry() {

    logger.info("start to work:");

    logger.info("waiting sync config....");
    //等待配置信息同步完成
    await chainConfig.waitSyncConfig()
    logger.info("sync config success");

    //启动同步链信息
    logger.info("start sync chain task");
    schedule.scheduleJob(chainJobSchedule, function () {
        chain.syncChainInfo();
    });

    //启动同步块信息
    logger.info("start sync chain block");
    schedule.scheduleJob(blockJobSchedule, function () {
        chain.syncBlock();
    });

    // //启动同步用户&资源信息
    logger.info("start sync chain user & resources");
    schedule.scheduleJob(userJobSchedule, function () {
        chain.syncUser();
        chain.syncResource();
    });
}


startEntry();