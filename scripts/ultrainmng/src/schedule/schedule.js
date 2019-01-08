const http = require('http');
var querystring = require('querystring');
const axios = require('axios')
var logger = require("../config/logConfig");
var IniFile = require('../common/util/iniFile');
var Constants = require('../common/constant/constants');
var ShellCmd = require('../common/util/shellCmd');
var CmdConstants = Constants.cmdConstants;
var WSS = require("../wss/wss")
var NodUltrain = require("../nodultrain/nodultrain")
/**
 * 调度程序
 * 调度到新链，增加新链
 */

/**
 * 子链切换
 * @returns {Promise<void>}
 */
var chainChange = async function (chainId, nodFilePath) {

    logger.info("start to change a new chain (chain id :" + chainId + ")");

    //1. 关闭nod和世界状态
    logger.info("step-1 : close nod and wss");
    let cmdRes = await ShellCmd.execBatchCmd(CmdConstants.KILL_NODULTRAIN, CmdConstants.KILL_WORLDSTATE);
    if (!cmdRes) {
        logger.info("step-1 : close nod and wss error");
        return false;
    }
    logger.info("step-1 : close nod and wss success");

    //2. 删除历史数据(块数据，memory.bin,ws数据）
    logger.info("step-2 : clear history data");
    cmdRes = await ShellCmd.execCmd(CmdConstants.CLEAR_BLOCK_DATA, CmdConstants.CLEAR_SHARED_MEMORY_DATA, CmdConstants.CLEAR_WORLD_STATE_FILE);
    if (!cmdRes) {
        logger.info("step-2 : clear history data error");
        return false;
    }
    logger.info("step-2 : clear history data success");

    //3.通过chain查询获取ip_list
    logger.info("step-3 : get new chain_id and ip_list");
    let ip_list = {};

    //4. 更新世界状态ini
    logger.info("step-4 : get subchain's chain and seed ip_list");
    WSS.updateConfig(chainId, ip_list);

    //5-启动ws
    logger.info("step-5 : start worldstate");
    cmdRes = ShellCmd.execCmd(CmdConstants.START_WORLDSTATE);
    if (!cmdRes) {
        logger.info("step-5 : start worldstate error");
        return false;
    }
    logger.info("step-5 : check worldstate is successed:");
    logger.info("step-5 : start worldstate success");
    //todo 调用接口获取状态

    //6.从主链获取当前子链的块高和hash数值
    let subChainBlockHeight = 500;
    let subChainBlockWSHash = "avdsf23sd234es"
    logger.info("step-6 : sync subchain block height and ws hash from mainchain");

    //7.管家用块高，hash作为参数传给ws
    let res = WSS.syncWorldstate(subChainBlockWSHash, subChainBlockHeight);
    logger.info("step-7 : send sync request to ws");

    //8.轮询世界状态状态
    res = WSS.pollingWSState();
    logger.info("step-8 : polling ws state success");

    //9.判断是否需要拉块
    res = WSS.syncBlocks(0, 1000);
    logger.info("step-9 : send block sync request to ws");

    //10.轮询ws查询同步块状态码
    res = WSS.pollingBlockState();
    logger.info("step-10 : polling block state success");

    //11.更新nod程序的ini文件并启动
    logger.info("step-11 : update nod config file");
    NodUltrain.updateConfig(nodFilePath, ip_list);
    logger.info("step-11 : update nod config file success");

    //12.启动nod程序
    logger.info("step-12 : start nod application");
    await ShellCmd.execCmd(CmdConstants.START_NODULTRAIN);

}

/**
 * 新节点入子链
 * @returns {Promise<void>}
 */
var addNewNodeToChain = async function (chainId, nodFilePath) {

    logger.info("start to add a new node to subchain (chain id :" + chainId + ")");

    //1. 关闭nod和世界状态
    logger.info("step-1 : close nod and wss");
    let cmdRes = await ShellCmd.execBatchCmd(CmdConstants.KILL_NODULTRAIN, CmdConstants.KILL_WORLDSTATE);
    if (!cmdRes) {
        logger.info("step-1 : close nod and wss error");
        return false;
    }
    logger.info("step-1 : close nod and wss success");

    //2. 删除历史数据(块数据，memory.bin,ws数据）
    logger.info("step-2 : clear history data");
    cmdRes = await ShellCmd.execCmd(CmdConstants.CLEAR_BLOCK_DATA, CmdConstants.CLEAR_SHARED_MEMORY_DATA, CmdConstants.CLEAR_WORLD_STATE_FILE);
    if (!cmdRes) {
        logger.info("step-2 : clear history data error");
        return false;
    }
    logger.info("step-2 : clear history data success");

    //3.通过chain查询获取ip_list
    logger.info("step-3 : get new chain_id and ip_list");
    let ip_list = {};

    //4. 更新世界状态ini
    logger.info("step-4 : get subchain's chain and seed ip_list");
    WSS.updateConfig(chainId, ip_list);

    //5-启动ws
    logger.info("step-5 : start worldstate");
    cmdRes = ShellCmd.execCmd(CmdConstants.START_WORLDSTATE);
    if (!cmdRes) {
        logger.info("step-5 : start worldstate error");
        return false;
    }
    logger.info("step-5 : check worldstate is successed:");
    logger.info("step-5 : start worldstate success");
    //todo 调用接口获取状态

    //6.从主链获取当前子链的块高和hash数值
    let subChainBlockHeight = 500;
    let subChainBlockWSHash = "avdsf23sd234es"
    logger.info("step-6 : sync subchain block height and ws hash from mainchain");

    //7.管家用块高，hash作为参数传给ws
    let res = WSS.syncWorldstate(subChainBlockWSHash, subChainBlockHeight);
    logger.info("step-7 : send sync request to ws");

    //8.轮询世界状态状态
    res = WSS.pollingWSState();
    logger.info("step-8 : polling ws state success");

    //9.判断是否需要拉块
    res = WSS.syncBlocks(0, 1000);
    logger.info("step-9 : send block sync request to ws");

    //10.轮询ws查询同步块状态码
    res = WSS.pollingBlockState();
    logger.info("step-10 : polling block state success");

    //11.更新nod程序的ini文件并启动
    logger.info("step-11 : update nod config file");
    NodUltrain.updateConfig(nodFilePath, ip_list);
    logger.info("step-11 : update nod config file success");

    //12.启动nod程序
    logger.info("step-12 : start nod application");
    await ShellCmd.execCmd(CmdConstants.START_NODULTRAIN);

}


module.exports = {
    chainChange,
    addNewNodeToChain
}