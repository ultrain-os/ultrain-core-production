const {U3} = require('u3.js');
const {createU3, format} = U3;
const axios = require('axios')
var qs = require('qs');


/**
 * 链相关操作的api
 */
var logger = require("../config/logConfig").getLogger("ChainApi");
var logUtil = require("../common/util/logUtil")
var constant = require("../common/constant/constants")
var utils = require("../common/util/utils");
var hashUtil = require("../common/util/hashUtil");
var sleep = require("sleep")

/**
 * 获取主网主链Chain Id
 * @param config
 * @returns {Promise<*>}
 */
const getChainId = async (config) => {

    try {
        var u3 = createU3({...config, sign: true, broadcast: true});
        var blockInfo = await u3.getBlockInfo("1");
        return blockInfo.action_mroot;
    } catch (e) {
        logger.error("getChainId error:", utils.logNetworkError(e));
    }

    return null;

}

const getChainIdByAllSeed = async (config,seedList) => {

    try {
        var u3 = createU3({...config, sign: true, broadcast: true});
        var blockInfo = await u3.getBlockInfo("1");
        return blockInfo.action_mroot;
    } catch (e) {
        logger.error("getChainId error:", utils.logNetworkError(e));

        for (let i =0;i<seedList.length;i++) {
            try {
                config.httpEndpoint = seedList[i];
                var u3 = createU3({...config, sign: true, broadcast: true});
                var blockInfo = await u3.getBlockInfo("1");
                return blockInfo.action_mroot;
            } catch (e) {
                logger.error("getChainId error:", utils.logNetworkError(e));
            }
        }
    }

    return null;

}


/**
 * 获取当前链的chainName
 * @param configSub
 * @returns {Promise<*>}
 */
const getChainName = async (url) => {
    let chainName = null;
    try {

        const params = {
            "code": constant.contractConstants.ULTRAINIO,
            "scope": constant.contractConstants.ULTRAINIO,
            "table": constant.tableConstants.GLOBAL,
            "json": true,
            "key_type": "name"
        };
        logger.info(params);
        let res = await axios.post(url + "/v1/chain/get_table_records", params);
        logger.info("global table :", res.data);
        if (utils.isNotNull(res) && res.data.rows.length > 0) {
            chainName = res.data.rows[0].chain_name;
            logger.info("getChainName from table ",chainName);
            logger.info("getChainName from table ",res.data.rows[0]);
        }
    } catch (e) {
        logger.error("getChainName error,", utils.logNetworkError(e));
    }

    return chainName;
}

const getBlockHeaderInfo = async (httpEndpoint,scope,table_key) => {
    let rows = [];
    try {
        const params = {
            "code": constant.contractConstants.ULTRAINIO,
            "scope": scope,
            "table": constant.tableConstants.BLOCK_HEADER,
            "json": true,
            "table_key_type": "uint64",
            "table_key": table_key,
        };
        let res = await axios.post(httpEndpoint + "/v1/chain_info/get_table_records", params);
        return res.data.rows;
    } catch (e) {
        logger.error("getBlockHeaderInfo error,",e);
    }

    return rows;
}

/**
 *
 * @param httpEndpoint
 * @param blockId
 * @returns {Promise<*>}
 */
const getBlockInfoData = async (httpEndpoint,blockId) => {
    let blockObj = null;
    try {
        const params = {
            "block_num_or_id" : blockId
        }
        let res = await axios.post(httpEndpoint + "/v1/chain_info/get_block_info", params);
        blockObj = res.data;
    } catch (e) {
        logger.error("get block info data error:",e);
    }

    return blockObj;
}

/**
 * 获取已同步最小块的信息
 * @param config
 * @param scope
 * @param table_key
 * @returns {Promise<*>}
 */
const getMinBlockHeaderInfo = async (httpEndpoint,scope) => {
    let blockId = 0;
    try {
        const params = {
            "code": constant.contractConstants.ULTRAINIO,
            "scope": scope,
            "table": constant.tableConstants.BLOCK_HEADER,
            "json": true,
            "table_key_type": "uint64",
            "limit": 1,
        };
        let res = await axios.post(httpEndpoint + "/v1/chain_info/get_table_records", params);
        logger.debug("getMinBlockHeaderInfo res:",res.data);
        if (res.data.rows.length > 0) {
            return res.data.rows[0].block_number;
        }
    } catch (e) {
        logger.error("getBlockHeaderInfo error,",e);
    }

    return blockId;
}

/**
 * 根据user 获取chain name
 * @param user
 * @returns {Promise<*|number|Location|string|WorkerLocation>}
 */
const getChainInfo = async function initChainName(config, user) {

    let result = null;
    logger.debug("getChainInfo config :", config);
    try {
        var u3 = createU3({...config, sign: true, broadcast: true});
        result = await u3.getProducerInfo({"owner": user});
        logger.debug("getChainInfo", result);
    } catch (e) {
        logger.error("getChainInfo error:", utils.logNetworkError(e));
    }

    return result;

}


