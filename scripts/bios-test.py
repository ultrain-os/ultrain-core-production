#!/usr/bin/env python

import argparse
import os
import random
import subprocess
import sys
import time
import requests
import json
from account_info import *
#from account_info_master import *
#from account_info_sub1 import *
#from account_info_sub2 import *
#from account_info_sub3 import *

local = True;
args = None
logFile = None
min_committee_staked = 420000000
min_committee_number = 30
max_resources_number = 10000
unlockTimeout = 999999999
maxBodySize = 2 * 1024 * 1024
reward_tensecperiod = 10000
reward_twosecperiod = 2000
newaccount_fee = 2000
max_ram_size = 12 * 1024 *1024 *1024
worldstate_interval = 1000
resourcelease_fee = 35068
defaultclu = '%s/ultrain-core/build/programs/clultrain/clultrain --wallet-url http://127.0.0.1:6666 '
defaultkul = '%s/ultrain-core/build/programs/kultraind/kultraind'
defaultcontracts_dir = '%s/ultrain-core/build/contracts/'
initaccount = accounts
def jsonArg(a):
    return " '" + json.dumps(a) + "' "

def run(args):
    print('bios-test.py:', args)
    logFile.write(args + '\n')
    if subprocess.call(args, shell=True):
        print('bios-test.py: exiting because of error')
        sys.exit(1)

def simple_run(args):
    print('bios-test.py:', args)
    logFile.write(args + '\n')
    if subprocess.call(args, shell=True):
        print('bios-test.py: error')

def retry(args):
    while True:
        print('bios-test.py:', args)
        logFile.write(args + '\n')
        if subprocess.call(args, shell=True):
            sleep(0.3)
            print('*** Retry')
        else:
            break

def background(args):
    print('bios-test.py:', args)
    logFile.write(args + '\n')
    return subprocess.Popen(args, shell=True)

def sleep(t):
    print('sleep', t, '...')
    time.sleep(t)
    print('resume')

def importKeys():
    run(args.clultrain + 'wallet import --private-key ' + args.private_key)
    run(args.clultrain + 'wallet import --private-key ' + args.initacc_sk)
    run(args.clultrain + 'wallet import --private-key  5KG6NiRGhsEP9vTf4WVe312iVQ3uemEXsstsqkT9Wj1MkdY5uJk')
    run(args.clultrain + 'wallet import --private-key  5KBtaXHJPeq9n5SzuBoxZNZFUZvEvzrRa4HhwpTHDV9dBmDXks8')  #utrioaccount privkey
    for i in range(0, len(account_sk_list)):
       run(args.clultrain + 'wallet import --private-key ' + account_sk_list[i])
    for i in range(0, len(rand_sk_lst)):
       run(args.clultrain + 'wallet import --private-key ' + rand_sk_lst[i])

def updateAuth(account, permission, parent, controller):
    run(args.clultrain + 'push action ultrainio updateauth' + jsonArg({
        'account': account,
        'permission': permission,
        'parent': parent,
        'auth': {
            'threshold': 1, 'keys': [], 'waits': [],
            'accounts': [{
                'weight': 1,
                'permission': {'actor': controller, 'permission': 'active'}
            }]
        }
    }) + '-p ' + account + '@' + permission)

def resign(account, controller):
    updateAuth(account, 'owner', '', controller)
    updateAuth(account, 'active', 'owner', controller)
    sleep(1)
    run(args.clultrain + 'get account ' + account)

def randomTransfer():
    subaccounts = accounts[1:3] #args.num_producers
    for i in subaccounts:
        for j in subaccounts:
            if i != j:
                simple_run(args.clultrain + 'transfer -f %s %s "0.%s UGAS" ' %(i, j, random.randint(1, 999)))
#    sleep(2)

