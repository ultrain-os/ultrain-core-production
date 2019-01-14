var chain = require("./chain/chain")
var chainConfig = require("./chain/chainConfig")
var chainApi = require("./chain/chainApi")
var logger = require("./config/logConfig").getLogger("SideChainService");
const schedule = require('node-schedule');
var sleep = require("sleep")
var utils = require('./common/util/utils')

//定时配置
var chainJobSchedule = "*/10 * * * * *"
var blockJobSchedule = "*/10 * * * * *"
var userJobSchedule = "*/10 * * * * *"

var singleJobSchedule = "*/20 * * * * *";


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
    schedule.scheduleJob(syncBlockSchedule, async function () {
        //用户同步
        await chain.syncUser();
        //资源同步
        await chain.syncResource();
        //块同步
        await chain.syncBlock();
    });

}


startEntry();