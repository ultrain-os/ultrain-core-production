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
    /**
     *
     * [{
            "account": "user.114",
            "public_key": "b9a55c3c661abd8c539b7a7c05c8176036f87aeeb6b117a138327cfdb374cc23",
            "url": "https://user.115.com",
            "location": 0
        }, {
            "account": "user.115",
            "public_key": "8f6e3b3336276138023617f0ae0e6fd0c37d27aa9995f9803fe78df4941dd3ec",
            "url": "https://user.115.com",
            "location": 0
        }]

     * **/

    for (var i in resultJson) {

        if (resultJson[i]) {
            //logger.debug("result[i]=", resultJson[i].owner);
            //logger.debug("result[i]=", resultJson[i].miner_pk);

            if (jsonArray.length == 0) {
                result.push({
                    account: resultJson[i].owner,
                    public_key: resultJson[i].miner_pk,
                    bls_key: resultJson[i].bls_pk,
                    url: "https://"+resultJson[i].owner+".com",
                    location: 0,
                    adddel_miner: add,
                    approve_num : 0
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
                url: "https://"+resultJson[i].owner+".com",
                location: 0,
                adddel_miner: add,
                approve_num : 0
            });
        }
    }

    //logger.debug("result array=", result);
    //logger.debug("result array=", result.length);

    return result;
}

/**
 * 生成委员会更新的成员数组 1-新增 0-删除
 * @param localArray
 * @param remoteArray
 * @returns {Array}
 */
function genChangeMembers(localArray, remoteArray) {
    var result = [];
    result = buildCommittee(remoteArray, localArray, 1, result);
    result = buildCommittee(localArray, remoteArray, 0, result);
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
function isStayInCommittee(members,user) {

    //.console.log(members);
    //console.log(user);
    if (members.length == 0) {
        return false;
    }

    let find = false;
    members.forEach(function (item,index) {
        //console.log(item)
        if (item.owner == user) {
            find =  true;
        }
    })


    return find;
}

module.exports = {
    genChangeMembers,
    isValidChangeMembers,
    isStayInCommittee
}