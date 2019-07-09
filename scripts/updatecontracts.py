#!/usr/bin/env python

import argparse
import os
import random
import subprocess
import sys
import time
import requests
import json

args = None
logFile = None

defaultclu = '%s/ultrain-core/build/programs/clultrain/clultrain --wallet-url http://127.0.0.1:6666 '
defaultkul = '%s/ultrain-core/build/programs/kultraind/kultraind'
defaultcontracts_dir = '%s/ultrain-core/build/contracts/'

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


def updateAuth(account, permission, parent, controller):
    retry(args.clultrain + 'push action ultrainio updateauth' + jsonArg({
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

def updateAuthkey(url, account, permission, parent, key):
    retry(args.clultrain + ' -u ' + url + 'push action ultrainio updateauth' + jsonArg({
        'account': account,
        'permission': permission,
        'parent': parent,
        'auth': {
            'threshold': 1, 'keys': [{
        "key": key,
        "weight": 1
    }], 'waits': [],
            'accounts': []
        }
    }) + '-p ' + account + '@' + permission)

def updateMultiAuthActor(url,account, permission, parent, controller1, controller2, controller3):
    retry(args.clultrain + ' -u ' + url + 'push action ultrainio updateauth' + jsonArg({
        'account': account,
        'permission': permission,
        'parent': parent,
        'auth': {
            'threshold': 2, 'keys': [], 'waits': [],
            'accounts': [{
                'weight': 1,
                'permission': {'actor': controller1, 'permission': 'active'}
            },{
                'weight': 1,
                'permission': {'actor': controller2, 'permission': 'active'}
            },{
                'weight': 1,
                'permission': {'actor': controller3, 'permission': 'active'}
            }]
        }
    }) + '-p ' + account + '@' + permission)

destChaininfo = {
        "ultrainio":" http://127.0.0.1:8888 ",
        #"11":" http://172.16.10.5:8899 ",
}

testNetHttp = {
    "ultrainio":" http://ultrain.natapp1.cc",
    "11":"http://pioneer.natapp1.cc",
    "12":"http://power.natapp1.cc",
}
masterNetHttp = {
    "ultrainio":" https://ultrain.services -n ",
    "pioneer":" https://pioneer.ultrain.services -n ",
    "unitopia":" https://unitopia.ultrain.services -n ",
    "newretail":" https://new-retail.ultrain.services -n ",
    "australia":" https://australia.ultrain.services -n "
}
signAccount = [
    "master.111",
    "master.112",
    "master.113",
]
def ModifyAccountKey():
    accountlist = [
        "root1",
    ]
    for i in range(0, len(accountlist)):
        for url in destChaininfo.values():
            updateAuthkey(url,accountlist[i], 'owner', '', "UTR7ASYVwTkGPMzfc9X7QAFFrTHjSeVkQSFTiTNabyAqDQ5vK3cjf")
            updateAuthkey(url,accountlist[i], 'active', 'owner', "UTR7ASYVwTkGPMzfc9X7QAFFrTHjSeVkQSFTiTNabyAqDQ5vK3cjf")

def updateAccountAuth():
    accountlist = [
        'ultrainio',
        'utrio.token',
        'utrio.bank',
        'utrio.msig'
    ]
    for i in range(0, len(accountlist)):
        for url in destChaininfo.values():
            updateMultiAuthActor(url,accountlist[i], 'owner', '', signAccount[0], signAccount[1], signAccount[2])
            updateMultiAuthActor(url,accountlist[i], 'active', 'owner', signAccount[0], signAccount[1], signAccount[2])

def generateContractTrx():
    os.system("rm -rf contracttrx/")
    os.system("mkdir contracttrx/")
    for chain_name,url in destChaininfo.items():
        retry(args.clultrain + ' -u ' + url + 'set contract utrio.token ' + args.contracts_dir + 'ultrainio.token/   -d -j -s  -x 86400  >' + "contracttrx/"+chain_name+"token.json")
        retry(args.clultrain + ' -u ' + url + 'set contract utrio.msig ' + args.contracts_dir + 'ultrainio.msig/   -d -j -s  -x 86400  >' + "contracttrx/"+chain_name+"msig.json")
        retry(args.clultrain + ' -u ' + url + 'set contract utrio.bank ' + args.contracts_dir + 'ultrainio.bank/   -d -j -s  -x 86400  >' + "contracttrx/"+chain_name+"bank.json")
        retry(args.clultrain + ' -u ' + url + 'set contract ultrainio ' + args.contracts_dir + 'ultrainio.system/   -d -j -s  -x 86400  >' + "contracttrx/"+chain_name+"sys.json")
        #retry(args.clultrain + ' -u ' + url + ' transfer ultrainio root4 "100 UGAS"  -d -j -s  -x 86400  >' + "contracttrx/"+chain_name+"sys.json")     

def propose():
    perm = ''' '[{"actor": "root", "permission": "active"}, {"actor": "root1", "permission": "active"}, {"actor": "root2", "permission": "active"}]' '''
    for chain_name,url in destChaininfo.items():
        clupropose = args.clultrain + ' -u ' + url + ' multisig  propose_trx '
        retry(clupropose + chain_name+"token " + perm+  "contracttrx/"+chain_name+"token.json " + " utriomsig -p utriomsig@active")
        retry(clupropose + chain_name+"msig " + perm+  "contracttrx/"+chain_name+"msig.json "+ " utriomsig -p utriomsig@active")
        retry(clupropose + chain_name+"bank " + perm+ "contracttrx/"+chain_name+"bank.json " + " utriomsig -p utriomsig@active")
        #retry(clupropose + chain_name+"sys " + perm+  "contracttrx/"+chain_name+"sys.json "+ " utriomsig -p utriomsig@active")

def approve():
    for chain_name,url in destChaininfo.items():
        cluapprove = args.clultrain + ' -u ' + url + ' multisig  approve  utriomsig '
        for i in range(0,len(signAccount)):
            retry(cluapprove + chain_name+"token " + ''' '{"actor": "%s", "permission": "active"}'  -p  %s@active''' % (signAccount[i],signAccount[i]))
            retry(cluapprove + chain_name+"msig " +''' '{"actor": "%s", "permission": "active"}'  -p  %s@active''' % (signAccount[i],signAccount[i]))
            retry(cluapprove + chain_name+"bank " + ''' '{"actor": "%s", "permission": "active"}'  -p  %s@active''' % (signAccount[i],signAccount[i]))
            retry(cluapprove + chain_name+"sys " + ''' '{"actor": "%s", "permission": "active"}'  -p  %s@active''' % (signAccount[i],signAccount[i]))   

def execPropose():
    for chain_name,url in destChaininfo.items():
        cluexec = args.clultrain + ' -u ' + url + ' multisig  exec utriomsig '
        retry(cluexec + chain_name+"sys " + '''  -p  %s@active''' % ("utrio.mfee")) 

def updateContractTrx():
    for url in destChaininfo.values():
        retry(args.clultrain + ' -u ' + url + 'set contract utrio.token ' + args.contracts_dir + 'ultrainio.token/  -p utrio.token@active')
        retry(args.clultrain + ' -u ' + url + 'set contract utrio.bank ' + args.contracts_dir + 'ultrainio.bank/  -p utrio.bank@active')
        retry(args.clultrain + ' -u ' + url + 'set contract ultrainio ' + args.contracts_dir + 'ultrainio.system/  -p ultrainio@active')

def createAccount():
    accountList = ["u.mgr.zuofei","u.mgr.feifan","u.mgr.suyu"]
    pkList = [
    "UTR6r3BNrssrD9F5jJo17aszYN3S7mrYcCYqXGDgaEB1WPgF9LVfe",
    "UTR6r3BNrssrD9F5jJo17aszYN3S7mrYcCYqXGDgaEB1WPgF9LVfe",
    "UTR6r3BNrssrD9F5jJo17aszYN3S7mrYcCYqXGDgaEB1WPgF9LVfe",
    ]
    for i in range(0,len(accountList)):
        userName = accountList[i]
        pk = pkList[i]
        retry(args.clultrain+' create account ultrainio '+userName+' "'+pk+'" ')
        retry(args.clultrain+' system empoweruser '+userName+' 11 "'+pk+'" "'+pk+'" 1 -u ');
        # retry(args.clultrain+' system empoweruser '+userName+' pioneer "'+pk+'" "'+pk+'" 1 -u ');
        # retry(args.clultrain+' system empoweruser '+userName+' newretail "'+pk+'" "'+pk+'" 1 -u ');
        sleep(1)
# Command Line Arguments

parser = argparse.ArgumentParser()

commands = [
    ('k', 'mk',            ModifyAccountKey,           True,    "ModifyAccountKey"),
    ('A', 'ua',            updateAccountAuth,           True,    "updateAccountAuth"),
    ('g', 'gct',           generateContractTrx,        True,    "generateContractTrx"),
    ('p', 'prop',           propose,        True,    "propose"),
    ('a', 'appro',           approve,        True,    "approve"),
    ('e', 'execpropose',     execPropose,        True,    "execpropose"),
    ('u', 'updateContractTrx', updateContractTrx,        True,    "updateContractTrx"),
    ('c', 'createAccount', createAccount,        True,    "createAccount"),
]

parser.add_argument('--clultrain', metavar='', help="Clultrain command", default=defaultclu % '/root/workspace')
parser.add_argument('--kultraind', metavar='', help="Path to kultraind binary", default=defaultkul % '/root/workspace')
parser.add_argument('--contracts-dir', metavar='', help="Path to contracts directory", default=defaultcontracts_dir % '/root/workspace')
parser.add_argument('--genesis', metavar='', help="Path to genesis.json", default="./genesis.json")
parser.add_argument('--wallet-dir', metavar='', help="Path to wallet directory", default='./wallet/')
parser.add_argument('--log-path', metavar='', help="Path to log file", default='./output.log')
parser.add_argument('--symbol', metavar='', help="The utrio.system symbol", default='UGAS')
parser.add_argument('-al', '--all', action='store_true', help="Do everything marked with (*)")
parser.add_argument('-H', '--http-port', type=int, default=8000, metavar='', help='HTTP port for clultrain')
parser.add_argument('-pr','--programpath', metavar='', help="set programpath params")

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

logFile = open(args.log_path, 'a')

logFile.write('\n\n' + '*' * 80 + '\n\n\n')

haveCommand = False
for (flag, command, function, inAll, help) in commands:
    if getattr(args, command) or inAll and args.all:
        if function:
            haveCommand = True
            function()
if not haveCommand:
    print('updatecontracts.py: Tell me what to do. -a does almost everything. -h shows options.')
