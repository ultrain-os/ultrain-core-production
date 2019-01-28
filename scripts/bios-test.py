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

args = None
logFile = None

unlockTimeout = 999999999
defaultclu = '%s/ultrain-core/build/programs/clultrain/clultrain --wallet-url http://127.0.0.1:6666 '
defaultkul = '%s/ultrain-core/build/programs/kultraind/kultraind'
defaultcontracts_dir = '%s/ultrain-core/build/contracts/'

def jsonArg(a):
    return " '" + json.dumps(a) + "' "

def run(args):
    print('bios-boot-tutorial.py:', args)
    logFile.write(args + '\n')
    if subprocess.call(args, shell=True):
        print('bios-boot-tutorial.py: exiting because of error')
        sys.exit(1)

def simple_run(args):
    print('bios-boot-tutorial.py:', args)
    logFile.write(args + '\n')
    if subprocess.call(args, shell=True):
        print('bios-boot-tutorial.py: error')

def retry(args):
    while True:
        print('bios-boot-tutorial.py:', args)
        logFile.write(args + '\n')
        if subprocess.call(args, shell=True):
            print('*** Retry')
        else:
            break

def background(args):
    print('bios-boot-tutorial.py:', args)
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
    for i in range(0, len(account_sk_list)):
       run(args.clultrain + 'wallet import --private-key ' + account_sk_list[i])

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
    background(args.kultraind + ' --unlock-timeout %d --http-server-address 127.0.0.1:6666 --wallet-dir %s' % (unlockTimeout, os.path.abspath(args.wallet_dir)))
    sleep(1)
    run(args.clultrain + 'wallet create')

def stepKillAll():
    run('killall kultraind || true')
    sleep(1.5)

def stepStartWallet():
    startWallet()
    importKeys()

def createSystemAccounts():
    while True:
        j = json.loads(requests.get("http://127.0.0.1:8888/v1/chain/get_block_info",data = json.dumps({"block_num_or_id":"3"})).text)
        if ("proposer" in j):
            break
        print ("waiting for block 3 is ready in the chain....")
        sleep(5)
    for a in systemAccounts:
        run(args.clultrain + 'create account -u ultrainio ' + a + ' ' + args.public_key)

def stepInstallSystemContracts():
    retry(args.clultrain + 'set contract utrio.token ' + args.contracts_dir + 'ultrainio.token/')
    retry(args.clultrain + 'set contract utrio.msig ' + args.contracts_dir + 'ultrainio.msig/')
    retry(args.clultrain + 'set contract utrio.rand ' + args.contracts_dir + 'ultrainio.rand/')
    sleep(20)

def stepCreateTokens():
    retry(args.clultrain + 'push action utrio.token create \'["ultrainio", "1000000000.0000 UGAS"]\' -p utrio.token')
    retry(args.clultrain + 'push action utrio.token issue \'["ultrainio", "900000000.0000 UGAS", "memo"]\' -p ultrainio')
    sleep(15)

def stepSetSystemContract():
    retry(args.clultrain + 'set contract ultrainio ' + args.contracts_dir + 'ultrainio.system/')
    retry(args.clultrain + 'push action ultrainio setpriv' + jsonArg(['utrio.msig', 1]) + '-p ultrainio@active')
    sleep(15)

def stepCreateStakedAccounts():
    for i in range(0, args.num_producers):
        retry(args.clultrain + 'create account ultrainio %s %s ' % (accounts[i], account_pk_list[i]))
    sleep(10)
    retry(args.clultrain + 'set contract hello  ' + args.contracts_dir + 'hello/')

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
    for i in range(1, args.num_producers):
        retry(args.clultrain + 'transfer ultrainio %s "%.4f UGAS"' % (accounts[i], 5000))
    sleep(15)
    for i in range(1, args.num_producers):
        retry(args.clultrain + 'system regproducer %s %s %s https://%s.com 0  %s' % (accounts[i], pk_list[i], bls_pk_list[i], accounts[i], accounts[i]))
    sleep(15)
    funds = 500000000 / args.num_producers / 2
    for i in range(1, args.num_producers):
        retry(args.clultrain + 'system delegatecons utrio.stake %s  "%.4f UGAS" ' % (accounts[i], (funds*2)))
    stepInitSimpleTest()
    sleep(15)
    run(args.clultrain + 'system listproducers')