def startWallet():
    run('rm -rf ' + os.path.abspath(args.wallet_dir))
    run('mkdir -p ' + os.path.abspath(args.wallet_dir))
    background(args.kultraind + ' --max-body-size %d --unlock-timeout %d --http-server-address 127.0.0.1:6666 --wallet-dir %s' % (maxBodySize, unlockTimeout, os.path.abspath(args.wallet_dir)))
    sleep(1)
    run(args.clultrain + 'wallet create')

def stepKillAll():
    run('killall kultraind || true')
    sleep(1.5)

def stepStartWallet():
    startWallet()
    importKeys()

def createSystemAccounts():
    os.system("killall  rand.sh")
    while True:
        j = json.loads(requests.get("http://127.0.0.1:8888/v1/chain/get_block_info",data = json.dumps({"block_num_or_id":"3"})).text)
        if ("proposer" in j):
            break
        print ("waiting for block 3 is ready in the chain....")
        sleep(2)
    for a in systemAccounts:
        run(args.clultrain + 'create account ultrainio ' + a + ' ' + args.public_key)

def stepInstallSystemContracts():
    retry(args.clultrain + 'set contract utrio.token ' + args.contracts_dir + 'ultrainio.token/')
    retry(args.clultrain + 'set contract utrio.msig ' + args.contracts_dir + 'ultrainio.msig/')
    retry(args.clultrain + 'set contract utrio.rand ' + args.contracts_dir + 'ultrainio.rand/')
    retry(args.clultrain + 'set contract utrio.bank ' + args.contracts_dir + 'ultrainio.bank/')
    sleep(2)

def stepCreateTokens():
    retry(args.clultrain + 'push action utrio.token create \'["ultrainio", "1000000000.0000 UGAS"]\' -p utrio.token')
    if not (args.subchain and args.subchain != 'ultrainio') :
        retry(args.clultrain + 'push action utrio.token issue \'["ultrainio", "900000000.0000 UGAS", "memo"]\' -p ultrainio')
    retry(args.clultrain + 'push action utrio.token set_chargeparams \'{"symbol":"UGAS","precision":"4","operate_interval":"%s","operate_fee":"%s",,"is_forbid_trans":0}\'  -p  utrio.token'  % ( 60,100))
    sleep(2)

def stepSetSystemContract():
    retry(args.clultrain + 'set contract ultrainio ' + args.contracts_dir + 'ultrainio.system/')
    retry(args.clultrain + 'push action ultrainio setpriv' + jsonArg(['utrio.msig', 1]) + '-p ultrainio@active')
    sleep(2)

def getrewardaccount(producer):
    producer = producer + ".r"
    prodlen = len(producer)
    if prodlen > 12:
        producer = producer[(prodlen-12):prodlen]
    if producer[0] == '.':
        producer = "a"+producer[1:]
    return producer
def stepCreateStakedAccounts():
    retry(args.clultrain + ' create account ultrainio hello %s ' % args.initacc_pk)
    rewardlist = []
    for i in range(0, args.num_producers + 1 + args.non_producers):
        retry(args.clultrain + 'create account ultrainio %s %s ' % (accounts[i], account_pk_list[i]))
        if accounts[i] == "genesis":
            continue
        rewardacc = getrewardaccount(accounts[i])
        rewardlist.append(rewardacc)
        retry(args.clultrain + 'create account ultrainio %s %s ' % (rewardacc, account_pk_list[i]))
    print(" reward account list:")
    for a in rewardlist:
        print(a)

    for a in initialAccounts:
        retry(args.clultrain + ' create account ultrainio %s %s ' % (a, args.initacc_pk))
    if not (args.subchain and args.subchain != 'ultrainio') :
        retry(args.clultrain + ' create account ultrainio utrioaccount UTR5Fa66fjgrWaNbUUeK1ZMfSfSHjA4nofQkwbK6bf4v6DRfaNkk6')