/**
 * 获取账号
 * @param config
 * @param accountName
 * @returns {Promise<null>}
 */
async function getAccount(port, accountName) {
    try {
        const rs = await axios.post("http://127.0.0.1:"+port + "/v1/chain_info/get_account_exist", {"account_name": accountName});
        let res =  rs.data;
        if (res.is_exist == true) {
            return true;
        }
    } catch (e) {
        logger.error("getAccount("+accountName+") error",utils.logNetworkError(e));
    }
    return null;

}

/**
 * 获取本地producer列表
 * @returns {Promise<Array>}
 */
async function getProducerLists(prefix) {

    var result = [];
    try {
        const params = {
            "json": "true",
            "lower_bound": "0",
            "limit": 10000,
            "chain_name": constant.chainNameConstants.MAIN_CHAIN_NAME
        };
        logger.info("getProducerLists",prefix + "/v1/chain_info/get_producers");
        const rs = await axios.post(prefix + "/v1/chain_info/get_producers", params);

        logger.debug("getProducerLists:", rs.data.rows);
        var rows = rs.data.rows;
        for (var i in rows) {
            var row = rows[i];
            if (row.chain_name == constant.chainNameConstants.MAIN_CHAIN_NAME) {
                result.push({
                    owner: row.prod_detail.owner,
                    miner_pk: row.prod_detail.producer_key,
                    bls_pk: row.prod_detail.bls_key,
                });
            }

        }

        //logger.debug("getProducerLists result=", result);

    } catch (e) {
        logger.error("getProducerLists error:",utils.logNetworkError(e));
    }
    return result;
}

/**
 * 调用智能合约的入口方法
 * @param config 配置文件
 * @param contractName
 * @param actionName
 * @param params
 * @param accountName
 * @param privateKey
 * @returns {Promise<void>}
 */
async function contractInteract(config, contractName, actionName, params, accountName, privateKey) {
    try {

        logger.debug("contractInteract start ", contractName);
        const keyProvider = [privateKey];
        const u3 = createU3({...config, keyProvider});

        logger.debug("keyProvider:", keyProvider);

        const contract = await u3.contract(contractName);
        //let ee = error;
        //logger.debug("contract=", JSON.stringify(contract.fc.abi.structs));
        if (!contract) {
            throw new Error("can't found contract " + contractName);
        }
        if (!contract[actionName] || typeof contract[actionName] !== 'function') {
            throw new Error("action doesn't exist:" + actionName);
        }
        const data = await contract[actionName](params, {
            authorization: [`${accountName}@active`],
        });
        logger.debug('contractInteract success :', actionName);
        return data;
    } catch (err) {
        logger.debug('contractInteract error :', actionName);
        logger.error('' + actionName + ' error :', err);
    }
    return null;
}


/**
 * 通过主链获取子链的userbulletin
 * @returns {Promise<*>}
 */
getUserBulletin = async (config, chain_name) => {
    try {
        var u3 = createU3({...config, sign: true, broadcast: true});
        return await u3.getUserBulletin({"chain_name": chain_name});
    } catch (e) {
        logger.error("get user bulletin error :", utils.logNetworkError(e));
    }

    return null;

}

/**
 * 根据链名称获取种子ip
 * @param chain
 * @returns {Promise<string>}
 */
getChainSeedIP = async (chainName, chainConfig) => {

    // logger.debug(chainConfig.seedIpConfig);
    // logger.debug(chainName);
    try {
        if (utils.isNotNull(chainConfig.seedIpConfig) && chainConfig.seedIpConfig.length > 0) {

            //简单模式（不分内外网ip）
            let simpleModeFlag = true;

            if (chainConfig.seedIpConfig.length > 0) {
                let config = chainConfig.seedIpConfig[0];
                if (utils.isNotNull(config.seedList) && config.seedList.length > 0) {
                    simpleModeFlag = false;
                }
            }

            logger.error("ChainSeedIP simple mode:",simpleModeFlag);
            if (simpleModeFlag == true) {
                for (let i = 0; i < chainConfig.seedIpConfig.length; i++) {
                    //logger.debug(chainConfig.seedIpConfig[i])
                    if (chainConfig.seedIpConfig[i].chainName == chainName) {
                        return chainConfig.seedIpConfig[i].seedIp;
                    }
                }
            } else {
                //复杂模式，需要检查内外网ip的信息
                let privateIPList = [];
                let publicIPList = [];

                for (let i = 0; i < chainConfig.seedIpConfig.length; i++) {
                    //logger.debug(chainConfig.seedIpConfig[i])
                    if (chainConfig.seedIpConfig[i].chainName == chainName) {
                        for (let j =0;j<chainConfig.seedIpConfig[i].seedList.length;j++) {
                            privateIPList.push(chainConfig.seedIpConfig[i].seedList[j].privateIp);
                            if (utils.isNotNull(chainConfig.seedIpConfig[i].seedList[j].publicIp) && chainConfig.seedIpConfig[i].seedList[j].publicIp.length > 10) {
                                publicIPList.push(chainConfig.seedIpConfig[i].seedList[j].publicIp);
                            }
                        }
                    }
                }

                /**
                 * 检查内网节点是否连通
                 */
                logger.info("chain（"+chainName+"）private ip list：",privateIPList);
                logger.info("chain（"+chainName+"）public ip list：",publicIPList);

                let privateFlag = await checkSeedReady(privateIPList,chainConfig.nodPort);
                if (privateFlag == true) {
                    logger.info("I am in LAN("+chainName+") , use private ip list");
                    return privateIPList;
                } else {
                    logger.info("I am not in LAN("+chainName+") , use public ip list");
                    return publicIPList;
                }

            }

        }

    } catch (e) {
        logger.error("get chain seed ip error:", e);
    }
    return null;
}

