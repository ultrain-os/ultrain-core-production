var logger = require("../../config/logConfig").getLogger("CommitteeUtil")


/**
 * 构建Committee
 * @param resultJson
 * @param jsonArray
 * @param add
 * @param result
 * @returns {*}
 */
function buildCommittee(resultJson, jsonArray, add, result) {

    for (var i in resultJson) {

        if (resultJson[i]) {
            //logger.debug("result[i]=", resultJson[i].owner);
            //logger.debug("result[i]=", resultJson[i].miner_pk);

            if (jsonArray.length == 0) {
                result.push({
                    account: resultJson[i].owner,
                    public_key: resultJson[i].miner_pk,
                    bls_key: resultJson[i].bls_pk,
                    url: "https://" + resultJson[i].owner + ".com",
                    location: "ultrainio",
                    adddel_miner: add,
                    approve_num: 0
                });
                continue;
            }

            var isInJsonArray = false;
            for (var i2 in jsonArray) {

                if (jsonArray[i2]) {
                    if (jsonArray[i2].owner == resultJson[i].owner) {
                        isInJsonArray = true;
                        break;
                    }
                }
            }

            if (isInJsonArray) {
                continue;
            }

            result.push({
                account: resultJson[i].owner,
                public_key: resultJson[i].miner_pk,
                bls_key: resultJson[i].bls_pk,
                url: "https://" + resultJson[i].owner + ".com",
                location: "ultrainio",
                adddel_miner: add,
                approve_num: 0
            });
        }
    }

    //logger.debug("result array=", result);
    //logger.debug("result array=", result.length);

    return result;
}

function genSeedByChainId(chainId) {
    var str = chainId.toString().substr(0,1);
    return str.charCodeAt()%2;
}

//console.log(genSeedByChainId("BC123"));

/**
 * 生成委员会更新的成员数组 1-新增 0-删除
 * @param localArray
 * @param remoteArray
 * @returns {Array}
 */
function genChangeMembers(localArray, remoteArray, seed) {
    var resultAdd = [];
    resultAdd = buildCommittee(remoteArray, localArray, 1, resultAdd);
    var resultRemove = [];
    resultRemove = buildCommittee(localArray, remoteArray, 0, resultRemove);

    var result = [];

    if (resultAdd.length == 0 && resultRemove.length == 0) {
        return result;
    }

    //哪一个变化多先处理哪一个，如果相等，看seed（来源于chainid）
    if (resultAdd.length > resultRemove.length) {
        result = resultAdd;
    } else if (resultAdd.length < resultRemove.length) {
        result = resultRemove;
    } else {
        //equal
        if (seed == 0) {
            result = resultAdd;
        } else {
            result = resultRemove;
        }
    }

    if (result.length > 0) {
        result.sort(function (s, t) {
            if (s.account > t.account) {
                return 1;
            }

            if (s.account < t.account) {
                return -1;
            }
            return 0;

        })
    }
    return result;
}

/**
 * 验证更新委员会是否合法
 * @param resultArray
 * @returns {boolean}
 */
function isValidChangeMembers(resultArray) {
    if (resultArray.length == 0) {
        logger.info("subchain committee has no change")
        return false;
    }

    // if (resultArray.length > 30) {
    //     logger.error("subchain committee change num > 30",resultArray)
    //     return false;
    // }

    return true;
}

/**
 * 判断
 * @param members
 * @param user
 * @returns {boolean}
 */
function isStayInCommittee(members, user) {

    //.console.log(members);
    //console.log(user);
    if (members.length == 0) {
        return false;
    }

    let find = false;
    members.forEach(function (item, index) {
        //console.log(item)
        if (item.owner == user) {
            find = true;
        }
    })


    return find;
}

module.exports = {
    genChangeMembers,
    isValidChangeMembers,
    isStayInCommittee,
    genSeedByChainId
}
