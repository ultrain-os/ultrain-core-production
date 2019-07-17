var utils = require("../../common/util/utils")
/**
 * 命令相关常量
 *
 * @type {{KILL_NODULTRAIN: string, KILL_WORLDSTATE: string}}
 */
var cmdConstants = {
    KILL_NODULTRAIN: "killall 'nodultrain'",//关闭nodultrain
    KILL_WORLDSTATE: "killall 'wssultrain'", //关闭世界状态进程
    KILL_PM2: "killall 'pm2'",//关闭pm2
    PM2_LIST: "/usr/local/bin/pm2 list",//pm2 list
    CLEAR_BLOCK_DATA: "rm -rf ~/.local/share/ultrainio/nodultrain/data/blocks/",//清空本地块数据
    CLEAR_SHARED_MEMORY_DATA: "rm -rf ~/.local/share/ultrainio/nodultrain/data/state/",//清空memory.bin数据
    CLEAR_WORLD_STATE_FILE: "rm -rf ~/.local/share/ultrainio/wssultrain/data/worldstate/", //清空ws下文件
    CLEAR_STATE_FILE: "rm ~/.local/share/ultrainio/nodultrain/data/state/* -rf", //清空state目录文件
    START_WORLDSTATE: "~/workspace/ultrain-core/build/programs/wssultrain/wssultrain > /log/ws.log 2>&1 &",//启动世界状态程序
    START_NODULTRAIN: "sh ~/workspace/ultrain-core/scripts/_runultrain.sh ~/workspace",//启动nod程序
    START_NODULTRAIN_FILE: utils.formatHomePath("~/ultrainmng/tool/_runultrain.sh"),
    START_WORLDSTATE_FILE: utils.formatHomePath("~/ultrainmng/tool/_runworldstate.sh"),
    START_NODULTRAIN_ARG: [utils.formatHomePath("~/workspace")],
    ENABLE_RESTART: "enableRestart",
    DISABLE_RESTART: "disableRestart",
    ENABLE_SYNC_USER_RES: "enableUserRes",
    DISABLE_SYNC_USER_RES: "disableUserRes",
    ADD_NOD_CONFIG: "ADD_NOD_CONFIG",
    ENABLE_SYNC_UGAS: "enableUgas",
    DISABLE_SYNC_UGAS: "disableUgas",
    ENABLE_SYNC_USER_RES_BY_BLOCK: "enableUserResByBlock",
    DISABLE_SYNC_USER_RES_BY_BLOCK: "disableUserResByBlock",
    SET_SYNC_BLOCK_MAX_COUNT: "syncBlockMaxCount",
    RESTART_NOD: "restartNod",
    UPDATE_MONITOR: "updateMonitor",
}

//编码常量
var encodingConstants = {
    UTF8: "UTF-8",
    GBK: "GBK",
}

//链常量
var chainNameConstants = {
    MAIN_CHAIN_NAME: "ultrainio",
    INVAILD_CHAIN_NAME: "zzzzzzzzzzzzj", //非法链
    MAIN_CHAIN_NAME_TRANSFER: "master", //只有转账的时候用它标志主链
}

//时间常量（单位ms）
var timeConstats = {
    SECOND: 1000,
    SECONDS10: 1000 * 10,
    MINUTE: 1000 * 60,
    HOUR: 1000 * 60 * 60,
    DAY: 1000 * 60 * 60 * 24
}

//合约常量
var contractConstants = {
    ULTRAINIO: "ultrainio",
    UTRIO_BANK:"utrio.bank",
    UTRIO_RAND:"utrio.rand",
}

//action常量
var actionConstants = {
    VOTE_ACCOUNT: "voteaccount", //用户投票
    VOTE_RESOURCE_LEASE: "voteresourcelease", //资源投票

}

//表常量
var tableConstants = {
    RESOURCE_LEASE: "reslease",//用户资源表，
    PENDING_MINER: "pendingminer",//委员会投票表
    PENDING_ACCOUNT: "pendingacc",//账户投票表
    PENDING_RES: "pendingres", //资源投票表
    WORLDSTATE_HASH: "wshash", //世界状态hash表
    GLOBAL: "global",
    BULLETIN_BANK: "bulletinbank",
    CHAINS: "chains",
    BLOCK_HEADER:"blockheaders",
}

//scode常量
var scopeConstants = {
    SCOPE_MAIN_CHAIN: "ultrainio",//主链scope
}