def stepCreateinitAccounts():
    for a in initialAccounts:
        retry(args.clultrain + ' create account -u ultrainio %s %s ' % (a, args.initacc_pk))
    retry(args.clultrain + ' create account -u ultrainio hello %s ' % args.initacc_pk)
    sleep(10)
    for a in initialAccounts:
        retry(args.clultrain + 'transfer  ultrainio  %s  "%s UGAS" '  % (a,"100000000.0000"))
        retry(args.clultrain + 'system resourcelease ultrainio  %s  10 100' % a)
    retry(args.clultrain + 'system resourcelease ultrainio  hello  10 100')
    retry(args.clultrain + 'transfer ultrainio utrio.rand "10000 UGAS" ')
    retry(args.clultrain + 'set account permission utrio.rand active \'{"threshold":1,"keys": [{"key": "%s","weight": 1}],"accounts": [{"permission":{"actor":"utrio.rand","permission":"utrio.code"},"weight":1}]}\' owner -p utrio.rand' % (args.public_key))

def stepResign():
    resign('ultrainio', 'utrio.null')
#    for a in accountsToResign:
#        resign(a, 'utrio.null')

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
        retry(args.clultrain + 'create account -u ultrainio %s %s ' % (cur_accounts[i], acc_pk_list[i]))
    sleep(10)
    for a in cur_accounts:
        retry(args.clultrain + 'transfer ultrainio %s  "100.0000 UGAS"' % a)
    sleep(10)
    for i in range(0, pklen):
        retry(args.clultrain + 'system regproducer %s %s https://%s.com 0 ' % (cur_accounts[i], miner_pk_list[i], cur_accounts[i]))
    sleep(10)
    for i in range(0, pklen):
        retry(args.clultrain + 'system delegatecons utrio.stake %s  "1000000.0000 UGAS" ' % (cur_accounts[i]))

# Command Line Arguments

parser = argparse.ArgumentParser()

commands = [
    ('k', 'kill',           stepKillAll,                True,    "Kill all nodultrain and kultraind processes"),
    ('w', 'wallet',         stepStartWallet,            True,    "Start kultraind, create wallet, fill with keys"),
#    ('b', 'boot',           stepStartBoot,              True,    "Start boot node"),
    ('s', 'sys',            createSystemAccounts,       True,    "Create system accounts (utrio.*)"),
    ('c', 'contracts',      stepInstallSystemContracts, True,    "Install system contracts (token, msig)"),
    ('t', 'tokens',         stepCreateTokens,           True,    "Create tokens"),
    ('S', 'sys-contract',   stepSetSystemContract,      True,    "Set system contract"),
    ('i', 'create-initacc', stepCreateinitAccounts,     True,    "create initial accounts"),
    ('T', 'stake',          stepCreateStakedAccounts,   True,    "Create staked accounts"),
    ('I', 'initsimpletest', stepInitSimpleTest,         False,    "Simple transfer contract call test"),
    ('P', 'reg-prod',       stepRegProducers,           True,    "Register producers"),
#    ('R', 'claim',          claimRewards,               True,    "Claim rewards"),
     ('q', 'resign',         stepResign,                 False,    "Resign utrio"),
#    ('m', 'msg-replace',    msigReplaceSystem,          False,   "Replace system contract using msig"),
    ('X', 'xfer',           stepTransfer,               False,   "Random transfer tokens (infinite loop)"),
#    ('l', 'log',            stepLog,                    True,    "Show tail of node's log"),
    ('R', 'resourcetrans',  stepResourceTransaction,    False,    "resource transaction"),
    ('u', 'unregproducers',  stepunregproducersTest,    False,    "stepunregproducersTest"),
    ('r', 'regproducers',  stepregproducersTest,    False,    "stepregproducersTest"),
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
parser.add_argument('--num-producers', metavar='', help="Number of producers to register", type=int, default=6, dest="num_producers")
parser.add_argument('-a', '--all', action='store_true', help="Do everything marked with (*)")
parser.add_argument('-H', '--http-port', type=int, default=8000, metavar='', help='HTTP port for clultrain')
parser.add_argument('-p','--programpath', metavar='', help="set programpath params")
parser.add_argument('-m', '--masterchain', action='store_true', help="set current master chain")
parser.add_argument('-sub', '--subchain', type=str, help="set subchain name info")
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

adjustaccounts = ["genesis",]
if args.masterchain:
    for a in accounts[1:]:
        adjustaccounts.append("master"+a)
elif args.subchain:
   for a in accounts[1:]:
      adjustaccounts.append("user"+"."+args.subchain+a)
else:
    for a in accounts[1:]:
        adjustaccounts.append("user"+a)
accounts = adjustaccounts

#args.clultrain += '--url http://localhost:%d ' % args.http_port

logFile = open(args.log_path, 'a')

logFile.write('\n\n' + '*' * 80 + '\n\n\n')

haveCommand = False
for (flag, command, function, inAll, help) in commands:
    if getattr(args, command) or inAll and args.all:
        if function:
            haveCommand = True
            function()
if not haveCommand:
    print('bios-boot-tutorial.py: Tell me what to do. -a does almost everything. -h shows options.')
