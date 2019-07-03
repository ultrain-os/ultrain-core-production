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

//清除pm2 日志(每2小时）
var logJobSchedule = "0 0 */2 * * *";

//清除链信息缓存(每2小时）
var clearCacheSchedule = "0 30 */2 * * *";

//上传ram信息
var ramJobSchedule = "0 0 3 * * *";

//logrotate调度（每1小时）
var logrotateSchedule = "0 24 */1 * * *";

/**
 * 管家程序入口
 * @returns {Promise<void>}
 */
async function startEntry() {

    logger.info("Ultrainmng start to work:");


    logger.info("waiting sync config data....");
    //等待配置信息同步完成
    await chainConfig.syncConfig();
    let count = 0;
    let res = chainConfig.isReady();
    while (res.result == false) {
        logger.error("chainConfig.isReady error:",res.msg);
        count++;
        sleep.msleep(1000 * 1);
        await chainConfig.syncConfig();
        logger.info("config is not ready ,wait to next check...("+count+")");
        monitor.setErrorStartup(res.msg);
        if (count >= 10) {
            count =0;
            logger.info("config is not ready,do monitor check");
            await monitor.checkIn();
            sleep.msleep(1000 * 1);
            await chainConfig.syncConfig();
        }
        res = chainConfig.isReady();
    }
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

    var clearCacheSyncCycle = utils.isNotNull(chainConfig.configFileData.local.clearCacheSchedule) ?  chainConfig.configFileData.local.clearCacheSchedule : clearCacheSchedule;
    logger.info("clearCacheSchedule ",clearCacheSchedule);

    var uploadUgasSyncCycle = utils.isNotNull(chainConfig.configFileData.local.uploadUgasSyncCycle) ?  chainConfig.configFileData.local.uploadUgasSyncCycle : "0 */10 * * * *";
    logger.info("uploadUgasSyncCycle ",uploadUgasSyncCycle);

    //先做一次链信息同步
    logger.info("do sync chain info :")
    await chain.syncChainInfo();
    logger.info("do sync chain info end ")

    //链信息同步-委员会同步
    logger.info("start chain style sync :",chainSyncCycleSchedule)
    schedule.scheduleJob(chainSyncCycleSchedule, async function () {
        //链同步，本地维护员信息同步
         await chain.syncChainInfo();
    });

    //同步块，资源，用户-10s
    logger.info("start user,resource,block sync:",syncBlockSchedule)

    //用户同步
    schedule.scheduleJob(syncBlockSchedule, async function () {
        await chain.syncUser();
    });

    //委员会同步
    schedule.scheduleJob(syncBlockSchedule, async function () {
        await chain.syncCommitee();
    });

    //资源同步-近段时间
    schedule.scheduleJob(syncBlockSchedule, async function () {
        await chain.syncNewestResource();
    });


    //块同步
    schedule.scheduleJob(syncBlockSchedule, async function () {
       await chain.syncBlock();
    });

    //ugas同步
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

    logger.info("clearCacheSyncCycle sync:",clearCacheSyncCycle);
    schedule.scheduleJob(clearCacheSyncCycle, async function () {
        await chain.clearCache();
    });

    logger.info("upload ram info:",ramJobSchedule);
    schedule.scheduleJob(ramJobSchedule,async function () {
        await monitor.uploadRamUsage();
    })

    logger.info("upload ugas info:",uploadUgasSyncCycle);
    schedule.scheduleJob(uploadUgasSyncCycle,async function () {
        await monitor.uploadUgasToMonitor();
    })


    logger.info("logrotate info:",logrotateSchedule);
    schedule.scheduleJob(logrotateSchedule,async function () {
        await monitor.logrotate();
    })

    let clearRestartSchedule = "18 */30 * * * *"
    logger.info("clear restart count:",clearRestartSchedule);
    schedule.scheduleJob(clearRestartSchedule,async function () {
        await chain.clearRestartCount();
    })

}


startEntry();

