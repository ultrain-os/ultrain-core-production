# Release Notes

### 2019-08-02 Release(Commit: 0230026a5bc827d013c9d474f4230663e0c51dd1)

* Nodultrain
    * optimization for network connection stability and security. (nana, yemao)
    * refactor of consensus code (xiaofeng, wangxiao)
    * black/whitelist (zhiping, raymond)
    * randomly select delayed transaction when proposing (yufengshen, xiaofeng)
    * keepalive issue fix (raymond, liuwei)
    * default max client adjust (nana, yemao)
    * entering fast loop only when F+1 committee members are ahead (xiaofeng, wangxiao)
    * blk_num handling during mongo_plugin exception (liuwei, raymond)
    * propose skip verifyBa0 failed trx (yufengshen, xiaofeng)
* Contract
    * fixed rate for token transfer; add set_trans_fee rpc（yhc, yus)
* Ultrainmng
    * encryption for monitor-mng communication (zuofei)
    * reduce the freq of calling seed rpc (zuofei)

### 2019-07-11 Release(Commit: 424e935eeeb778e34352552f806e82b395f243d5)

* Nodultrain
    * restrict the size of write queue for consensus protocol (yemao, nana)
    * add evil detection for aggregating signature (xiaofeng, yemao)
    * add blocknum entry in mongoDb for quick restart (raymond, liuwei)
    * allow retry for static connection to adapts to neighboring seeds (yemao)
* Contract
    * save long-term worldstate hash in system contract (raymond, zhiping)
    * add bound for transfer fee (huichao, yufengshen)
    * rm unregprod，allow regproducer to modify reward account(huichao, yufengshen)
    * add logic for community/tech team reward (huichao, yufengshen)
    * reinforce the total amount consistency for main<->shard chain token transfer (huichao, yufengshen)
    * add fee for linkauth (huichao, yus)
    * check producerKey & blsKey in moveprod (huichao, zhongwei)
* Scripts & Tools
    * add resign of system account in bios script (huichao, yus)
    * optimize the random algorithm for seed selection (zuofei)
    * add private key generator (xiaofeng, huichao)
    * add tools for processing data for mongo quick start (liuwei, zhiping)
* Wssultrain
    * fix timeout issue when repairing worldstate file (zhiping, liuwei)
    * add rpc for returning local latest block num (zhiping, liuwei)
    * add support for saving long-term worldstate file (zhiping, liuwei)
* Ultrainmng
    * upload token distribution to monitor (zuofei)
    * add chainId param to "is ready" interface (zuofei)
    * optimize timeout handling logic for file updating (zuofei)
    * add monitor upgrade interface to mng (zuofei)
    * add restart process for seed & mongo nodes (zuofei)


### 2019-06-19 Release(Commit: 1e24b13e2dd79032693ae26dc381e0dfda55ce1e)

* Nodultrain
    * get ws data from different seeds (zhiping, liuwei)
    * check committee private key/bls key when node joins the network (nana, yemao)
    * put shard-chain genesis pk when creating and starting new shard-chain (xiaofeng, yhc)
    * de-duplicate p2p bucket (yemao, nana)
    * fix issue of bax node can't provide block syncing service (xiaofeng)
    * remove static connection that has no handshake received (yemao, nana)
    * allow packed_generated_transaction hard_fail in verifyBa0 (yufengshen)
    * add get_block_info，get_table_records to chain_info_api (zhongwei)
* Contract
    * add isexistaccount and syncaccount (yhc, yus)
    * optimize out account_sequence_object object (yhc, yus)
    * tune claimrewards (yhc, yus)
* Wssultrain & WS tool
    * add try/catch for read_section (liuwei)
* Ultrainmng
    * allow genesis-pk config transfer during shuffling
    * move some RPC calls to local
    * handling the case for non-exist config file when mng starts


### 2019-05-22 Release(Commit: 439d1fe4806c91e0fe0eab05b7615e68de46ed73)

* Nodultrain
    * unifying the versioning mechnism; add it to chian info (yufengshen, xiaofeng)
    * fix crash of replaying after starting from ws (xiaofeng)
    * in transaction receipt add packed_generated_trx(zhong wei, yufengshen，yus)
    * make variant，merkle proof work for packed_generated_trx (zhongwei)
    * on finished support (xiaofeng, yufengshen)
    * fix the order for chainbase add/delete/modify operation (raymond)
* Contract
    * switch committee scheduing to be light client based (zhongwei, yus)
    * add on finish support to system contract (xiaofeng, yus, yhc)
    * system params setting; allow user to buy resource package; check for system params overflow (yhc, yus)
* Wssultrain & WS tool
    * optimize ws tool (zhiping, liuwei)
* Ultrainmng
    * parse ws file and upload ram usage to monitor (zuofei)
    * light client based committee syncing between main/shard chains (zuofei)
    * optimze voterand; allow hot update for voterand (zuofei, liaopeng)
    * upload node version to monitor (zuofei)

### 2019-05-01 Release(Commit: e587b4e5b9929ca3dc9db4c02a4fb4e0ce195cb3)

* Nodultrain
    * fix issue of signature missing in producer node (yufengshen)
    * optimize bls check (xiaofeng)
    * optimize versioning (huichao)
    * close connection if handshake message timeouts (yemao)
    * exception detection when distributing rewards (yhc)
    * check bls/committee private key matching during handshake (nana)
* Wssultrain
    * divide ws file transmition into batches (zhiping)
    * change to C/S structure; add config for enable-listen with default open (zhiping)
* Ultrainmng
    * add (cpu, load, ram) info to monitor (zuofei)
    * optimize for exception when failing to get chianID (zuofei)
    * handling timeout exception for receiving worldstate file (zuofei)
    * update ws config during scheduling (zuofei)
* Monitor
    * collect and store more hardware info (zuofei)
    * add support for hardware overload exception (zuofei)
