var logger = require("../../config/logConfig");


/**
 * 日志包装工具类（u3使用）
 * @type {{debug: logger.debug, log: logger.log, error: logger.error}}
 */
const loggerUtil = {
    log: function () {
            logger.info(arguments);
    },
    error: function () {
        logger.error(arguments);

    },
    debug: function () {
            logger.debug(arguments);
    }
}

module.exports = loggerUtil

