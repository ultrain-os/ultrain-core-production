var chain = require("./chain/chain")
var chainConfig = require("./chain/chainConfig")
var logger = require("./config/logConfig").getLogger("SideChainService");
const schedule = require('node-schedule');
var sleep = require("sleep")

//定时配置
var chainJobSchedule = "*/5 * * * * *"
var blockJobSchedule = "*/10 * * * * *"
var userJobSchedule = "*/10 * * * * *"

var debug = true;

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

    if (debug) {
        //联调-单个循环

        schedule.scheduleJob(chainJobSchedule, async function () {
            //委员会同步
            //await  chain.syncChainInfo();
            //用户同步
            await  chain.syncUser();
            //资源同步
            //await  chain.syncResource();
            //块同步
            //await chain.syncBlock();
        });
    } else {

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
}


startEntry();