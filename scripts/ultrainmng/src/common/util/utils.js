var logger = require("../../config/logConfig");
var sleep = require('sleep')
/**
 * 判断是否为空
 * @param data
 * @returns {boolean}
 */
function isNull(data) {
    return (data == "" || data == undefined || data == null) ? true : false;
}

/**
 * 判断是否不为空
 * @param data
 * @returns {boolean}
 */
function isNotNull(data) {
    return !isNull(data);
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
    if (isNotNull(e.code)) {
        return e.code;
    }

    return e;
}

module.exports = {
    isNull,
    isNotNull,
    isAllNull,
    isAllNotNull,
    logNetworkError
}