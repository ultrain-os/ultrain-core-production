var logger = require("../../config/logConfig").getLogger("UltrainEncryptUtil");



//hex string max length
var HEX_MAX_LENGTH = 5;

//string split step
var STEP_LENGTH = 2;

//ascii move step
var ASCII_MOVE_STEP = 5;


/**
 * reverse str
 * @param str
 * @returns {string}
 */
function reverse( str ){
    return str.split('').reverse().join('');
};

/**
 * generate 0 num str
 * @param count
 * @returns {string}
 */
function genZeroNum(count) {
    let res = "";
    let i=0;
    while (i++ < count) {
        res = res + "0";
    }

    return res;
}

/**
 *
 * @param str
 * @returns {string}
 */
function strToHex(str) {
    let res = "";
    for (let i=0;i<str.length;i++) {
        let code = str[i].charCodeAt();
        let hexStr = code.toString(16);
        if (hexStr.length < HEX_MAX_LENGTH) {
            hexStr = genZeroNum(HEX_MAX_LENGTH-hexStr.length)+hexStr;
        }
        res = res+hexStr;
    }

    return res;

}

function hexToStr(str) {
    let group = parseInt(str.length/HEX_MAX_LENGTH);
    let res = "";
    for (let i=0;i<group;i++) {
        let startStr = str.substr(i*HEX_MAX_LENGTH,HEX_MAX_LENGTH);
        let num = parseInt(startStr,16);
        res = res+ String.fromCharCode(num);
    }

    return res;

}

/**
 *
 * @param str
 * @param step
 * @returns {string}
 */
function encodeStrWithAscii(str,step) {
    let res = "";
    for (let i=0;i<str.length;i++) {
        let code = str[i].charCodeAt();
        let newChar = String.fromCharCode(code+step);
        res = res+newChar;
    }

    return res;

}

/**
 *
 * @param data
 * @returns {*}
 */
function decodeUltrain(data) {
    //hextostr
    let decodeStr = hexToStr(data);

    //ascii encode
    decodeStr = encodeStrWithAscii(decodeStr,-1*ASCII_MOVE_STEP);

    //reverse
    decodeStr = reverse(decodeStr);


    // change place
    let length = decodeStr.length;
    let group = parseInt(length / STEP_LENGTH);
    if (group > 2) {
        for (let i=0,j= group-1;i<j;i++,j--) {
            let startStr = decodeStr.substr(i*STEP_LENGTH,STEP_LENGTH);
            let endStr = decodeStr.substr(j*STEP_LENGTH,STEP_LENGTH);

            decodeStr = decodeStr.substr(0,i*STEP_LENGTH)+endStr+decodeStr.substr(i*STEP_LENGTH+STEP_LENGTH);
            decodeStr = decodeStr.substr(0,j*STEP_LENGTH)+startStr+decodeStr.substr(j*STEP_LENGTH+STEP_LENGTH);
        }
    }

    return decodeStr;
}

function encodeUltrain(data) {
    let length = data.length;

    let encodeStr = data;
    //switch
    let group = parseInt(length / STEP_LENGTH);
    if (group > 2) {
        for (let i=0,j= group-1;i<j;i++,j--) {
            let startStr = encodeStr.substr(i*STEP_LENGTH,STEP_LENGTH);
            let endStr = encodeStr.substr(j*STEP_LENGTH,STEP_LENGTH);

            encodeStr = encodeStr.substr(0,i*STEP_LENGTH)+endStr+encodeStr.substr(i*STEP_LENGTH+STEP_LENGTH);
            encodeStr = encodeStr.substr(0,j*STEP_LENGTH)+startStr+encodeStr.substr(j*STEP_LENGTH+STEP_LENGTH);
        }
    }

    //reverse
    encodeStr = reverse(encodeStr);

    //ascii
    encodeStr = encodeStrWithAscii(encodeStr,ASCII_MOVE_STEP);

    //strtohex
    encodeStr = strToHex(encodeStr);

    return encodeStr;
}

module.exports = {
    decodeUltrain,
    encodeUltrain,
}

