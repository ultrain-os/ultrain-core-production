var log4js = require('log4js');
var path = require('path');
var os = require('os');

/**
 * log文件mng（前缀+系统名+时间）
 * @type {string}
 */
var dir = "/log/mng/";
var filename = "mng-"+os.hostname()+"-";
/**
 * log整体配置
 * @type {{categories: {default: {level: string, appenders: string[]}}, appenders: {console: {type: string}, default: {filename: string, "maxLogSize ": number, alwaysIncludePattern: boolean, pattern: string, type: string}}}}
 */
var logConfig = {
    "appenders": {
        "console": {
            "type": "console"
        },
        "default": {
            "type": "dateFile",
            "filename": dir+filename,
            "pattern": "yyyy-MM-dd.log",
            "alwaysIncludePattern": true,
            "maxLogSize ": 314572800 //单文件最大300M
        }
    },
    "categories": {
        "default": {
            "appenders": [
                "console",
                "default"
            ],
            "level": "all"
        }
    }
};
log4js.configure(logConfig);
var logger = require('log4js').getLogger("sidechain");

module.exports = logger