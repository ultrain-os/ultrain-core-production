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