def stepInitSimpleTest():
    retry(args.clultrain + 'push action hello hi \'{"user":"%s"}\' -p %s' % (accounts[1],accounts[1]))
    for i in range(10):
        retry(args.clultrain + 'transfer -f %s %s "0.1234 UGAS" ' %(accounts[1], accounts[2]))
        retry(args.clultrain + 'transfer -f %s %s "0.1234 UGAS" ' %(accounts[2], accounts[1]))
    for i in range(10):
        transvale = random.randint(1, 999)
        retry(args.clultrain + 'transfer  %s %s "0.%s UGAS" ' %(accounts[1], accounts[2], transvale))
        retry(args.clultrain + 'transfer  %s %s "0.%s UGAS" ' %(accounts[2], accounts[1], transvale))

def stepRegProducers():
    for i in range(1, args.num_producers+1):
        retry(args.clultrain + 'system regproducer %s %s %s %s https://%s.com "ultrainio" -u' % (accounts[i], pk_list[i], bls_pk_list[i], getrewardaccount(accounts[i]), accounts[i]))
    #retry(args.clultrain + 'set contract hello  ' + args.contracts_dir + 'hello/')
    sleep(2)
    delegateaccount = "utrio.stake"
    if not (args.subchain and args.subchain != 'ultrainio') :
        delegateaccount = "ultrainio"
        retry(args.clultrain + ' push action ultrainio setfreeacc \'{"account":"utrioaccount", "number":10000}\' -p ultrainio')
    for i in range(1, args.num_producers+1):
        retry(args.clultrain + 'system delegatecons %s %s  "%.4f UGAS" ' % (delegateaccount, accounts[i], min_committee_staked/10000))

    if local == True :
        if args.subchain and args.subchain != "ultrainio" :
            masterproducerinfo = ""
            for i in range(1, args.num_master_prods + 1):
                masterproducerinfo += '{"owner":"%s","producer_key":"%s","bls_key":"%s"},' % (("master"+initaccount[i]), pk_list[i], bls_pk_list[i] )
            masterproducerinfo = masterproducerinfo[:-1]
            retry(args.clultrain + ' push action ultrainio setmasterchaininfo \'{"chaininfo":{"owner": "ultrainio",\
            "master_prods":[%s],"block_height":%s,"block_id":"%s","master_chain_ext":[],"committee_mroot":"%s"}}\' -p ultrainio ' % \
            ( masterproducerinfo, args.num_master_block, args.master_block_id,args.committee_mroot) )
    sleep(15)
    #table_extension key, detail see ultrainio.system.hpp
    #enum global_state_exten_type_key {
         # global_state_key_start = 0,
         # update_auth = 1,
         # confirm_point_interval = 2,
         # sidechain_charge_ratio = 3,
         # is_claim_reward = 4,
    #};
    retry(args.clultrain + ' push action ultrainio setsysparams \'{"params":{"chain_type": "0", "max_ram_size":"%s",\
        "min_activated_stake":%s,"min_committee_member_number":%s,\
        "block_reward_vec":[{"consensus_period":10,"reward":"%s"},{"consensus_period":2,"reward":"%s"}],\
        "max_resources_number":%s, "newaccount_fee":%s, "chain_name":"%s", "worldstate_interval":%s,"resource_fee":%s,"table_extension":[[1,"10000"], [2, "12"], [3, "1"], [4, "true"]]}}\' -p ultrainio ' % \
        (max_ram_size, min_committee_staked, min_committee_number, reward_tensecperiod, reward_twosecperiod, max_resources_number, \
        newaccount_fee, args.subchain, worldstate_interval, resourcelease_fee) )
    sleep(5)
    retry(args.clultrain + ' system listproducers')

def stepCreateinitAccounts():
    # for i in range(1, args.num_producers+1):
    #     retry(args.clultrain + 'transfer ultrainio %s "%.4f UGAS"' % (accounts[i], 5000))

    # for a in initialAccounts:
    #     retry(args.clultrain + 'transfer  ultrainio  %s  "%s UGAS" '  % (a,"100000000.0000"))
    # retry(args.clultrain + 'system resourcelease ultrainio  hello  10 100  "ultrainio"')
    pass
def stepResign():
    #resign('ultrainio', 'utrio.null')
    resign('utrio.stake', 'utrio.null')
    #resign('utrio.bank', 'utrio.null')

