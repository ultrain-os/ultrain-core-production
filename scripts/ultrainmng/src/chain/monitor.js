const {U3} = require('u3.js');
const fs = require('fs');
var logger = require("../config/logConfig").getLogger("Monitor");
var chainConfig = require("./chainConfig")
var chainApi = require("./chainApi")
var timeConstats = require("../common/constant/constants").timeConstats
var chainNameConstants = require("../common/constant/constants").chainNameConstants
var contractConstants = require("../common/constant/constants").contractConstants
var tableConstants = require("../common/constant/constants").tableConstants
var scopeConstants = require("../common/constant/constants").scopeConstants
var actionConstants = require("../common/constant/constants").actionConstants
var chainIdConstants = require("../common/constant/constants").chainIdConstants
var pathConstants = require("../common/constant/constants").pathConstants
var sleep = require("sleep")
var utils = require("../common/util/utils")
var committeeUtil = require("./util/committeeUtil");
var blockUtil = require("./util/blockUtil");
var voteUtil = require("./util/voteUtil");
var NodUltrain = require("../nodultrain/nodultrain")
var WorldState = require("../worldstate/worldstate")
var chainUtil = require("./util/chainUtil");
const publicIp = require('public-ip');

/**
 *
 * @returns {string}
 */
function getNodVersion() {
    return "1.0.0";
}

/**
 *
 * @returns {string}
 */
function getMngVersion() {
    return "1.0.0";
}

/**
 *
 * @returns {*}
 */
function getMonitorUrl() {
    return chainConfig.configFileData.target["monitor-server-endpoint"];
}

/**
 *
 * @returns {boolean}
 */
function checkNeedSync() {
    if (chainConfig.configFileData.local["monitor"] == false) {
        return false;
    }

    return true;
}


/**
 * check in
 * @returns {Promise<void>}
 */
async function checkIn() {


    if (checkNeedSync() == false) {
        return
    }

    logger.info("monitor checkin start");
    logger.info("monitor check in(http:" + getMonitorUrl());
    let publicip = await utils.getPublicIp();
    logger.debug("extern ip:"+publicip);
    var user = "unkown";
    if (utils.isNotNull(chainConfig.myAccountAsCommittee)) {
        user = chainConfig.myAccountAsCommittee;
    }
    var param = {
        "chainId": chainConfig.chainName,
        "ipLocal": utils.getLocalIPAdress(),
        "ipPublic": publicip,
        "user": user,
        "nodVersion": getNodVersion(),
        "mngVersion": getMngVersion()
    };
    await chainApi.monitorCheckIn(getMonitorUrl(), param);

    logger.info("monitor checkin end");
}


module.exports = {
    checkIn
}
