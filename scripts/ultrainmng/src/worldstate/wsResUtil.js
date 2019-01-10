const http = require('http');
var querystring = require('querystring');
const axios = require('axios')
var logger = require("../config/logConfig").getLogger("WSResUtil");
var IniFile = require('../common/util/iniFile');
var Constants = require('../common/constant/constants');
var utils = require("../common/util/utils")

/**
 * 世界状态返回的状态码枚举
 * @type {{HASH_NOT_MATH: number, SUCCESS: number, ENDPOINT_UNREACHABLE: number, ONGOING: number}}
 */
var resCodeEnum = {
    SUCCESS: 0,
    ONGOING: 1,
    ENDPOINT_UNREACHABLE: 2,
    HASH_NOT_MATH: 3
}

/**
 * 判断是否成功
 * @param res
 * @returns {boolean}
 */
var isSuccess = function (res) {
    return checkStatus(res, resCodeEnum.SUCCESS);
}

/**
 * 判断还在进行中
 * @param res
 * @returns {boolean}
 */
var isOngoing = function (res) {
    return checkStatus(res, resCodeEnum.ONGOING);
}

/**
 * 判断hash不匹配
 * @param res
 * @returns {boolean}
 */
var isHashNotMatch = function (res) {
    return checkStatus(res, resCodeEnum.HASH_NOT_MATH);
}

/**
 * 判断是否节点不可达
 * @param res
 * @returns {boolean}
 */
var isUnreachabled = function (res) {
    return checkStatus(res, resCodeEnum.ENDPOINT_UNREACHABLE);
}

/**
 * 错误
 * @param res
 * @returns {boolean}
 */
var isError = function (res) {
    return !checkStatus(res, resCodeEnum.SUCCESS);
}

/**
 * 未知错误
 * @param res
 * @returns {boolean}
 */
var isUnknownError = function (res) {
    try {
        for (keyinfo in resCodeEnum) {
            if (resCodeEnum[keyinfo] == res.code) {
                return false;
            }
        }

        return true;

    } catch (e) {
        logger.error("check ws unknown status", e);
    }

    return false;
}

/**
 * 比较状态
 * @param code
 * @param compareCode
 * @returns {boolean}
 */
var checkStatus = function (res, code) {
    try {
        // console.log("res: "+res.code);
        // console.log("compare: "+code);
        if (res.code == code) {
            return true;
        }
    } catch (e) {
        //logger.error("check ws res status", e);
    }

    return false;
}

module.exports = {
    isSuccess,
    isError,
    isOngoing,
    isHashNotMatch,
    isUnreachabled,
    isUnknownError

}