/**
 * vote相关工具
 */
var logger = require("../../config/logConfig").getLogger("VoteUtil")
var utils = require('../../common/util/utils')
var chainApi = require("../chainApi")

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
 * user 投票人
 * voteUser 被投票人
 * lease_num 资源份数
 * expire_date 时间
 */
function findVoteRes(tableData, user, voteUser, lease_num, expire_date) {

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
            if (row.owner == voteUser) {
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
                    if (resObj.lease_num >= lease_num) {
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
                if (!utils.isNull(await chainApi.getAccount(chainConfig.configSub, mainResObj.owner))) {
                    result.push(mainResObj);
                }

                index_main++;

            } else if (mainResObj.owner > subResObj.owner) {
                //主链对象owner > 子链当前对象owner
                index_sub++;

            } else if (mainResObj.owner == subResObj.owner) {
                //主链对象和子链对象一致，比较资源大小
                if (mainResObj.lease_num > subResObj.lease_num) {
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

var subResList = {
    rows: [{
        owner: 'hello',
        lease_num: 10,
        start_time: '2019-01-10T11:53:50',
        end_time: '2019-04-20T11:53:50'
    },
        {
            owner: 'root',
            lease_num: 10,
            start_time: '2019-01-10T11:53:50',
            end_time: '2019-04-20T11:53:50'
        },
        {
            owner: 'root1',
            lease_num: 10,
            start_time: '2019-01-10T11:53:50',
            end_time: '2019-04-20T11:53:50'
        },
        {
            owner: 'root2',
            lease_num: 10,
            start_time: '2019-01-10T11:53:50',
            end_time: '2019-04-20T11:53:50'
        },
        {
            owner: 'root3',
            lease_num: 10,
            start_time: '2019-01-10T11:53:50',
            end_time: '2019-04-20T11:53:50'
        },
        {
            owner: 'root4',
            lease_num: 10,
            start_time: '2019-01-10T11:53:50',
            end_time: '2019-04-20T11:53:50'
        },
        {
            owner: 'root5',
            lease_num: 10,
            start_time: '2019-01-10T11:53:50',
            end_time: '2019-04-20T11:53:50'
        },
        {
            owner: 'user.112',
            lease_num: 1,
            start_time: '2019-01-10T12:39:40',
            end_time: '2019-01-30T12:39:40'
        }]
};
var mainResList = {
    rows: [{
        owner: 'user.111',
        lease_num: 800,
        start_time: '2019-01-10T11:51:30',
        end_time: '2019-01-30T11:51:30'
    }]
}

//logger.error(genVoteResList(subResList, mainResList));

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
function findVoteCommitee(tableData, user, voteUser) {

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
        logger.error("findVoteCommitee error", e);
    }

    return false;
}


module.exports = {
    findVoteRecord,
    genVoteResList,
    findVoteCommitee,
    findVoteRes
}