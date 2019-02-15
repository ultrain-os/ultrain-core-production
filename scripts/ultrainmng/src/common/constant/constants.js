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
    START_NODULTRAIN_ARG: ["/root/workspace"]

}

//编码常量
var encodingConstants = {
    UTF8: "UTF-8",
    GBK: "GBK",
}

//链常量
var chainNameConstants = {
    MAIN_CHAIN_NAME: "0",
    INVAILD_CHAIN_NAME: "18446744073709551615", //非法链
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
    ULTRAINIO: "ultrainio"
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
}

//scode常量
var scopeConstants = {
    SCOPE_MAIN_CHAIN: 0,//主链scope
}

//chainid常量
var chainIdConstants = {
    NONE_CHAIN: "0000000000000000000000000000000000000000000000000000000000000000",//未同步子链创世块
}

//路径常量
var pathConstants = {
    WSS_DATA: "/root/.local/share/ultrainio/wssultrain/data/worldstate/download/",
    MNG_CONFIG: "/root/.local/share/ultrainio/ultrainmng/config/"
}

//ini配置常量
var iniConstants = {
    UDP_SEED: "udp-seed",
    MONITOR_SERVER_ENDPOINT: "monitor-server-endpoint",
    P2P_PEER_ADDRESS: "p2p-peer-address",
    RPOS_P2P_PEER_ADDRESS: "rpos-p2p-peer-address",
    SUBCHAIN_HTTP_ENDPOINT: "subchainHttpEndpoint",
    GENESIS_TIME:"genesis-time"

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
    iniConstants
}