def resourceTransaction(fromacc,recacc,value):
    retry(args.clultrain + 'system delegatebw  %s %s "%s UGAS"  "%s UGAS"'  % (fromacc,recacc,5000/value,5000/value))
    sleep(20)
    j = json.loads(requests.get("http://127.0.0.1:8888/v1/chain/get_account_info",data = json.dumps({"account_name":recacc})).text)
    assert j["cpu_weight"] ==5000/value*10000,'account:'+recacc+' cpu_weight:'+str(j["cpu_weight"])+'!='+str(5000/value*10000)
    assert j["net_weight"] ==5000/value*10000,'account:'+recacc+' net_weight:'+str(j["net_weight"])+'!='+str(5000/value*10000)
    retry(args.clultrain + 'system undelegatebw  %s  %s  "%s UGAS"  "%s UGAS" '  % (fromacc,recacc,50/value,60/value))
    sleep(20)
    j = json.loads(requests.get("http://127.0.0.1:8888/v1/chain/get_account_info",data = json.dumps({"account_name":recacc})).text)
    assert j["net_weight"] == 4950/value*10000,'undelegate account:'+recacc+' net_weight:'+str(j["net_weight"])+'!='+str(4950/value*10000)
    assert j["cpu_weight"] == 4940/value*10000,'undelegate account:'+recacc+' cpu_weight:'+str(j["cpu_weight"])+'!='+str(4940/value*10000)
    retry(args.clultrain + 'system buyram  %s  %s  "%s UGAS"  '  % (fromacc,recacc,50000/value))
    sleep(2)
    retry(args.clultrain + 'transfer  %s  %s  "%s UGAS" '  % (fromacc,recacc,20000/value))
    sleep(25)
    j = json.loads(requests.get("http://127.0.0.1:8888/v1/chain/get_account_info",data = json.dumps({"account_name":recacc})).text)
    ramjson = json.loads(requests.get("http://127.0.0.1:8888/v1/chain/get_table_records",data = json.dumps({"code":"ultrainio","scope":"ultrainio","table":"rammarket","json":"true","table_key":"","lower_bound":"","upper_bound":"","limit":10,"key_type":"","index_position":""})).text)   #Calculating ram ratio
    ramvalue = ramjson["rows"][0]["base"]["balance"].replace(" RAM","")
    sysvalue = ramjson["rows"][0]["quote"]["balance"].replace(" UGAS","")
    shouldbuyram = float(ramvalue)*50000/value/float(sysvalue)
    sellram_before = j["ram_quota"]
    assert j["ram_quota"] >= shouldbuyram,'buyram account:'+recacc+' RAM:'+str(j["ram_quota"])+'<'+str(shouldbuyram)
    core_liquid_balance = float(j["core_liquid_balance"].replace(" UGAS",""))
    assert core_liquid_balance == 20000/value,'transfer account:'+recacc+' balance:'+str(core_liquid_balance)+'!='+str(20000/value)
    #print(requests.get("http://127.0.0.1:8888/v1/chain/get_account_info",data = json.dumps({"account_name":recacc})).text)
    #retry(args.clultrain + 'get account  %s ' % (recacc))
    retry(args.clultrain + 'system sellram  %s  "1024 bytes" '  % (recacc))
    sleep(20)
    j = json.loads(requests.get("http://127.0.0.1:8888/v1/chain/get_account_info",data = json.dumps({"account_name":recacc})).text)
    assert (sellram_before-j["ram_quota"]) == 1024,'sellram account:'+recacc+' sell_ram:'+str(sellram_before-j["ram_quota"])+'!='+str(1024)
    retry(args.clultrain + 'set contract %s  %sultrainio.msig/' % (recacc,args.contracts_dir))
    sleep(20)
    j = json.loads(requests.get("http://127.0.0.1:8888/v1/chain/get_raw_code_and_abi",data = json.dumps({"account_name":recacc})).text)
    assert j["wasm"] != "",'set contract account:'+recacc+' failed ,wasm is null'

