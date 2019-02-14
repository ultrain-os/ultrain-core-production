
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

// console.log(ipListToStr(["172.17.0.9","172.17.0.10","172.17.0.11"],"222",","))

module.exports = {
    isNull,
    isNotNull,
    isAllNull,
    isAllNotNull,
    logNetworkError,
    isNullList,
    getLocalIPAdress,
    ipListToStr
}