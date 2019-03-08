/**
 * vote相关工具
 */
var logger = require("../../config/logConfig").getLogger("VoteUtil")
var utils = require('../../common/util/utils')
var chainApi = require("../chainApi")
var chainUtil = require("./chainUtil")
var constants = require("../../common/constant/constants")

/**
 * 判断是否需要给用户投票
 * user 投票人
 * voteUser 被投票人
 */
function findVoteRecord(tableData, user, voteUser) {

    if (utils.isNull(tableData)) {
        return false;
    }
    let rows = tableData.rows;
    if (utils.isNullList(rows)) {
        return false;
    }


    try {

        //logger.debug(tableData.rows.length);
        for (var i = 0; i < tableData.rows.length; i++) {
            //logger.debug("row "+i+":");
            //logger.debug(tableData.rows[i]);
            let row = rows[i];
            if (row.owner == voteUser) {
                for (var j = 0; j < row.provided_approvals.length; j++) {
                    if (row.provided_approvals[j].account == user) {
                        return true;
                    }
                }
            }

        }
    } catch (e) {
        logger.error("needvoteuser error", e);
    }

    return false;
}

var voteuserdata = {
    "rows": [{
        "owner": "user.122",
        "proposal_account": [{
            "account": "user.122",
            "owner_key": "UTR7FLuM19CdCppghCjeswxPWAQxQj2LZnrKsCNwoXmi1GBwwrHUU",
            "active_key": "UTR7FLuM19CdCppghCjeswxPWAQxQj2LZnrKsCNwoXmi1GBwwrHUU",
            "updateable": 1,
            "location": 0,
            "approve_num": 2
        }
        ],
        "provided_approvals": [{
            "account": "user.115",
            "last_vote_time": "2019-01-11T10:43:35",
            "resource_index": 0
        }
        ]
    }
    ],
    "more": false
};

//console.log(findVoteRecord(voteuserdata, "user.115", "user.122"));


/**
 * 判断是否需要给资源投票
 * @param tableData
 * @param user
 * @param changeResObj
 * @param chainConfig
 * @returns {boolean}
 */
function findVoteRes(tableData, user, changeResObj,chainConfig) {

    if (utils.isNull(tableData)) {
        return false;
    }
    let rows = tableData.rows;
    if (utils.isNullList(rows)) {
        return false;
    }


    try {

        //logger.debug(tableData.rows.length);
        for (var i = 0; i < tableData.rows.length; i++) {
            //logger.debug("row "+i+":");
            //logger.debug(tableData.rows[i]);
            let row = rows[i];
            //查找目标用户是否有投票记录
            if (row.owner == changeResObj.owner) {
                var provide_index = -1;
                //查找目标用户是否已经被我投了
                for (var j = 0; j < row.provided_approvals.length; j++) {
                    if (row.provided_approvals[j].account == user) {
                        provide_index = row.provided_approvals[j].resource_index;
                        break;
                    }
                }
                //目标用户已经被我投了,查找当时我投了资源数
                if (provide_index >=0) {
                    var resObj = row.proposal_resource[provide_index];
                    //如果之前的数值<=当前的数值，说明已经投了
                    if (chainUtil.isResourceChanged(resObj,changeResObj,chainConfig)) {
                        return true;
                    }
                }
            }

        }
    } catch (e) {
        logger.error("needvoteuser error", e);
    }

    return false;
}
var voteResList = {
    "rows": [{
        "owner": "user.112",
        "proposal_resource": [{
            "account": "user.112",
            "lease_num": 1,
            "end_time": "2019-01-30T12:21:40",
            "location": 0,
            "approve_num": 1
        },{
            "account": "user.112",
            "lease_num": 10,
            "end_time": "2019-01-30T12:21:40",
            "location": 0,
            "approve_num": 2
        }
        ],
        "provided_approvals": [{
            "account": "user.111",
            "last_vote_time": "2019-01-11T07:28:00",
            "resource_index": 0
        },{
            "account": "user.113",
            "last_vote_time": "2019-01-11T07:52:00",
            "resource_index": 1
        },{
            "account": "user.114",
            "last_vote_time": "2019-01-11T07:53:30",
            "resource_index": 1
        }
        ]
    }
    ],
    "more": false
};
//console.log(findVoteRes(voteResList,"user.113","user.112",1,null));