def stepResourceTransaction():
    resourceAccount = [
        "resacc11",
        "resacc22",
        "resacc33aaaa",
        "resacc44aaaa"
    ]
    retry(args.clultrain + 'system newaccount --transfer ultrainio %s %s --stake-net "0 UGAS" --stake-cpu "0 UGAS" --buy-ram "1.000 UGAS" ' % (resourceAccount[0], args.public_key))
    retry(args.clultrain + 'create account ultrainio %s %s ' % (resourceAccount[1], args.public_key))
    sleep(20)
    j = json.loads(requests.get("http://127.0.0.1:8888/v1/chain/get_account_info",data = json.dumps({"account_name":resourceAccount[0]})).text)
    assert j["ram_usage"] ==272,'system newaccount:'+resourceAccount[2]+' ramusage:'+str(j["ram_usage"])+'!=272'

    r = requests.get("http://127.0.0.1:8888/v1/chain/get_account_info",data = json.dumps({"account_name":resourceAccount[1]}))
    j = json.loads(r.text)
    assert j["ram_usage"] ==0,'create account:'+resourceAccount[2]+' ramusage:'+str(j["ram_usage"])+'!=0'
    resourceTransaction("ultrainio",resourceAccount[0],1)
    resourceTransaction("ultrainio",resourceAccount[1],1)
    retry(args.clultrain + 'system newaccount --transfer %s %s %s --stake-net "0 UGAS" --stake-cpu "0 UGAS" --buy-ram "1.000 UGAS" ' % (resourceAccount[0],resourceAccount[2], args.public_key))
    retry(args.clultrain + 'create account %s %s %s ' % (resourceAccount[1], resourceAccount[3], args.public_key))
    sleep(20)
    j = json.loads(requests.get("http://127.0.0.1:8888/v1/chain/get_account_info",data = json.dumps({"account_name":resourceAccount[2]})).text)
    assert j["ram_usage"] ==272,'system newaccount:'+resourceAccount[2]+' ramusage:'+str(j["ram_usage"])+'!=272'
    j = json.loads(requests.get("http://127.0.0.1:8888/v1/chain/get_account_info",data = json.dumps({"account_name":resourceAccount[3]})).text)
    assert j["ram_usage"] ==0,'create account:'+resourceAccount[3]+' ramusage:'+str(j["ram_usage"])+'!=0'
    resourceTransaction(resourceAccount[0],resourceAccount[2],5)
    resourceTransaction(resourceAccount[1], resourceAccount[3],5)

def stepTransfer():
    while True:
        randomTransfer()

def stepunregproducersTest():
    cur_accounts= [

    ]
    for a in cur_accounts:
        retry(args.clultrain + 'system unregprod %s  ' % a)
def stepregproducersTest():
    cur_accounts= [

    ]
    miner_pk_list = [

    ]
    acc_pk_list = [

    ]
    pklen = len(miner_pk_list)
    for i in range(0, pklen):
        j = json.loads(requests.get("http://127.0.0.1:8888/v1/chain/get_account_info",data = json.dumps({"account_name":cur_accounts[i]})).text)
        if ("account_name" in j):
            continue
        retry(args.clultrain + 'create account ultrainio %s %s ' % (cur_accounts[i], acc_pk_list[i]))
    sleep(10)
    for a in cur_accounts:
        retry(args.clultrain + 'transfer ultrainio %s  "100.0000 UGAS"' % a)
    sleep(10)
    for i in range(0, pklen):
        retry(args.clultrain + 'system regproducer %s %s https://%s.com  "ultrainio"' % (cur_accounts[i], miner_pk_list[i], cur_accounts[i]))
    sleep(10)
    for i in range(0, pklen):
        retry(args.clultrain + 'system delegatecons utrio.stake %s  "1000000.0000 UGAS" ' % (cur_accounts[i]))

