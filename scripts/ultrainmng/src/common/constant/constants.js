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
    CLEAR_BLOCK_DATA: "rm -rf /root/.local/share/ultrainio/nodultrain/data/blocks/",//清空本地块数据
    CLEAR_SHARED_MEMORY_DATA: "rm -rf /root/.local/share/ultrainio/nodultrain/data/state/",//清空memory.bin数据
    CLEAR_WORLD_STATE_FILE: "rm -rf /root/.local/share/ultrainio/wssultrain/data/worldstate/", //清空ws下文件
    START_WORLDSTATE: "/root/workspace/ultrain-core/build/programs/wssultrain/wssultrain > /log/ws.log 2>&1 &",//启动世界状态程序
    START_NODULTRAIN: "sh /root/workspace/ultrain-core/scripts/_runultrain.sh /root/workspace",//启动nod程序
    START_NODULTRAIN_FILE: "/root/ultrainmng/tool/_runultrain.sh",
    START_WORLDSTATE_FILE: "/root/ultrainmng/tool/_runworldstate.sh",
    START_NODULTRAIN_ARG: ["/root/workspace"],
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
    WSS_DATA: "/root/.local/share/ultrainio/wssultrain/data/worldstate/download/",
    WSS_LOCAL_DATA: "/root/.local/share/ultrainio/wssultrain/data/worldstate/",
    MNG_CONFIG: "/root/.local/share/ultrainio/ultrainmng/config/",
    FILE_DOWNLOAD_PATH:"/root/.local/share/ultrainio/ultrainmng/download/",
}

//ini配置常量
var iniConstants = {
    UDP_SEED: "udp-seed",
    MONITOR_SERVER_ENDPOINT: "monitor-server-endpoint",
    P2P_PEER_ADDRESS: "p2p-peer-address",
    RPOS_P2P_PEER_ADDRESS: "rpos-p2p-peer-address",
    SUBCHAIN_HTTP_ENDPOINT: "subchainHttpEndpoint",
    GENESIS_TIME: "genesis-time",
    MNG_PATH: "mngpath",
    NOD_PATH: "nodpath",
    WSS_PATH: "wsspath",
    MONITOR: "monitor",
    PEER_KEY: "peer-key",
    CHAIN_NAME: "chain-name",
}

/**
 * 缓存key常量
 * @type {{NOD_FILE_KEY: string, MNG_FILE_KEY: string}}
 */
var cacheKeyConstants = {
    NOD_FILE_KEY: "nod_file_key",
    MNG_FILE_KEY: "mng_file_key",
    WS_FILE_KEY: "ws_file_key"
}

/**
 * 文件名constants
 * @type {{NOD_EXE_FILE: string}}
 */
var filenameConstants = {
    NOD_EXE_FILE: "nodultrain",
    MNG_FILE: "sideChainService.js",
    WS_EXE_FILE:"wssultrain"
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
}