checkSeedReady = async (ipList,nodPort) => {

    logger.info("check seed ready start....");
    try {
        if (ipList.length > 0) {
            for (let i=0;i<ipList.length;i++) {
                try {
                    let url = "http://"+ipList[i]+":"+nodPort+"/v1/chain_info/get_chain_info";
                    let res = await axios.post(url, {},{timeout: 1000*10});
                    logger.info("check seed ready url（"+ipList[i]+"） chain id:",res.data.chain_id);
                    if (utils.isNotNull(res.data.chain_id)) {
                        logger.info("check seed("+ipList[i]+") success,chainid is ",res.data.chain_id);
                        return true;
                    } else {
                        logger.error("check seed("+ipList[i]+") error,chainid is null");
                    }
                } catch (e) {
                    logger.error("check seed ready url（"+ipList[i]+"） error",utils.logNetworkError(e));
                }
            }
        }

    } catch (e) {
        logger.error("checkSeedReady eroor;",e);
    }


    return false;
}
/**
 * 获取链httplist
 * @param chainName
 * @param chainConfig
 * @returns {Promise<Array>}
 */
getChainHttpList = async (chainName, chainConfig) => {

    let seedList = await getChainSeedIP(chainName, chainConfig);
    let chainHttpList = [];
    if (utils.isNullList(seedList) == false) {
        for (let i = 0; i < seedList.length; i++) {
            let url = "http://" + seedList[i] + ":"+chainConfig.nodPort;
            chainHttpList.push(url);
        }
    }

    return chainHttpList;
}

/**
 * 根据链名称获取白名单公钥信息（genesis+seed）
 * @param chain
 * @returns {Promise<string>}
 */
getChainPeerKey = async (chainName, chainConfig) => {

    // logger.debug(chainConfig.seedIpConfig);
    // logger.debug(chainName);
    try {
        if (utils.isNotNull(chainConfig.seedIpConfig)) {
            for (let i = 0; i < chainConfig.seedIpConfig.length; i++) {
                //logger.debug(chainConfig.seedIpConfig[i])
                if (chainConfig.seedIpConfig[i].chainName == chainName) {
                    return chainConfig.seedIpConfig[i].peerKeys;
                }
            }
        }

    } catch (e) {
        logger.error("get getChainPeerKey error:", e);
    }
    return [];
}

/**
 *
 * @param prefix
 * @param path
 * @param param
 * @param backUplist
 * @returns {Promise<*>}
 */
multiRequest = async function (prefix, path, params, prefixlist) {
    logger.debug("multiRequest:", prefix);
    logger.debug("multiRequest:", prefixlist);
    let res = null;
    try {
        res = await axios.post(prefix + path, params);
        if (res.status == 200) {
            logger.debug("multiRequest(" + path + ") success:", res);
            return res;
        } else {
            logger.error("multiRequest(" + path + ") error:", res);
        }
    } catch (e) {
        logger.error("multiRequest("+path+") error:", utils.logNetworkError(e));
        // if (prefixlist.length > 0) {
        //     for (let i = 0; i < prefixlist.length; i++) {
        //         let newPrefix = prefixlist[i];
        //         try {
        //             logger.info("multiRequest(" + path + "), user new url:" + newPrefix);
        //             res = await axios.post(newPrefix + path, params);
        //             if (res.status == 200) {
        //                 return res;
        //             }
        //         } catch (e) {
        //             logger.error("multiRequest error:", e);
        //             logger.error("multiRequest(" + path + ") error:", res);
        //         }
        //     }
        // }
    }

    return res;
}

/**
 * 获取表数据
 * @param config
 * @param code
 * @param scope
 * @param table
 * @param limit
 * @param table_key
 * @param lower_bound
 * @param upper_bound
 * @returns {Promise<*>}
 */
