var process = require('child_process');
var logger = require("../../config/logConfig").getLogger("ShellCmd");
var Constants = require('../constant/constants');
var async = require('async');
var deasync = require('deasync');

/**
 * shell命令执行
 */
class ShellCmd {
}

/**
 * 命令执行
 * @param command
 * @returns {Promise<void>}
 */
ShellCmd.execCmd = async function (command) {

    try {
        logger.info("execmd :",command);
        var done = false;
        var result = true;
        /**
         * 由于execSync只有在node11以后才有，暂时用主动卡死方式执行
         */
        process.exec(command, function (error, stdout, stderr, finish) {
            if (error !== null) {
                logger.error('exec error: ' + error);
            } else {
                logger.info("exccmd success:"+command);
                result = true;
            }
            done = true;

        });
        // require('deasync').loopWhile(function () {
        //     return !done;
        // });
        return result;

    } catch (e) {
        logger.error("exec cmd error:",e);
    }
    return false;
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
     process.execFile(command, args, options, function (error, stdout, stderr) {
         if (error !== null) {
             logger.error('exec error: ' + error);
         } else {
             logger.info("exccmd success:"+command);
         }
    });

     return true;
}


module.exports = ShellCmd;