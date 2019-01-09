var IniFile = require('../common/util/iniFile');
var ShellCmd = require('../common/util/shellCmd');
var Constants = require('../common/constant/constants');
var WorldState = require('../worldstate/worldstate');
var utils = require('../common/util/utils');
var async = require('async');
var wsResUtil = require("../worldstate/wsResUtil")
var sleep = require('sleep')


/**
 * 世界状态测试
 * @returns {Promise<void>}
 */
async function interactWithWorldState() {

        /**
         * 关闭
         */
        let result = await WorldState.stop(1000);
        if (result) {
            console.log("wss stop successed");
        } else {
            console.log("wss stop error");
            return;
        }

        /**
         * start
         */
        let chainId = "11";
        let seedIp = "11.11.11.11"
        result = await WorldState.start(chainId, seedIp, 5000);
        if (result) {
            console.log("wss start successed");
        } else {
            console.log("wss start error");
            return;
        }

        await WorldState.checkAlive();
        /**
         * require_ws
         */
        let hash = "hash";
        let height = 0;
        result = await WorldState.syncWorldState(hash, height);
        if (result == false) {
            console.log("sync worldstate request error");
        } else {
            console.log("sync worldstate request success");
        }

        /**
         * 轮询检查同步世界状态情况
         */
        result = await WorldState.pollingkWSState(1, 500, 3000);
        if (result == false) {
            console.log("sync worldstate  error");
        } else {
            console.log("sync worldstate  success");
        }

        /**
         * 调用block
         */
        result = await WorldState.syncBlocks();
        if (result == false) {
            console.log("sync block request error");
        } else {
            console.log("sync block request success");
        }

        /**
         * 轮询block状态
         */
        result = await WorldState.pollingBlockState(1, 500, 3000);
        if (result == false) {
            console.log("sync block  error");
        } else {
            console.log("sync block  success");
        }
}



interactWithWorldState();