getTableInfo = async (httpEndpoint, code, scope, table, limit, table_key, lower_bound, upper_bound) => {
    try {
        const params = {"code": code, "scope": scope, "table": table, "json": true, "key_type": "name"};
        logger.debug(params);
        if (utils.isNotNull(limit)) {
            params.limit = limit;
        }
        if (utils.isNotNull(table_key)) {
            params.table_key = table_key;
        }
        if (utils.isNotNull(lower_bound)) {
            params.lower_bound = lower_bound;
        }
        if (utils.isNotNull(upper_bound)) {
            params.upper_bound = upper_bound;
        }
        logger.debug("url:",httpEndpoint+"/v1/chain_info/get_table_records");
        logger.debug("params:",params);

        let res = await multiRequest(httpEndpoint, "/v1/chain_info/get_table_records", params, []);
        //logger.info(res);
        return res.data;
    } catch (e) {
        logger.error("get_table_records error:", utils.logNetworkError(e));
    }

    return null;
}


getTableScopeInfo = async (httpEndpoint, code, scope, table, limit, table_key, lower_bound, upper_bound) => {
    try {
        const params = {"code": code, "scope": sjkuhjhkbjbkhncope, "table": table, "json": true, "key_type": "name"};
        logger.debug(params);
        if (utils.isNotNull(limit)) {
            params.limit = limit;
        }
        if (utils.isNotNull(table_key)) {
            params.table_key = table_key;
        }
        if (utils.isNotNull(lower_bound)) {
            params.lower_bound = lower_bound;
        }
        if (utils.isNotNull(upper_bound)) {
            params.upper_bound = upper_bound;
        }
        //logger.info("url:",httpEndpoint+"/v1/chain/get_table_by_scope");
        //logger.info("params:",params);

        let res = await multiRequest(httpEndpoint, "/v1/chain/get_table_by_scope", params, []);
        //logger.info(res.data);
        return res.data;
    } catch (e) {
        logger.error("get_table_by_scope error:", utils.logNetworkError(e));
    }

    return null;
}


/**
 * getCommitteeBulletin
 *
 * @param config
 * @param chain_name
 * @returns {Promise<Array>}
 */
getCommitteeBulletin = async (config,chain_name) => {
    let bulletinList = [];
    try {
        const params = {"chain_name": chain_name};
        let res = await multiRequest(config.httpEndpoint, "/v1/chain/get_committee_bulletin", params, config.seedHttpList);
        logger.debug("getCommitteeBulletin res:",res.data);
        bulletinList = res.data;
    } catch (e) {
        logger.error("getCommitteeBulletin error:",e);
    }

    return bulletinList;
}

/**
 * 获取全表数据
 * @param config
 * @param code
 * @param scope
 * @param table
 * @returns {Promise<*>}
 */
getTableAllData = async (config, code, scope, table, pk) => {
    let tableObj = {rows: [], more: false};
    let count = 10000; //MAX NUM
    let limit = 1000; //limit
    let finish = false;
    let lower_bound = null;
    var index = 0;
    try {
        while (finish == false) {
            logger.info("table: " + table + " scope:" + scope + " lower_bound(request)：" + lower_bound);
            index++;
            let tableinfo = await getTableInfo(config.httpEndpoint, code, scope, table, limit, null, lower_bound, null);
            logger.debug("tableinfo:" + table + "):", tableinfo);
            if (utils.isNullList(tableinfo.rows) == false) {
                for (let i = 0; i < tableinfo.rows.length; i++) {
                    if (tableinfo.rows[i][pk] != lower_bound || lower_bound == null) {
                        tableObj.rows.push(tableinfo.rows[i]);
                    }
                }

                if (utils.isNotNull(pk)) {
                    lower_bound = tableinfo.rows[tableinfo.rows.length - 1][pk];
                }

                logger.info("table: " + table + " scope:" + scope + " lower_bound(change)：" + lower_bound);
            }

            //查看是否还有
            finish = true;
            if (utils.isNotNull(tableinfo.more) && tableinfo.more == true) {
                finish = false;
            }
            logger.debug("tableinfo more：" + tableinfo.more);
            if (index * limit >= count) {
                logger.info("table: " + table + " count > " + count + " break now!");
                break;
            }

        }
    } catch (e) {
        logger.error("getTableAllData error:", utils.logNetworkError(e));
    }

    logger.debug("getTableAllData(" + table + "):", tableObj);
    return tableObj;

}


getTableAllScopeData = async (config, code, scope, table, pk) => {
    let tableObj = {rows: [], more: false};
    let count = 1000000; //MAX NUM
    let limit = 100; //limit
    let finish = false;
    let lower_bound = null;
    var index = 0;
    try {
        while (finish == false) {
            logger.info("table: " + table + " scope:" + scope + " lower_bound(request)：" + lower_bound);
            index++;
            let tableinfo = await getTableScopeInfo(config.httpEndpoint, code, scope, table, limit, null, lower_bound, null);
            logger.debug("tableinfo:" + table + "):", tableinfo);
            if (utils.isNullList(tableinfo.rows) == false) {
                for (let i = 0; i < tableinfo.rows.length; i++) {
                    if (tableinfo.rows[i][pk] != lower_bound || lower_bound == null) {
                        tableObj.rows.push(tableinfo.rows[i]);
                    }
                }

                if (utils.isNotNull(pk)) {
                    lower_bound = tableinfo.rows[tableinfo.rows.length - 1][pk];
                }

                logger.info("table: " + table + " scope:" + scope + " lower_bound(change)：" + lower_bound);
            }

            //查看是否还有
            finish = true;
            if (utils.isNotNull(tableinfo.more) && tableinfo.more != '0' && tableinfo.more != 0) {
                finish = false;
            }
            logger.info("tableinfo more：" + tableinfo.more);
            if (index * limit >= count) {
                logger.info("table: " + table + " count > " + count + " break now!");
                break;
            }

        }
    } catch (e) {
        logger.error("getTableAllData error:", utils.logNetworkError(e));
    }

    logger.debug("getTableAllData(" + table + "):", tableObj);
    return tableObj;

}