def stepexecrand():
    transrandaccount = "utrio.rand"
    if not (args.subchain and args.subchain != 'ultrainio') :
        transrandaccount = "ultrainio"
    retry(args.clultrain + 'transfer %s utrio.rand "6000 UGAS" ' % transrandaccount)
    retry(args.clultrain + 'set account permission utrio.rand active \'{"threshold":1,"keys": [{"key": "%s","weight": 1}],"accounts": [{"permission":{"actor":"utrio.rand","permission":"utrio.code"},"weight":1}]}\' owner -p utrio.rand' % (args.public_key))

    for i in range(0, len(rand_acc_lst)):
        retry(args.clultrain + ' create account ultrainio %s %s ' % ( rand_acc_lst[i], rand_pk_lst[i]))
    for a in rand_acc_lst:
        retry(args.clultrain + 'transfer  %s  %s  "%s UGAS" '  % (transrandaccount, a, "1000.0000"))
    for i in range(len(rand_acc_lst)):
        value = 1000
        retry(args.clultrain + 'transfer %s utrio.rand \'%.4f UGAS\' \'in0x1WaiterRegister\' -p %s' % ( rand_acc_lst[i],value, rand_acc_lst[i]))

    randpath = "/root/workspace"
    if args.programpath:
        randpath = args.programpath
    listprods = args.clultrain + 'system listproducers'
    # os.system("cd %s/ultrain-core/scripts/rand;  ./rand.sh c  sleep 2;  ./rand.sh r  sleep 2;\
    #   nohup ./rand.sh e >/dev/null 2>&1 &  sleep 2;echo  '\n Genesis end \n';echo %s;%s" % ( randpath, listprods, listprods))
    os.system("cd %s/ultrain-core/scripts/rand; \
      nohup ./rand.sh e >/dev/null 2>&1 &  sleep 2;echo  '\n Genesis end \n';echo %s;%s" % ( randpath, listprods, listprods))
# Command Line Arguments

parser = argparse.ArgumentParser()

commands = [
    ('k', 'kill',           stepKillAll,                True,    "Kill all nodultrain and kultraind processes"),
    ('w', 'wallet',         stepStartWallet,            True,    "Start kultraind, create wallet, fill with keys"),
    ('s', 'sys',            createSystemAccounts,       True,    "Create system accounts (utrio.*)"),
    ('c', 'contracts',      stepInstallSystemContracts, True,    "Install system contracts (token, msig)"),
    ('t', 'tokens',         stepCreateTokens,           True,    "Create tokens"),
    ('S', 'sys-contract',   stepSetSystemContract,      True,    "Set system contract"),
    ('T', 'stake',          stepCreateStakedAccounts,   True,    "Create staked accounts"),
    ('I', 'initsimpletest', stepInitSimpleTest,         False,    "Simple transfer contract call test"),
    ('i', 'create-initacc', stepCreateinitAccounts,     False,    "create initial accounts"),
    ('P', 'reg-prod',       stepRegProducers,           True,    "Register producers"),
    ('q', 'resign',         stepResign,                 False,    "Resign utrio"),
    ('X', 'xfer',           stepTransfer,               False,   "Random transfer tokens (infinite loop)"),
    ('R', 'resourcetrans',  stepResourceTransaction,    False,    "resource transaction"),
    ('u', 'unregproducers',  stepunregproducersTest,    False,    "stepunregproducersTest"),
    ('r', 'regproducers',  stepregproducersTest,    False,    "stepregproducersTest"),
    ('e', 'execrand',          stepexecrand,            False,    "stepexecrand"),
]

