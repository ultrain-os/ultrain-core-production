var log4js = require('log4js');
var path = require('path');
var os = require('os');

/**
 * log文件ming（前缀+系统名+时间）
 * @type {string}
 */
var filename = "/log/mng/mng-"+os.hostname()+"-";
/**
 * log整体配置（单文件最大300M)
 * @type {{categories: {default: {level: string, appenders: string[]}}, appenders: {console: {type: string}, default: {filename: string, "maxLogSize ": number, alwaysIncludePattern: boolean, pattern: string, type: string}}}}
 */
var logconfig = {
    "appenders": {
        "console": {
            "type": "console"
        },
        "default": {
            "type": "dateFile",
            "filename": filename,
            "pattern": "yyyy-MM-dd.log",
            "alwaysIncludePattern": true,
            "maxLogSize ": 314572800
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
log4js.configure(logconfig);
var logger = require('log4js').getLogger("sidechain");

module.exports = logger