/**
 * 返回不同子链对应的非noneproducer的链接点
 * @param chainId
 * @returns {Promise<string>}
 */
getSubchanEndPoint = async (chainName, chainConfig) => {

    // logger.debug(chainConfig.seedIpConfig);
    // logger.debug(chainName);
    try {
        if (utils.isNotNull(chainConfig.seedIpConfig)) {
            for (let i = 0; i < chainConfig.seedIpConfig.length; i++) {
                //logger.debug(chainConfig.seedIpConfig[i])
                if (chainConfig.seedIpConfig[i].chainName == chainName) {
                    return chainConfig.seedIpConfig[i].subchainHttpEndpoint;
                }
            }
        }

    } catch (e) {
        logger.error("getSubchanEndPoint error:", e);
    }
    return "";
}

/**
 * 返回不同子链对应的monitor-server-endpoint的链接点
 * @param chainId
 * @returns {Promise<string>}
 */
getSubchanMonitorService = async (chainName, chainConfig) => {

    // logger.debug(chainConfig.seedIpConfig);
    // logger.debug(chainName);
    try {
        if (utils.isNotNull(chainConfig.seedIpConfig)) {
            for (let i = 0; i < chainConfig.seedIpConfig.length; i++) {
                //logger.debug(chainConfig.seedIpConfig[i])
                if (chainConfig.seedIpConfig[i].chainName == chainName) {
                    return chainConfig.seedIpConfig[i].monitorServerEndpoint;
                }
            }
        }

    } catch (e) {
        logger.error("getSubchanMonitorService error:", e);
    }
    return "";
}

getSubchainConfig = async (chainName, chainConfig) => {

    // logger.debug(chainConfig.seedIpConfig);
    // logger.debug(chainName);
    try {
        if (utils.isNotNull(chainConfig.seedIpConfig)) {
            for (let i = 0; i < chainConfig.seedIpConfig.length; i++) {
                //logger.debug(chainConfig.seedIpConfig[i])
                if (chainConfig.seedIpConfig[i].chainName == chainName) {
                    return chainConfig.seedIpConfig[i];
                }
            }
        }

    } catch (e) {
        logger.error("getSubchainConfig error:", e);
    }
    return "";
}

/**
 * 获取子链近半小时更新的资源信息
 * @param chainName
 * @param chainConfig
 * @returns {Promise<*>}
 */
getSubchainResource = async (chainName, chainConfig) => {

    try {
        const params = {"chain_name": chainName};
        let rs = await multiRequest(chainConfig.config.httpEndpoint, "/v1/chain/get_subchain_resource", params, chainConfig.config.seedHttpList);
        return rs.data;
    } catch (e) {
        logger.error("getSubchainResource error:", utils.logNetworkError(e));
    }

    return null;
}

/**
 *
 * @param config
 * @returns {Promise<*>}
 */
const getChainBlockDuration = async (config) => {
    try {
        const rs = await multiRequest(config.httpEndpoint, "/v1/chain_info/get_chain_info", {}, config.seedHttpList);
        return rs.data.block_interval_ms;
    } catch (e) {
        logger.error("getChainBlockDuration error,", utils.logNetworkError(e));
    }

    return null;

}

/**
 *
 * @param config
 * @returns {Promise<*>}
 */
const getHeadBlockNum = async (port) => {
    try {
        const rs = await multiRequest("http://127.0.0.1:"+port, "/v1/chain_info/get_chain_info", {}, []);
        return rs.data.head_block_num;
    } catch (e) {
        logger.error("getHeadBlockNum error,", utils.logNetworkError(e));
    }

    return null;

}

/**
 *
 * @param config
 * @returns {Promise<*>}
 */
const getMasterHeadBlockNum = async (httpendpoint) => {
    try {
        const rs = await multiRequest(httpendpoint, "/v1/chain_info/get_chain_info", {}, []);
        return rs.data.head_block_num;
    } catch (e) {
        logger.error("getMasterHeadBlockNum error,", utils.logNetworkError(e));
    }

    return null;

}

/**
 *
 * @param config
 * @returns {Promise<*>}
 */
