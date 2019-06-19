# Release Notes

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
