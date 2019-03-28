var chain = require("./chain/chain")
var monitor = require("./chain/monitor")
var chainConfig = require("./chain/chainConfig")
var chainApi = require("./chain/chainApi")
var logger = require("./config/logConfig").getLogger("SideChainService");
const schedule = require('node-schedule');
var sleep = require("sleep")
var utils = require('./common/util/utils')

//定时配置
var singleJobSchedule = "*/10 * * * * *";

//清除pm2 日志(每1分钟）
var logJobSchedule = "*/60 * * * * *";

/**
 * 管家程序入口
 * @returns {Promise<void>}
 */
async function startEntry() {

    logger.info("Ultrainmng start to work:");


    logger.info("waiting sync config data....");
    //等待配置信息同步完成
    await chainConfig.waitSyncConfig()
    logger.info("sync config success");

    //定时同步时间
    var syncBlockSchedule = utils.isNotNull(chainConfig.configFileData.local.blockSyncCycle) ?  chainConfig.configFileData.local.blockSyncCycle : singleJobSchedule;
    logger.info("syncBlockSchedule ",syncBlockSchedule);
    var chainSyncCycleSchedule = utils.isNotNull(chainConfig.configFileData.local.chainSyncCycle) ?  chainConfig.configFileData.local.chainSyncCycle : singleJobSchedule;
    logger.info("chainSyncCycleSchedule ",chainSyncCycleSchedule);
    var chainSyncWorldState = utils.isNotNull(chainConfig.configFileData.local.worldstateSyncCycle) ?  chainConfig.configFileData.local.worldstateSyncCycle : singleJobSchedule;
    logger.info("worldstateSyncCycleSchedule ",chainSyncWorldState);
    var resourceJobSchedule = utils.isNotNull(chainConfig.configFileData.local.resourceSyncCycle) ?  chainConfig.configFileData.local.resourceSyncCycle : singleJobSchedule;
    logger.info("resourceJobSchedule ",resourceJobSchedule);

    var monitorSchedule = utils.isNotNull(chainConfig.configFileData.local.monitorSyncCycle) ?  chainConfig.configFileData.local.monitorSyncCycle : singleJobSchedule;
    logger.info("monitorSchedule ",monitorSchedule);

    var pm2logSyncCycle = utils.isNotNull(chainConfig.configFileData.local.pm2logSyncCycle) ?  chainConfig.configFileData.local.pm2logSyncCycle : logJobSchedule;
    logger.info("pm2logSyncCycle ",pm2logSyncCycle);

    //先做一次链信息同步
    logger.info("do sync chain info :")
    await chain.syncChainInfo();
    logger.info("do sync chain info end ")

    //链信息同步-委员会同步
    logger.info("start chain style sync :",chainSyncCycleSchedule)
    schedule.scheduleJob(chainSyncCycleSchedule, async function () {
        //委员会同步
         await chain.syncChainInfo();
    });

    //同步块，资源，用户-10s
    logger.info("start user,resource,block sync:",syncBlockSchedule)

    //用户同步
    schedule.scheduleJob(syncBlockSchedule, async function () {
        await chain.syncUser();
    });

    //资源同步-近段时间
    schedule.scheduleJob(syncBlockSchedule, async function () {
        await chain.syncNewestResource();
    });


    //块同步
    schedule.scheduleJob(syncBlockSchedule, async function () {
        await chain.syncBlock();
    });

    //ugar同步
    schedule.scheduleJob(syncBlockSchedule, async function () {
        await chain.syncUgas();
    });

    //资源同步-所有数据
    logger.info("start resource all data sync:",resourceJobSchedule)
    schedule.scheduleJob(resourceJobSchedule, async function () {
        await chain.syncAllResource();
    });

    //世界状态同步
    logger.info("start world state sync:",chainSyncWorldState)
    schedule.scheduleJob(chainSyncWorldState, async function () {
        await chain.syncWorldState();
    });

    //monitor同步
    logger.info("monitor sync:",monitorSchedule);
    schedule.scheduleJob(monitorSchedule, async function () {
        await monitor.checkIn();
    });

    //pm2 log清除
    logger.info("pm2 log sync:",pm2logSyncCycle);
    schedule.scheduleJob(pm2logSyncCycle, async function () {
        await monitor.pm2LogFlush();
    });


}


startEntry();