const getHeadBlockProposer = async (config) => {
    try {
        const rs = await multiRequest(config.httpEndpoint, "/v1/chain_info/get_chain_info", {}, config.seedHttpList);
        return rs.data.head_block_proposer;
    } catch (e) {
        logger.error("getHeadBlockProposer error,", utils.logNetworkError(e));
    }

    return null;

}

/**
 *
 * @param config
 * @returns {Promise<*>}
 */
const getServerVersion = async (port) => {
    let path = "http://127.0.0.1:"+port+"/v1/chain_info/get_chain_info"
    try {
        const rs = await axios.post(path);
        logger.debug("get get_chain_info  ",rs.data);
        if (rs.status == 200) {
            return rs.data.server_version;
        } else {
            logger.error("request node serverVersion error code (" + path + ")");
        }
    } catch (e) {
        logger.error("getServerVersion error,", utils.logNetworkError(e));
    }

    return null;
}


/**
 * monitor check in
 * @param url
 * @param param
 * @returns {Promise<*>}
 */
monitorCheckIn = async (url, param) => {
    try {
        logger.info("monitorCheckIn param:", qs.stringify(param));
        const rs = await axios.post(url + "/filedist/checkIn", qs.stringify(param));
        logger.info("monitorCheckIn result:", rs.data);
        return rs.data;
    } catch (e) {
        logger.error("monitorCheckIn error,", utils.logNetworkError(e));
    }
}

/**
 * confirmBlockCheckIn
 * @param url
 * @param param
 * @returns {Promise<*>}
 */
confirmBlockCheckIn = async (url,param) => {
    try {
        logger.info("confirmBlockCheckIn param:", qs.stringify(param));
        const rs = await axios.post(url + "/alert/uploadConfirmBlock", qs.stringify(param));
        logger.info("confirmBlockCheckIn result:", rs.data);
        return rs.data;
    } catch (e) {
        logger.error("confirmBlockCheckIn error,", utils.logNetworkError(e));
    }
}

/**
 * ramUsageCheckIn
 * @param url
 * @param param
 * @returns {Promise<*>}
 */
ramUsageCheckIn = async (url,param) => {
    try {
        logger.info("ramUsageCheckIn param:", qs.stringify(param));
        const rs = await axios.post(url + "/filedist/addRamUsage", qs.stringify(param));
        logger.info("ramUsageCheckIn result:", rs.data);
        return rs.data;
    } catch (e) {
        logger.error("addRamUsage error,", utils.logNetworkError(e));
    }
}



/**
 * uploadugas
 * @param url
 * @param param
 * @returns {Promise<*>}
 */
uploadugas = async (url,param) => {
    try {
        logger.info("uploadugas param:", qs.stringify(param));
        const rs = await axios.post(url + "/filedist/addUgasInfo", qs.stringify(param));
        logger.info("uploadugas result:", rs.data);
        return rs.data;
    } catch (e) {
        logger.error("uploadugas error,", utils.logNetworkError(e));
    }
}


/**
 *
 * @param url
 * @param param
 * @returns {Promise<*>}
 */
checkDeployFile = async (url, param) => {
    try {
        logger.info("checkDeployFile param:", qs.stringify(param));
        const rs = await axios.post(url + "/filedist/checkDeployInfo", qs.stringify(param));
        logger.info("checkDeployFile result:", rs.data);
        return rs.data;
    } catch (e) {
        logger.error("checkDeployFile error,", utils.logNetworkError(e));
    }
}

/**
 *
 * @param url
 * @param param
 * @returns {Promise<*>}
 */
finsihDeployFile = async (url, param) => {
    try {
        logger.debug("finishDeployInfo param:", qs.stringify(param));
        const rs = await axios.post(url + "/filedist/finishDeployInfo", qs.stringify(param));
        logger.info("finishDeployInfo result:", rs.data);
        return rs.data;
    } catch (e) {
        logger.error("finishDeployInfo error,", utils.logNetworkError(e));
    }
}

/**
 *
 * @returns {Promise<void>}
 */
getSubchainList = async (chainConfig) => {
    let apiArray = [];
    try {
        if (utils.isNotNull(chainConfig.seedIpConfig)) {
            for (let i = 0; i < chainConfig.seedIpConfig.length; i++) {
                //logger.debug(chainConfig.seedIpConfig[i])
                apiArray.push({
                    "chainName": chainConfig.seedIpConfig[i].chainName,
                    "httpEndpoint": chainConfig.seedIpConfig[i].subchainHttpEndpoint
                })
            }
        }
    } catch (e) {
        logger.error("getSubchainList error,", e);
    }

    return apiArray;
}

getSyncBlockChainList = async (chainConfig, isMainChain) => {
    if (isMainChain) {
        return await getSubchainList(chainConfig);
    } else {
        return [{
            "chainName": constant.chainNameConstants.MAIN_CHAIN_NAME,
            "httpEndpoint": chainConfig.config.httpEndpoint
        }];
    }
}