// let json = "{\"code\":0,\"data\":{\"list\":[{\"chainId\":\"11\",\"gmtCreate\":\"2019-02-28 20:22:10\",\"gmtModify\":\"2019-03-11 14:16:20\",\"id\":31,\"ipLocal\":\"172.17.0.17\",\"ipPublic\":\"115.236.45.242\",\"isProducer\":1,\"mngVersion\":\"3991ee062070c2f73e479fc875918f75d04d0496\",\"nodeVersion\":\"a677684fd0fae5e95c01dbf4334d324c2b3be993\",\"user\":\"genesis\"},{\"chainId\":\"11\",\"gmtCreate\":\"2019-02-20 19:48:50\",\"gmtModify\":\"2019-03-11 14:16:20\",\"id\":8,\"ipLocal\":\"172.17.0.18\",\"ipPublic\":\"115.236.45.242\",\"isProducer\":1,\"mngVersion\":\"3991ee062070c2f73e479fc875918f75d04d0496\",\"nodeVersion\":\"a677684fd0fae5e95c01dbf4334d324c2b3be993\",\"user\":\"user.11.111\"},{\"chainId\":\"11\",\"gmtCreate\":\"2019-03-07 13:09:10\",\"gmtModify\":\"2019-03-11 14:16:20\",\"id\":72,\"ipLocal\":\"172.17.0.20\",\"ipPublic\":\"115.236.45.242\",\"isProducer\":1,\"mngVersion\":\"3991ee062070c2f73e479fc875918f75d04d0496\",\"nodeVersion\":\"a677684fd0fae5e95c01dbf4334d324c2b3be993\",\"user\":\"user.11.112\"},{\"chainId\":\"11\",\"gmtCreate\":\"2019-03-07 13:09:01\",\"gmtModify\":\"2019-03-11 14:16:20\",\"id\":68,\"ipLocal\":\"172.17.0.21\",\"ipPublic\":\"115.236.45.242\",\"isProducer\":1,\"mngVersion\":\"3991ee062070c2f73e479fc875918f75d04d0496\",\"nodeVersion\":\"a677684fd0fae5e95c01dbf4334d324c2b3be993\",\"user\":\"user.11.113\"},{\"chainId\":\"11\",\"gmtCreate\":\"2019-03-07 11:22:41\",\"gmtModify\":\"2019-03-11 14:16:20\",\"id\":67,\"ipLocal\":\"172.17.0.22\",\"ipPublic\":\"115.236.45.242\",\"isProducer\":1,\"mngVersion\":\"3991ee062070c2f73e479fc875918f75d04d0496\",\"nodeVersion\":\"a677684fd0fae5e95c01dbf4334d324c2b3be993\",\"user\":\"user.11.114\"},{\"chainId\":\"11\",\"gmtCreate\":\"2019-03-07 13:09:01\",\"gmtModify\":\"2019-03-11 14:16:20\",\"id\":70,\"ipLocal\":\"172.17.0.23\",\"ipPublic\":\"115.236.45.242\",\"isProducer\":1,\"mngVersion\":\"3991ee062070c2f73e479fc875918f75d04d0496\",\"nodeVersion\":\"a677684fd0fae5e95c01dbf4334d324c2b3be993\",\"user\":\"user.11.115\"},{\"chainId\":\"11\",\"gmtCreate\":\"2019-03-07 13:09:01\",\"gmtModify\":\"2019-03-11 14:16:20\",\"id\":69,\"ipLocal\":\"172.17.0.25\",\"ipPublic\":\"115.236.45.242\",\"isProducer\":0,\"mngVersion\":\"3991ee062070c2f73e479fc875918f75d04d0496\",\"nodeVersion\":\"a677684fd0fae5e95c01dbf4334d324c2b3be993\",\"user\":\"user.11.122\"}],\"pageIndex\":0,\"pageSize\":10,\"total\":7},\"msg\":\"success\",\"sign\":\"\",\"time\":1552284994198}";
// //json = "123456789";
// console.log("baseStr:",json);
//
// let encodeStr = encodeUltrain(json);
// console.log("encodeStr:",encodeStr);
//
// let decodeStr = decodeUltrain(encodeStr);
// console.log("decodeStr:",decodeStr);



