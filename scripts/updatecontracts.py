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

def updateAuthkey(clu, account, permission, parent, key):
    retry(clu + ' push action ultrainio updateauth ' + jsonArg({
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

def updateMultiAuthActor(clu,account, permission, parent, controller1, controller2, controller3):
    retry(clu + ' push action ultrainio updateauth ' + jsonArg({
        'account': account,
        'permission': permission,
        'parent': parent,
        'auth': {
            'threshold': 2, 'keys': [], 'waits': [],
            'accounts': [{
                'weight': 1,
                'permission': {'actor': controller1, 'permission': permission}
            },{
                'weight': 1,
                'permission': {'actor': controller2, 'permission': permission}
            },{
                'weight': 1,
                'permission': {'actor': controller3, 'permission': permission}
            }]
        }
    }) + '-p ' + account + '@' + permission)

destChaininfo = {
        "ultrainio":"http://127.0.0.1:8888",
        #"11":" http://172.16.10.5:8899 ",
}

testNetHttp = {
    "ultrainio":"http://172.16.10.6:8888",
    "11":"http://172.16.10.6:8889",
    "12":"http://172.16.10.6:8890",
    # "13":"http://172.16.10.6:9911",
}
masterNetHttp = {
    "ultrainio":"http://40.121.11.165:8888",#  "ultrainio":"https://ultrain.services",
    "pioneer":"http://139.217.87.141:8888",#  "pioneer":"https://pioneer.ultrain.services",
    "unitopia":"http://40.121.21.140:8888",#  "unitopia":"https://unitopia.ultrain.services",
    "newretail":"http://139.217.96.141:8888",#  "newretail":"https://new-retail.ultrain.services",
    "australia":"http://120.92.210.60:8888",#"australia":"https://australia.ultrain.services"
}
signAccount = ["u.mgr.feifan","u.mgr.suyu","u.mgr.zuofei"]
def ModifyAccountKey():
    accountlist = [
        # "u.mgr.feifan",
        'hello',
        'root',
        'root1',
        'root2',
        'root3',
        'root4',
        'root5',
    ]
    for i in range(0, len(accountlist)):
        for url in testNetHttp.values():
            clu = args.clultrain + ' -u ' + url + ' -n '
            retry(clu + ' transfer utrio.rand %s  "2.4 UGAS" ' % accountlist[i] )
            updateAuthkey(clu,accountlist[i], 'owner', '', "UTR5x7SYcRsvhh6B5yYQDNtbaDMoXuLM9wA5gMAky4fQ1EMLCeRyL")
            updateAuthkey(clu,accountlist[i], 'active', 'owner', "UTR5x7SYcRsvhh6B5yYQDNtbaDMoXuLM9wA5gMAky4fQ1EMLCeRyL")
            # retry(clu + ' transfer utrio.fee  ultrainio  "2.4 UGAS" ' )

def updateAccountAuth():
    accountlist = [
        'ultrainio',
        'utrio.token',
        'utrio.bank',
        'utrio.msig',
        'utrio.fee',
        'utrio.reward',
        'utrio.cmnity',
        'utrio.thteam',
        'utrio.dapp'
    ]
    for i in range(0, len(accountlist)):
        for url in masterNetHttp.values():
            clu = args.clultrain + ' -u ' + url + ' '
            updateMultiAuthActor(clu,accountlist[i], 'owner', '', signAccount[0], signAccount[1], signAccount[2])
            updateMultiAuthActor(clu,accountlist[i], 'active', 'owner', signAccount[0], signAccount[1], signAccount[2])

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
    for url in masterNetHttp.values():
        clu = args.clultrain + ' -u ' + url
        retry(clu + ' set contract utrio.token ' + args.contracts_dir + 'ultrainio.token/  -p utrio.token@active')
      #   retry(clu + ' set contract utrio.bank ' + args.contracts_dir + 'ultrainio.bank/  -p utrio.bank@active')
        retry(clu + ' set contract ultrainio ' + args.contracts_dir + 'ultrainio.system/  -p ultrainio@active')

def verifyContractCode():
    print("utrio.token code:")
    for url in masterNetHttp.values():
        clu = args.clultrain + ' -u ' + url
        retry(clu + ' get code  utrio.token ')
    print("ultrainio code:")
    for url in masterNetHttp.values():
        clu = args.clultrain + ' -u ' + url
        retry(clu + ' get code ultrainio ' )


def createAccount():
    accountList = [
        'utrio.mfee',
        'utrio.reward',
      #   'utrio.cmnity',
      #   'utrio.thteam',
      #   'utrio.dapp'
        ]
    pkList = [
    "UTR6r3BNrssrD9F5jJo17aszYN3S7mrYcCYqXGDgaEB1WPgF9LVfe",
    "UTR6r3BNrssrD9F5jJo17aszYN3S7mrYcCYqXGDgaEB1WPgF9LVfe",
    #"UTR6r3BNrssrD9F5jJo17aszYN3S7mrYcCYqXGDgaEB1WPgF9LVfe",
    ]
    clu = args.clultrain + " -u  https://ultrain.services -n "
    for i in range(0,len(accountList)):
        userName = accountList[i]
        pk = pkList[i]
        simple_run(clu +' create account ultrainio '+userName+' "'+pk+'" ')
        sleep(20)
        simple_run(clu +' system empoweruser '+userName+' pioneer "'+pk+'" "'+pk+'" 1 -u ');
        simple_run(clu +' system empoweruser '+userName+' unitopia "'+pk+'" "'+pk+'" 1 -u ');
        simple_run(clu +' system empoweruser '+userName+' newretail "'+pk+'" "'+pk+'" 1 -u ');
        simple_run(clu +' system empoweruser '+userName+' australia "'+pk+'" "'+pk+'" 1 -u ');
        

def verifycreateisexist():
    accountList = [        'utrio.cmnity',
        'utrio.thteam',
        #'utrio.dapp'
        ]
    noexistaccount = []
    for i in range(0,len(accountList)):
        for url in masterNetHttp.values():
            j = json.loads(requests.get(url+"/v1/chain/get_account_exist",data = json.dumps({"account_name":accountList[i]})).text)
            if j["is_exist"] == False :
                noexistaccount.append(accountList[i]);
    print ("no exist account size: %d" % (len(noexistaccount)) )
    for a in noexistaccount:
        print(a)
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
    ('V', 'verifyContractCode', verifyContractCode,        True,    "verifyContractCode"),
    ('c', 'createAccount', createAccount,        True,    "createAccount"),
    ('v', 'verifycreateisexist', verifycreateisexist,        True,    "verifycreateisexist"),
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