/**
 * 对比两张表的数据找出需要更新的对象
 * @param subResList
 * @param mainResList
 * @returns {Array}
 */
async function genVoteResList(subResList, mainResList, chainConfig) {
    var result = [];

    try {

        var subRows = utils.isNotNull(subResList.rows) ? subResList.rows : []; //子链列表
        var mainRows = utils.isNotNull(mainResList.rows) ? mainResList.rows : [];
        []; //主链列表
        var index_sub = 0;
        var index_main = 0;
        while (index_main < mainRows.length) {

            //子链对象
            let subResObj = getResObj(subRows, index_sub);
            //主链对象
            let mainResObj = getResObj(mainRows, index_main);

            //主链对象在子链找不到（子链到底 || 主链对象owner<子链当前对象owner）
            if (utils.isNull(subResObj) || mainResObj.owner < subResObj.owner) {
                //查看子链上是否有该用户才处理
                if (!utils.isNull(await chainApi.getAccount(chainConfig.configSub.httpEndpoint, mainResObj.owner))) {
                    result.push(mainResObj);
                } else {
                    logger.info("can't find account:"+mainResObj.owner);
                }

                index_main++;

            } else if (mainResObj.owner > subResObj.owner) {
                //主链对象owner > 子链当前对象owner
                index_sub++;

            } else if (mainResObj.owner == subResObj.owner) {
                //主链对象和子链对象一致，比较资源大小
                if (chainUtil.isResourceChanged(mainResObj,subResObj,chainConfig)) {
                    result.push(mainResObj);
                }

                index_main++;
                index_sub++;
            }

        }
    } catch (e) {
        logger.error("genVoteResList error:", e);
    }
    return result;
}



/**
 *
 * @param rows
 * @param index
 * @returns {*}
 */
function getResObj(rows, index) {
    if (index < rows.length) {
        return rows[index];
    }

    return null;
}


/**
 * 判断是否已经投过这个委员会成员
 * user 投票人
 * voteUser 被投票委员会人
 */
function findVoteCommitee(tableData, user, voteUser,adddel_miner) {

    if (utils.isNull(tableData)) {
        return false;
    }
    let rows = tableData.rows;
    if (utils.isNullList(rows)) {
        return false;
    }


    try {

        //logger.debug(tableData.rows.length);
        for (var i = 0; i < tableData.rows.length; i++) {
            //logger.debug("row "+i+":");
            //logger.debug(tableData.rows[i]);
            let row = rows[i];
            if (row.owner == voteUser) {
                for (var j = 0; j < row.provided_approvals.length; j++) {
                    if (row.provided_approvals[j].account == user) {
                        var index = row.provided_approvals[j].resource_index;
                        if (utils.isNotNull(row.proposal_miner) && utils.isNotNull(row.proposal_miner[index]) && row.proposal_miner[index].adddel_miner == adddel_miner) {
                            return true;
                        }
                    }
                }
            }

        }
    } catch (e) {
        logger.error("findVoteCommitee error", e);
    }

    return false;
}

function getMaxValidWorldState(rows) {

    let result = {block_num : -1, hash_v : []};

    try {
        if (utils.isNullList(rows)) {
            return null;
        } else {
            for (var i = rows.length - 1; i >= 0; i--) {
                let obj = rows[i];
                if (utils.isNullList(obj.hash_v) == false) {
                    for (var t = 0; t < obj.hash_v.length; t++) {
                        if (obj.hash_v[t].valid == 1) {
                            result.block_num = obj.block_num;
                            result.hash_v.push(obj.hash_v[t]);
                            return result;
                        }
                    }

                }

            }

        }

    } catch (e) {
        logger.error("getMaxValidWorldState error:",e);
    }

    return null;
}


module.exports = {
    findVoteRecord,
    genVoteResList,
    findVoteCommitee,
    findVoteRes,
    getMaxValidWorldState,
}