//chainid常量
var chainIdConstants = {
    NONE_CHAIN: "0000000000000000000000000000000000000000000000000000000000000000",//未同步子链创世块
}

//路径常量
var pathConstants = {
    WSS_DATA: utils.formatHomePath("~/.local/share/ultrainio/wssultrain/data/worldstate/download/"),
    WSS_LOCAL_DATA: utils.formatHomePath("~/.local/share/ultrainio/wssultrain/data/worldstate/"),
    MNG_CONFIG: utils.formatHomePath("~/.local/share/ultrainio/ultrainmng/config/"),
    FILE_DOWNLOAD_PATH:utils.formatHomePath("~/.local/share/ultrainio/ultrainmng/download/"),
}

//ini配置常量
var iniConstants = {
    UDP_SEED: "udp-seed",
    MONITOR_SERVER_ENDPOINT: "monitor-server-endpoint",
    P2P_PEER_ADDRESS: "p2p-peer-address",
    RPOS_P2P_PEER_ADDRESS: "rpos-p2p-peer-address",
    SUBCHAIN_HTTP_ENDPOINT: "subchainHttpEndpoint",
    GENESIS_TIME: "genesis-time",
    GENESIS_PK: "genesis-pk",
    MNG_PATH: "mngpath",
    NOD_PATH: "nodpath",
    WSS_PATH: "wsspath",
    MONITOR: "monitor",
    PEER_KEY: "peer-key",
    CHAIN_NAME: "chain-name",
    MONGODB_BLOCK_START: "mongodb-block-start",
}

/**
 * 缓存key常量
 * @type {{NOD_FILE_KEY: string, MNG_FILE_KEY: string}}
 */
var cacheKeyConstants = {
    NOD_FILE_KEY: "nod_file_key",
    MNG_FILE_KEY: "mng_file_key",
    WS_FILE_KEY: "ws_file_key",
    RAND_FILE_KEY: "rand_file_key",
    SERVER_VERSOIN_KEY: "server_version_key",
    SEED_CONFIG_KEY: "seed_config_key"
}

/**
 * 文件名constants
 * @type {{NOD_EXE_FILE: string}}
 */
var filenameConstants = {
    NOD_EXE_FILE: "nodultrain",
    MNG_FILE: "sideChainService.js",
    WS_EXE_FILE:"wssultrain",
    RAND_FILE:"votingRandService.js"
}

/**
 * 加密算法
 * @type {{SHA256: string, SHA1: string, MD5: string}}
 */
var algorithmConstants = {
    MD5 : "md5",
    SHA1 : "sha1",
    SHA256 : "sha256"
}

/**
 * status 常量
 * @type {{STARTING: number, SUCCESS: number, STOP: number, EXCEPTION: number, UNSTART: number}}
 */
var statusConstants = {
    SUCCESS: 4,
    EXCEPTION: 3,
    STOP: 2,
    STARTING: 1,
    UNSTART: 0
}

var PRIVATE_KEY = "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQC6TQALgms1BnT02fowOtSGGCQ1ed1GVWvzODASnDMlyRsbiwnsMROf7YZ7umA4ma5n9erPyw27ile7JDjsQo1GbUZn2tAbjg1G7VPgkxp9QZp8uXquTI9bDEYXIeYQS9f71mh8DkR3VOUru8+j5uCOqmF+jiDMOt8qf5Yyhw5fbQIDAQAB";
//API验证时间前后不超过1小时（单位ms）
var API_MAX_INTEVAL_TIME = 1000 * 60 * 60;

//未知用户
var UNKNOWN_USER = "unknown001";

/**
 * apiTimeConstants
 * @type {{SEED_CHECK_API_TIME: number, DEFAULT_API_TIME: number}}
 */
var apiTimeConstants = {
    DEFAULT_SEED_API_TIME : 6000,
    LOCAL_API_TIME : 3000,
}


module.exports = {
    cmdConstants,
    encodingConstants,
    chainNameConstants,
    timeConstats,
    contractConstants,
    tableConstants,
    actionConstants,
    scopeConstants,
    chainIdConstants,
    pathConstants,
    iniConstants,
    cacheKeyConstants,
    filenameConstants,
    algorithmConstants,
    statusConstants,
    PRIVATE_KEY,
    API_MAX_INTEVAL_TIME,
    UNKNOWN_USER,
    apiTimeConstants,
}
