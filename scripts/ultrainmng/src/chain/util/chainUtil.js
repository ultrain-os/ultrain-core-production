var logger = require("../../config/logConfig").getLogger("ChainUtil")
var utils = require('../../common/util/utils')

/**
 * 创世时间格式化
 * @param time
 * @returns {*|void|string|never}
 */
function formatGensisTime(time) {
    //return time.replace("T"," ");
    return time;
}

// let data = "1971-01-24T14:06:00";
//
// logger.info(formatGensisTime(data));

module.exports = {
    formatGensisTime
}