/**
 * 获取目标chain中已同步的块信息
 * @param configTemp
 * @param targetChainHttp
 * @param targetChainName
 * @param searchChainName
 * @returns {Promise<*>}
 */
const getTargetChainBlockNum = async (configTemp, targetChainHttp, targetChainName, searchChainName) => {

    try {
        configTemp.httpEndpoint = targetChainHttp;
        var u3Temp = createU3({...configTemp, sign: true, broadcast: true});
        var subchainBlockNumResult = await u3Temp.getSubchainBlockNum({"chain_name": searchChainName});
        return subchainBlockNumResult;
    } catch (e) {
        logger.error("getTargetChainBlockNum error:", utils.logNetworkError(e))
    }
}

/**
 *
 * @param url
 * @param param
 * @returns {Promise<*>}
 */
addSwitchLog = async (url, param) => {
    try {
        logger.debug("addSwitchLog param:", qs.stringify(param));
        const rs = await axios.post(url + "/filedist/addSwitchLog", qs.stringify(param));
        logger.info("addSwitchLog result:", rs.data);
        return rs.data;
    } catch (e) {
        logger.error("addSwitchLog error,", utils.logNetworkError(e));
    }
}

/**
 *
 * @param url
 * @param param
 * @returns {Promise<*>}
 */
addRestartLog = async (url, param) => {
    try {
        logger.debug("addRestartLog param:", qs.stringify(param));
        const rs = await axios.post(url + "/filedist/addRestartLog", qs.stringify(param));
        logger.info("addRestartLog result:", rs.data);
        return rs.data;
    } catch (e) {
        logger.error("addRestartLog error,", utils.logNetworkError(e));
    }
}

/**
 *
 * @returns {Promise<*>}
 */
getSubchainBlockNum = async (config, chain_name) => {
    try {
        var u3 = createU3({...config, sign: true, broadcast: true});
        return await u3.getSubchainBlockNum({"chain_name": chain_name});
    } catch (e) {
        logger.error("getSubchainBlockNum :", e);
    }

    return null;

}

/**
 * 获取子链中主链的块高
 * @returns {Promise<*>}
 */
getMasterBlockNum = async (port) => {

    try {
        const rs = await multiRequest("http://127.0.0.1:"+port, "/v1/chain_info/get_master_block_num", {}, []);
        logger.info("getMasterBlockNum data:",rs.data);
        return rs.data;
    } catch (e) {
        logger.error("getMasterBlockNum error,", utils.logNetworkError(e));
    }

}


/**
 *
 * @returns {Promise<*>}
 */
getSubchainCommittee = async (config, chain_name) => {
    try {
        var u3 = createU3({...config, sign: true, broadcast: true});
        return await u3.getSubchainCommittee({"chain_name": chain_name});
    } catch (e) {
        logger.error("getSubchainCommittee :", e);
    }

    return null;

}

var maxCheckSeedTime =3;

var checkSubSeedTime =0;

var checkMainSeedTime = 0;

checkSubchainSeed = async (chainConfig) => {

    let path = chainConfig.configSub.httpEndpoint + "/v1/chain_info/get_chain_info"
    try {
        let res = await axios.post(path);
        logger.info("[seed check] checkSubchainSeed success,use seed:"+chainConfig.configSub.httpEndpoint);
        checkSubSeedTime = 0;
        return;
    } catch (e) {
        logger.error("[seed check] checkSubchainSeed error("+chainConfig.configSub.httpEndpoint+") :", utils.logNetworkError(e));
        checkSubSeedTime++;
        if (checkSubSeedTime <= maxCheckSeedTime) {
            logger.error("[seed check] check subchain seed time("+checkSubSeedTime+") <= max("+maxCheckSeedTime+"),wait next...");
            return;
        }

        checkSubSeedTime = 0;

        let seedHttpList = chainConfig.configSub.seedHttpList;
        if (seedHttpList.length > 0) {
            logger.info("[seed check] start to use other seed http to request checkSubchainSeed");
            for (let i = 0; i < seedHttpList.length; i++) {
                chainConfig.configSub.httpEndpoint = seedHttpList[i];
                path = chainConfig.configSub.httpEndpoint + "/v1/chain_info/get_chain_info"
                try {
                    let res = await axios.post(path);
                    chainConfig.u3Sub = createU3({...chainConfig.configSub, sign: true, broadcast: true});
                    logger.info("[seed check] checkSubchainSeed("+path+") success,use new seed:"+chainConfig.configSub.httpEndpoint);
                    return null;
                } catch (e) {
                    logger.error("[seed check] checkSubchainSeed("+path+") error:", utils.logNetworkError(e));
                }
            }
        }

    }

    return null;
}

