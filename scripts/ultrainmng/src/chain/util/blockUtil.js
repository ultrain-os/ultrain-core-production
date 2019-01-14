var logger = require("../../config/logConfig").getLogger("BlockUtil")
var rand = require('random-lib')
var utils = require('../../common/util/utils')

//非propser出块概率配置
var ratio = 0.5;

/**
 * 随机一定概率判断是否出块
 * @returns {boolean}
 */
function randomPushBlock() {
    if (ratio >= 1) {
        return true
    }
    let result = rand.intsSync({min: 1, max: 100, num: 1});
    if (result <= ratio * 100) {
        return true;
    }

    return false;
}

/**
 * 判断这个块是否需要上报
 * @param block
 * @param user
 * @returns {boolean}
 */
function needPushBlock(block, user) {

    if (utils.isNull(block)) {
        return false;
    }

    /**
     * 创世块
     */
    if (utils.isNull(block.proposer) || block.proposer == "genesis") {
        logger.debug("block.proposer is genesis or null, need push");
        return true;
    }

    /**
     * 块的proposer是自己
     */
    if (block.proposer == user) {
        logger.debug("block.proposer is self(" + user + "), need push");
        return true;
    }

    /**
     * 不是自己出的块，按一定概率上报
     */
    if (randomPushBlock() == true) {
        logger.debug("block.proposer is not self(" + user + "),  random success to push");
        return true;
    } else {
        logger.debug("block.proposer is not self(" + user + "),  random failed,need not to push");
    }

    return false;
}

// function unitTest() {
//     /**
//      * 随机概率
//      */
//     var i = 0;
//     var s = 0;
//     for (var i=0;i<20;i++){
//         if (randomPushBlock() == true) {
//             s++;
//             console.log(true);
//         }
//     }
//
//     console.log("total num :"+i+" success:"+s);
//
// }
//
// unitTest()

module.exports = {
    needPushBlock
}