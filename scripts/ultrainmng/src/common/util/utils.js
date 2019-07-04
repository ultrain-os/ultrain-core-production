//var logger = require("../../config/logConfig").getLogger("Utils");
const publicIp = require('public-ip');
const fs = require('fs');
const path = require('path');
const os = require('os');

/**
 * 判断是否为空
 * @param data
 * @returns {boolean}
 */
function isNull(data) {
    return (data == "" || data == undefined || data == null) ? true : false;
}

/**
 * 判断是否为空
 * @param data
 * @returns {boolean}
 */
function isNullList(data) {
    if (isNull(data)) {
        return true;
    }

    if (data.length ==0) {
        return true;
    }
    return false;
}

/**
 * 判断是否不为空
 * @param data
 * @returns {boolean}
 */
function isNotNull(data) {
    return isNull(data) == false;
}

/**
 * 判断所有参数是否都不为空
 * @returns {boolean}
 */
function isAllNotNull() {

    for (var i = 0; i < arguments.length; i++) {
        if (isNull(arguments[i])) {
            return false;
        }
    }

    return true;
}

/**
 * 判断所有参数是否都不为空
 * @returns {boolean}
 */
function isAllNull() {

    for (var i = 0; i < arguments.length; i++) {
        if (isAllNotNull(arguments[i])) {
            return false;
        }
    }

    return true;
}


/**
 *
 * @param e
 * @returns {*}
 */
function logNetworkError(e) {

    if (isNotNull(e.data)) {
        return e.data;
    }

    if (isNotNull(e.code)) {
        return e.code;
    }

    return e;
}

/**
 * 获取本机ip
 * @returns {*|string}
 */
function getLocalIPAdress(){
    var interfaces = require('os').networkInterfaces();
    //console.log(interfaces);
    for(var devName in interfaces){
        var iface = interfaces[devName];
        for(var i=0;i<iface.length;i++){
            var alias = iface[i];
            if(alias.family === 'IPv4' && alias.address !== '127.0.0.1' && !alias.internal){
                return alias.address;
            }
        }
    }
}

/**
 * 获取外网ip
 * @returns {Promise<void>}
 */
var cachePublicIp = "";
var ptime = 0;
var errortime = 0;
getPublicIp = async () => {

    let ip = getLocalIPAdress();
    //logger.info("cachePublicIp:",cachePublicIp);
    if (isNotNull(cachePublicIp)) {
        ip = cachePublicIp;
        ptime++;
    }
    if (ptime == 0) {
        try {

            let retry = 3;
            let res = await publicIp.v4();
            while (isNull(res)) {
                res = await publicIp.v4();
                retry--;
                if (retry <= 0) {
                    break;
                }
            }

            if (isNotNull(res)) {
                ip = res;
                cachePublicIp = res;
            }
        } catch (e) {
            console.error("getPublicIp error:", e);
        }
    }

    if (ptime >= 10) {
        ptime = 0;
    }
    console.error("PublicIp:",ip);
    return ip;
}

function ipListToStr(iplist,port,splitChar) {
    let result = "";
    if (iplist.length > 0) {
        result = iplist[0]+":"+port;
        for (var i=1;i<iplist.length;i++) {
            result = result+splitChar+iplist[i]+":"+port;
        }
    }

    return result;
}

/**
 *
 * @param str
 * @param logstr
 */
function addLogStr(str,logstr) {
    let time  = getLocalTime();
    return str + time+":"+logstr +"\r\n    ";
}

function getLocalTime() {
    return new Date(parseInt(new Date().getTime()) * 1).toLocaleString().replace(/:\d{1,2}$/,' ');
}

/**
 *
 * @param filepath
 * @returns {*}
 */
function formatHomePath(filepath) {
    if (filepath.indexOf("~") == 0) {
        return path.join(os.homedir(),filepath.substr(1));
    }

    return filepath;
}


module.exports = {
    isNull,
    isNotNull,
    isAllNull,
    isAllNotNull,
    logNetworkError,
    isNullList,
    getLocalIPAdress,
    ipListToStr,
    getPublicIp,
    addLogStr,
    formatHomePath,
}