checkMainchainSeed = async (chainConfig) => {

    let path = chainConfig.config.httpEndpoint + "/v1/chain_info/get_chain_info"
    try {
        let res = await axios.post(path);
        logger.info("[seed check] checkMainchainSeed success,use seed:"+chainConfig.config.httpEndpoint);
        checkMainSeedTime = 0;
        return;
    } catch (e) {
        logger.info("[seed check] start to use other seed http to request checkMainchainSeed");

        checkMainSeedTime++;
        if (checkMainSeedTime <= maxCheckSeedTime) {
            logger.error("[seed check] check mainchain seed time("+checkMainSeedTime+") <= max("+maxCheckSeedTime+"),wait next...");
            return;
        }

        checkMainSeedTime = 0;
        let seedHttpList = chainConfig.config.seedHttpList;
        if (seedHttpList.length > 0) {
            logger.info("[seed check] start to use other seed http to request checkMainchainSeed");
            for (let i = 0; i < seedHttpList.length; i++) {
                chainConfig.config.httpEndpoint = seedHttpList[i];
                path = chainConfig.config.httpEndpoint + "/v1/chain_info/get_chain_info"
                try {
                    let res = await axios.post(path);
                    chainConfig.u3Sub = createU3({...chainConfig.config, sign: true, broadcast: true});
                    logger.info("[seed check] checkMainchainSeed("+path+") success,use new seed:"+chainConfig.config.httpEndpoint);
                    return null;
                } catch (e) {
                    logger.error("[seed check] checkMainchainSeed("+path+") error:", utils.logNetworkError(e));
                }
            }
        }

    }

    return null;
}

/**
 *
 * @param data
 * @param sign
 * @returns {boolean}
 */
verifySign = (data,sign) => {
    try {
        let apiTime = data.time;
        let nowTime = new Date().getTime();
        if (apiTime - nowTime >= constant.API_MAX_INTEVAL_TIME || nowTime - apiTime >= constant.API_MAX_INTEVAL_TIME) {
            logger.error("api time is not valid(api time("+apiTime+") : local time:("+nowTime+")");
            return false;
        }
        logger.debug("api time is  valid(api time("+apiTime+") : local time:("+nowTime+")");
        let sign = data.sign;
        logger.debug("check sign:",sign);
        data.sign = "sign";
        let res = JSON.stringify(data);
        logger.debug("check sign new data:",res);
        let rawStr = res+constant.PRIVATE_KEY;
        logger.debug("check sign raw string:",rawStr);
        let checkSign = hashUtil.calcMd5(rawStr);
        logger.debug("check sign md5:",checkSign);

        if (sign == checkSign) {
            logger.debug("sign("+sign+") == checksign("+checkSign+")")
            return true;
        }

        logger.error("sign("+sign+") != checksign("+checkSign+")")
    } catch (e) {
        logger.error("verify sign error:",e);
    }

    return false;

}

/**
 * get seed info
 * @param url
 * @param param
 * @returns {Promise<*>}
 */
getSeedInfo = async (url, param) => {
    try {
        logger.info("getSeedInfo param:", qs.stringify(param));
        const rs = await axios.post(url + "/filedist/getSeedInfo", qs.stringify(param));
        logger.info("monitor getSeedInfo result:", rs.data);
        let data =  rs.data;
        if (data.code ==0) {
            return rs.data;
        } else {
            logger.error("getSeedInfo error:",rs.data);
        }
    } catch (e) {
        logger.error("getSeedInfo error,", utils.logNetworkError(e));
    }

    return null;
}

/**
 *
 * @param config
 * @param blockNum
 * @param trxId
 * @returns {Promise<*>}
 */
getMerkleProof = async (config,blockNum,trxId) => {
    try {
        const rs = await multiRequest(config.httpEndpoint, "/v1/chain/get_merkle_proof", {block_number:blockNum,trx_id:trxId}, config.seedHttpList);
        return rs.data;
    } catch (e) {
        logger.error("get getMerkleProof error:",utils.logNetworkError(e));
    }

    return null;
}

module.exports = {
    getChainId,
    getChainInfo,
    getProducerLists,
    contractInteract,
    getUserBulletin,
    getAccount,
    getTableInfo,
    getTableAllData,
    getChainSeedIP,
    getSubchanEndPoint,
    getSubchainConfig,
    getSubchanMonitorService,
    getSubchainResource,
    getChainBlockDuration,
    monitorCheckIn,
    checkDeployFile,
    finsihDeployFile,
    getChainPeerKey,
    getChainName,
    getSubchainList,
    getSyncBlockChainList,
    getTargetChainBlockNum,
    addSwitchLog,
    getChainHttpList,
    getSubchainBlockNum,
    getSubchainCommittee,
    checkSubchainSeed,
    verifySign,
    getSeedInfo,
    checkMainchainSeed,
    addRestartLog,
    getMasterBlockNum,
    getHeadBlockNum,
    getMerkleProof,
    getHeadBlockProposer,
    getBlockHeaderInfo,
    getMinBlockHeaderInfo,
    confirmBlockCheckIn,
    getCommitteeBulletin,
    getChainIdByAllSeed,
    ramUsageCheckIn,
    getServerVersion,
    checkSeedReady,
    getBlockInfoData,
    getMasterHeadBlockNum,
    getTableAllScopeData,
    uploadugas,
}
