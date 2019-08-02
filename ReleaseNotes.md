# Release Notes

## 2019-08-02 Release(Commit: 0230026a5bc827d013c9d474f4230663e0c51dd1)

* Nodultrain
    * 网络连接安全&稳定性相关优化（江文娜, 叶茂）
    * 共识部分代码重构 (秦晓分, 王潇）
    * 黑白名单机制 (李志平, 雷红路）
    * delayed 交易在propose的时候随机顺序执行(沈宇峰, 秦晓分)
    * keepalive 问题fix (雷红路,刘伟)
    * default max client 数目调整(江文娜, 叶茂)
    * 只有在 f+1 委员会成员领先的时候才进入 fast loop (秦晓分,王潇)
    * mongo_plugin 异常处理 blk_num (刘伟,雷红路)
    * propose skip verifyBa0 failed trx (沈宇峰,秦晓分)
* Contract
    * 转账费用调整为 fixed rate, 增加 set_trans_fee rpc（闫会超，苏羽）
* Ultrainmng
    * monitor 和管家通信的签名机制 （左非）
    * 减少管家调用 seed rpc 的频率 （左非）

## 2019-07-11 Release(Commit: 424e935eeeb778e34352552f806e82b395f243d5)

* Nodultrain
    * 限定共识协议写队列size（叶茂，江文娜）
    * 增加聚合签名作恶检测逻辑（秦晓分，叶茂）
    * mongoDb数据表增加blocknum字段以支持快速重启（雷红路，刘伟）
    * 允许静态链接进行重连尝试以适应区域内seed角色节点（叶茂）
* Contract
    * 系统合约存储长期世界状态文件HASH数值 (雷红路，李志平)
    * 增加转账手续费上下限额处理逻辑（闫会超，沈宇峰）
    * 删除unregprod调用，允许regproducer修改reward账号（闫会超，沈宇峰）
    * 增加社区，技术团队reward限额逻辑（闫会超，沈宇峰）
    * 增强主侧链资产转移过程数额一致性校验逻辑（闫会超，沈宇峰）
    * 增加linkauth操作收取手续费逻辑（闫会超，苏羽）
    * moveprod增加producerKey&blsKey有效性检查（闫会超，张忠伟）
* Scripts & Tools
    * 创世脚本增加系统账号注销功能（闫会超，苏羽）
    * 优化seed节点随机选取算法（左非）
    * 增加配置文件使用的私钥生成工具（秦晓分，闫会超）
    * 增加mongo节点快速重启数据处理工具（刘伟，李志平）
* Wssultrain
    * 修复世界状态同步文件发生超时的错误处理问题 (李志平，刘伟)
    * 增加接口返回本地最大block块高（李志平，刘伟）
    * 增加支持保留长期世界状态文件（李志平，刘伟）
* Ultrainmng
    * 上传token发行及分布信息到monitor
    * 管家调用检查seed是否ready接口时增加chainId参数
    * 管家优化文件更新超时处理逻辑
    * 管家增加monitor升级接口
    * 管家增加seed与mongo节点重启处理流程


## 2019-06-19 Release(Commit: 1e24b13e2dd79032693ae26dc381e0dfda55ce1e)

* Nodultrain
    * 为了加快速度，从不同seed节点获取数据 (志平，刘伟)
    * 节点连接进共识网络，对共识私钥，bls私钥进行验证 (江文娜，叶茂)
    * 创建新侧链时，带上新侧链genesis节点公钥 (秦晓分，闫会超)
    * 删除p2p bucket中重复的公共地址 (叶茂，江文娜)
    * 修复调度后，现有节点bax，其他节点拉块报错，导致网络停止问题 (秦晓分)
    * 移除没有收到对方握手的静态连接 (叶茂，江文娜)
    * 验证块时，允许packed_generated_transaction hard_fail (沈宇峰)
    * 增加get_block_info，get_table_records到chain_info_api (张忠伟)
* Contract
    * 增加isexistaccount和syncaccount接口 (闫会超，苏羽)
    * 创建account和auth_sequence_object时，不创建account_sequence_object对象 (闫会超，苏羽)
    * 调整claimrewards (闫会超，苏羽)
* Wssultrain & WS tool
    * read_section增加try/catch保护 (刘伟)
* Ultrainmng
    * 修改nod api接口，调度时支持genesis-pk配置的迁移
    * 修改不需要从seed请求的RPC到本地
    * 管家启动时支持配置文件不存在或异常的情况


## 2019-05-22 Release(Commit: 439d1fe4806c91e0fe0eab05b7615e68de46ed73)

* Nodultrain
    * 在chain info 中增加软件版本信息，统一代码版本提取方式（首架，晓分）
    * 修复, 世界状态后replay crash 的bug（晓分）
    * 在transaction receipt 中增加类型packed_generated_trx(zhong wei, 首架，yus)
    * 增加variant，merkle proof等对packed_generated_trx的支持
    * 在区块中增加on finished 交易的支持(晓分，首架）
    * chainbase 增删改操作顺序的优化（Raymond）
* Contract
    * 委员会调度从基于共识的模式改为基于轻客户端的模式（zhong wei， yus）
    * 在系统合约中，增加on finish的支持（晓分，yus，非凡）
    * 新增设置系统参数的方法，增加普通用户购买套餐的开关,  增加系统参数越界的检查（非凡，yus）
* Wssultrain & WS tool
    * ws tool 的优化 （志平,刘伟）
* Ultrainmng
    * 解析世界状态，获取内存使用情况，并上传monitor （左非）
    * 管家基于轻客户端模式去同步委员会变化（左非）
    * 优化voterand 脚本，增加voterand 热更新的支持（左非, 廖鹏）
    * 上传node 版本号到monitor （左非）

## 2019-05-01 Release(Commit: e587b4e5b9929ca3dc9db4c02a4fb4e0ce195cb3)

* Nodultrain
    * 修复prodcuer节点中signature信息丢失问题（首架）
    * bls检测等优化（晓分）
    * 版本号优化，方便查询和调用（非凡）
    * 检查握手消息的接收情况，如果超时，关闭连接（叶茂）
    * 检测发送奖励时异常情况（非凡）
    * handshake时添加bls密钥和commite密钥的检查匹配（nana）
* Wssultrain
    * 世界状态传输切分优化，提升到50k （志平）
    * 修改为cs结构，增加enable-listen的配置，默认为打开（志平）
* Ultrainmng
    * 上传机器信息（cpu，load，内存等）到monitor监控（左非）
    * 优化chainid获取不到的处理逻辑
    * 处理获取世界状态文件超时或其它异常的处理
    * 调度时更新ws的配置文件
* Monitor
    * 增加机器信息等数据接收和存储 （左非）
    * 增加机器负载等异常情况报警