parser.add_argument('--public-key', metavar='', help="ULTRAIN Public Key", default='UTR5t23dcRcnpXTTT7xFgbBkrJoEHvKuxz8FEjzbZrhkpkj2vmh8M', dest="public_key")
parser.add_argument('--private-Key', metavar='', help="ULTRAIN Private Key", default='5HvhChtH919sEgh5YjspCa1wgE7dKP61f7wVmTPsedw6enz6g7H', dest="private_key")
parser.add_argument('--initacc-pk', metavar='', help="ULTRAIN Public Key", default='UTR6XRzZpgATJaTtyeSKqGhZ6rH9yYn69f5fkLpjVx6y2mEv5iQTn', dest="initacc_pk")
parser.add_argument('--initacc-sk', metavar='', help="ULTRAIN Private Key", default='5KZ7mnSHiKN8VaJF7aYf3ymCRKyfr4NiTiqKC5KLxkyM56KdQEP', dest="initacc_sk")
parser.add_argument('--clultrain', metavar='', help="Clultrain command", default=defaultclu % '/root/workspace')
parser.add_argument('--kultraind', metavar='', help="Path to kultraind binary", default=defaultkul % '/root/workspace')
parser.add_argument('--contracts-dir', metavar='', help="Path to contracts directory", default=defaultcontracts_dir % '/root/workspace')
parser.add_argument('--genesis', metavar='', help="Path to genesis.json", default="./genesis.json")
parser.add_argument('--wallet-dir', metavar='', help="Path to wallet directory", default='./wallet/')
parser.add_argument('--log-path', metavar='', help="Path to log file", default='./output.log')
parser.add_argument('--symbol', metavar='', help="The utrio.system symbol", default='UGAS')
parser.add_argument('--num-producers', metavar='', help="Number of producers to register", type=int, default=5, dest="num_producers")
parser.add_argument('--non-producers', metavar='', help="Number of non-producers to create", type=int, default=1, dest="non_producers")
parser.add_argument('--num-master-prods', metavar='', help="Number of master producers to register", type=int, default=5, dest="num_master_prods")
parser.add_argument('--num-master-block', metavar='', help="Number of master chain block height ", type=int, default=0, dest="num_master_block")
parser.add_argument('--master-block-id', metavar='', help="Number of master chain block height id ", type=str, default="", dest="master_block_id")
parser.add_argument('-a', '--all', action='store_true', help="Do everything marked with (*)")
parser.add_argument('-H', '--http-port', type=int, default=8000, metavar='', help='HTTP port for clultrain')
parser.add_argument('-p','--programpath', metavar='', help="set programpath params")
parser.add_argument('-m', '--masterchain', action='store_true', help="set current master chain")
parser.add_argument('-sub', '--subchain', type=str, default="ultrainio", help="set subchain name info")
parser.add_argument('--committee_mroot', metavar='', help="committee_mroot", type=str, default="", dest="committee_mroot")
for (flag, command, function, inAll, help) in commands:
    prefix = ''
    if inAll: prefix += '*'
    if prefix: help = '(' + prefix + ') ' + help
    if flag:
        parser.add_argument('-' + flag, '--' + command, action='store_true', help=help, dest=command)
    else:
        parser.add_argument('--' + command, action='store_true', help=help, dest=command)

args = parser.parse_args()
if args.programpath:
    args.clultrain = defaultclu % (args.programpath)
    args.kultraind = defaultkul % (args.programpath)
    args.contracts_dir = defaultcontracts_dir % (args.programpath)

print(args.clultrain)
print(args.kultraind)
print(args.contracts_dir)

if local == True :
    adjustaccounts = ["genesis",]
    if args.masterchain:
        for a in accounts[1:]:
            adjustaccounts.append("master"+a)
    elif args.subchain and args.subchain != 'ultrainio' :
        for a in accounts[1:]:
            adjustaccounts.append(args.subchain+a)
    else:
        for a in accounts[1:]:
            adjustaccounts.append("user"+a)
    accounts = adjustaccounts

os.remove(args.log_path)
logFile = open(args.log_path, 'a')

logFile.write('\n\n' + '*' * 80 + '\n\n\n')

haveCommand = False
for (flag, command, function, inAll, help) in commands:
    if getattr(args, command) or inAll and args.all:
        if function:
            haveCommand = True
            function()
if not haveCommand:
    print('bios-test.py: Tell me what to do. -a does almost everything. -h shows options.')
