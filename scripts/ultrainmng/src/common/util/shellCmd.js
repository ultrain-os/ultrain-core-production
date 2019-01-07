var process = require('child_process');
var logger = require("../../config/logconfig");
var Constants = require('../constant/constants');
var async = require('async');
var deasync = require('deasync');

/**
 * shell命令执行
 */
class ShellCmd {
}

/**
 * 命令执行（异步）
 * @param command
 * @returns {Promise<void>}
 */
ShellCmd.execCmd = async function (command) {

    var done = false;
    var result = false;
    /**
     * 由于execSync只有在node11以后才有，暂时用主动卡死方式记性，后期优化
     */
    process.exec(command, function (error, stdout, stderr , finish) {
        done = true;
        if (error !== null) {
            logger.log('exec error: ' + error);
        } else {
            console.info(stdout);
            result = true;
        }

    });
    require('deasync').loopWhile(function(){return !done;});
    return result;
}

/**
 * 批量执行多个命令，只要有一个失败就暂停
 * @returns {Promise<void>}
 */
ShellCmd.execBatchCmd = async function () {

    for (var i = 0; i < arguments.length; i++) {
        if (!this.execCmd(arguments[i])) {
            return false;
        }
    }
}

/**
 * 执行文件
 * @param command
 * @param args
 * @param options
 * @returns {Promise<void>}
 */
ShellCmd.execCmdFiles = async function (command, args, options) {
     let finish = 0;
     process.execFile(command, args, options, function (error, stdout, stderr) {
        if (error !== null) {
            logger.log('exec file: ' + error);
        }
        logger.debug(stdout);
    });
}

/**
 * 关闭nod程序
 * @returns {Promise<void>}
 */
ShellCmd.stopNodultrain = async function () {
    await this.execCmd(Constants.cmdConstants.KILL_NODULTRAIN);
}

/**
 * 关闭pm2
 * @returns {Promise<void>}
 */
ShellCmd.stopPM2 = async function () {
    await this.execCmd(Constants.cmdConstants.KILL_PM2);
}

module.exports = ShellCmd;