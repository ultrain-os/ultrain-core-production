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

/**
 *
 * @param account
 * @param permName
 * @returns {*}
 */
function getOwnerPkByAccount(account,permName) {

    try {

        let permissions = account.permissions;
        if (permissions.length > 0) {
            for (let i=0;i<account.permissions.length;i++) {
                var permission = account.permissions[i];
                if (permission.perm_name == permName) {
                    logger.debug("account.permissions[i].required_auth",account.permissions[i]);
                    logger.debug("account.permissions[i].required_auth",account.permissions[i].required_auth);
                    return account.permissions[i].required_auth.keys[0].key;
                }
            }
        }


        if (utils.isNull(account)) {
            return null;
        }


    } catch (e) {
        logger.error("getOwnerPkByAccount error:",e);
    }

    return null;

}


// let data = "1971-01-24T14:06:00";
//
// logger.info(formatGensisTime(data));

module.exports = {
    formatGensisTime,
    getOwnerPkByAccount
}