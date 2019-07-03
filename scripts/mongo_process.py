#!/usr/bin/python
import pymongo
import datetime
import os

config="/root/.local/share/ultrainio/nodultrain/config/config.ini"
#write mongo start num
def start_num(blk_num):
    need = True
    start_num="mongodb-block-start = %s"%(blk_num)
    with open(config, "r") as f:
        lines = f.readlines()
    with open(config, "w") as f_w:
        for line in lines:
            if "mongodb-block-start" in line:
                need=False
                line = line.replace(line, "%s"%(start_num))
            f_w.write(line)
    if need == True :
        open(config,'a').write("%s"%(start_num))

#prepare db
myclient = pymongo.MongoClient('mongodb://root:Uranus@127.0.0.1:27017/ultrain')
uldb = myclient['ultrain']

blocks          = uldb['blocks']
account_control = uldb['account_controls']
accounts        = uldb['accounts']
actions_traces  = uldb['action_traces']
actions         = uldb['actions']
block_states    = uldb['block_states']
pub_keys        = uldb['pub_keys']
trans_traces    = uldb['transaction_traces']
trans           = uldb['transactions']

#get latest block_num
#blk_num       = block_states.find({'irreversible':{'$exists': False}}).sort('block_num',1).limit(1)[0]['block_num']-2
curs = block_states.find({'irreversible':{'$exists': False}}).sort('block_num',1).limit(1)
if curs.count() :
    blk_num =curs[0]['block_num']-2
    print blk_num+1
else :
    print "cannot get block num"
    os._exit(1)

#rm all collections record later than the last block_num
dl_act_t  = actions_traces.delete_many({'block_num':{'$gt':blk_num}})
dl_act    = actions.delete_many({'block_num':{'$gt':blk_num}})
dl_trx_tr = trans_traces.delete_many({'block_num':{'$gt':blk_num}})
dl_trx    = trans.delete_many({'block_num':{'$gt':blk_num}})

#count the deleted records
print "delete nums:   "
print ("actions_trace: %d" % dl_act_t.deleted_count)
print ("actions:       %d" % dl_act.deleted_count)
print ("trans_traces:  %d" % dl_trx_tr.deleted_count)
print ("trans:         %d" % dl_trx.deleted_count)

#not need, ultrmng will do it
#start_num(blk_num+1)
