#!/usr/bin/python
import pymongo
import datetime
import os
import time
import sys
#global var
nodconfig="/root/.local/share/ultrainio/nodultrain/config/config.ini"

#write mongo start num
def start_num(blk_num):
    need = True
    start_num="mongodb-block-start = %s"%(blk_num)
    with open(nodconfig, "r") as f:
        lines = f.readlines()
    with open(nodconfig, "w") as f_w:
        for line in lines:
            if "mongodb-block-start" in line:
                need=False
                line = line.replace(line, "%s"%(start_num))
            f_w.write(line)
    if need == True :
        open(config,'a').write("%s"%(start_num))

#parse parameters
n=len(sys.argv)
if not n == 3 :
   print "please provide mongod dir and mongo data dir"
   os._exit(0)


#start mongod
bin_dir = sys.argv[1]
data_dir= sys.argv[2]
mongd_cmd="%s/mongod --auth --dbpath %s/data --logpath %s/log/mongodb.log --fork --bind_ip 0.0.0.0"%(bin_dir,data_dir,data_dir)
if os.system('pidof mongod'):
    os.system(mongd_cmd)
while os.system('pidof mongod'):
    time.sleep( 2 )

#prepare db
info=[x for x in open(nodconfig).readlines() if x.find('mongodb-uri')>-1][0].split("/")[2]
user=info.split("@")[0].split(":")[0]
password=info.split("@")[0].split(":")[1]
host=info.split("@")[1].split(":")[0]
port=info.split("@")[1].split(":")[1]
uri = "mongodb://%s:%s@%s:%s/ultrain"%(user,password,host,port)

myclient = pymongo.MongoClient(uri)
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
curs = block_states.find({'irreversible':{'$exists': False}}).sort('block_num',1).limit(1)
if curs.count() :
    blk_num =curs[0]['block_num']-2
else :
    print "block was cutted"
    blk_num=block_states.find({'irreversible':{'$exists': True}}).sort('block_num',-1).limit(1)[0]['block_num']
if not blk_num :
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

print "block_num:%s"%(blk_num+1)
#not need, ultrmng will do it
#start_num(blk_num+1)
