const crypto = require('crypto');
const fs = require('fs');
var logger = require("../../config/logConfig").getLogger("HashUtil");
var constants = require("../constant/constants");

/**
 *
 * @param str
 * @param algorithm
 * @param encoding
 * @returns {string}
 */
function checksum(str, algorithm, encoding) {

    let hash = crypto.createHash(algorithm);
    hash.update(str, encoding);
    return hash.digest('hex');
}

/**
 *
 * @param filepath
 * @returns {Promise<*>}
 */
 function calcHash(filepath,algorithm) {
    try {
        logger.info("start calc hash file :",filepath);
        if (fs.existsSync(filepath)) {
            let data =fs.readFileSync(filepath,null);
            //logger.info("data:",data);
            let hash =  checksum(data, algorithm,null);
            logger.info("finish calc hash file :",filepath);
            logger.info("finish calc hash file hash :",hash);
            return hash;

        } else {
            logger.error("hashutil not exist:",filepath);
        }

    } catch (e) {
        logger.error("calc hash file error:",e);
    }

    return null;

}

function calcMd5(data) {
    try {
        return checksum(data, "md5",null);
    } catch (e) {
        logger.error("calcMd5 error:",e)
    }

    return null;
}

// async  function test() {
//     let hash = await calcHash("/root/workspace/ultrain-core/scripts/ultrainmng/deploy/sideChainService.js");
//     console.log("hash:"+hash);
//     hash = await calcHash("/root/workspace/ultrain-core/build/programs/nodultrain/nodultrain");
//     console.log("hash:"+hash);
//
//     hash = await calcHash("/root/workspace/ultrain-core/scripts/ultrainmng/src/sideChainService.js");
//     console.log("hash:"+hash);
//
// }
//
// test();

module.exports = {
    calcHash,
    calcMd5
}




let data = "{\"code\":0,\"data\":{\"list\":[{\"chainId\":\"11\",\"gmtCreate\":\"2019-02-28 20:22:10\",\"gmtModify\":\"2019-03-11 15:18:20\",\"id\":31,\"ipLocal\":\"172.17.0.17\",\"ipPublic\":\"115.236.45.242\",\"isProducer\":1,\"mngVersion\":\"3991ee062070c2f73e479fc875918f75d04d0496\",\"nodeVersion\":\"a677684fd0fae5e95c01dbf4334d324c2b3be993\",\"user\":\"genesis\"},{\"chainId\":\"11\",\"gmtCreate\":\"2019-02-20 19:48:50\",\"gmtModify\":\"2019-03-11 15:18:20\",\"id\":8,\"ipLocal\":\"172.17.0.18\",\"ipPublic\":\"115.236.45.242\",\"isProducer\":1,\"mngVersion\":\"3991ee062070c2f73e479fc875918f75d04d0496\",\"nodeVersion\":\"a677684fd0fae5e95c01dbf4334d324c2b3be993\",\"user\":\"user.11.111\"},{\"chainId\":\"11\",\"gmtCreate\":\"2019-03-07 13:09:10\",\"gmtModify\":\"2019-03-11 15:18:20\",\"id\":72,\"ipLocal\":\"172.17.0.20\",\"ipPublic\":\"115.236.45.242\",\"isProducer\":1,\"mngVersion\":\"3991ee062070c2f73e479fc875918f75d04d0496\",\"nodeVersion\":\"a677684fd0fae5e95c01dbf4334d324c2b3be993\",\"user\":\"user.11.112\"},{\"chainId\":\"11\",\"gmtCreate\":\"2019-03-07 13:09:01\",\"gmtModify\":\"2019-03-11 15:18:20\",\"id\":68,\"ipLocal\":\"172.17.0.21\",\"ipPublic\":\"115.236.45.242\",\"isProducer\":1,\"mngVersion\":\"3991ee062070c2f73e479fc875918f75d04d0496\",\"nodeVersion\":\"a677684fd0fae5e95c01dbf4334d324c2b3be993\",\"user\":\"user.11.113\"},{\"chainId\":\"11\",\"gmtCreate\":\"2019-03-07 11:22:41\",\"gmtModify\":\"2019-03-11 15:18:20\",\"id\":67,\"ipLocal\":\"172.17.0.22\",\"ipPublic\":\"115.236.45.242\",\"isProducer\":1,\"mngVersion\":\"3991ee062070c2f73e479fc875918f75d04d0496\",\"nodeVersion\":\"a677684fd0fae5e95c01dbf4334d324c2b3be993\",\"user\":\"user.11.114\"},{\"chainId\":\"11\",\"gmtCreate\":\"2019-03-07 13:09:01\",\"gmtModify\":\"2019-03-11 15:18:20\",\"id\":70,\"ipLocal\":\"172.17.0.23\",\"ipPublic\":\"115.236.45.242\",\"isProducer\":1,\"mngVersion\":\"3991ee062070c2f73e479fc875918f75d04d0496\",\"nodeVersion\":\"a677684fd0fae5e95c01dbf4334d324c2b3be993\",\"user\":\"user.11.115\"},{\"chainId\":\"11\",\"gmtCreate\":\"2019-03-07 13:09:01\",\"gmtModify\":\"2019-03-11 15:18:20\",\"id\":69,\"ipLocal\":\"172.17.0.25\",\"ipPublic\":\"115.236.45.242\",\"isProducer\":0,\"mngVersion\":\"3991ee062070c2f73e479fc875918f75d04d0496\",\"nodeVersion\":\"a677684fd0fae5e95c01dbf4334d324c2b3be993\",\"user\":\"user.11.122\"}],\"pageIndex\":0,\"pageSize\":10,\"total\":7},\"msg\":\"success\",\"time\":1552288708577}MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQC6TQALgms1BnT02fowOtSGGCQ1ed1GVWvzODASnDMlyRsbiwnsMROf7YZ7umA4ma5n9erPyw27ile7JDjsQo1GbUZn2tAbjg1G7VPgkxp9QZp8uXquTI9bDEYXIeYQS9f71mh8DkR3VOUru8+j5uCOqmF+jiDMOt8qf5Yyhw5fbQIDAQAB";

 let sign = calcMd5(data);
console.log("hash:"+sign);

