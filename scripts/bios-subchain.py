#!/usr/bin/env python

import argparse
import json
import os
import random
import re
import subprocess
import sys
import time
import string
import json
from account_info import *
#from account_info_master import *
#from account_info_sub1 import *
#from account_info_sub2 import *
#from account_info_sub3 import *

logFile = None

unlockTimeout = 999999999
defaultclu = '%s/ultrain-core/build/programs/clultrain/clultrain --wallet-url http://127.0.0.1:6666 '
defaultkul = '%s/ultrain-core/build/programs/kultraind/kultraind'
defaultcontracts_dir = '%s/ultrain-core/build/contracts/'

# accoding to location in accoint_info file : user111-user115
startl=1;
endl=5;

local = True;

def sleep(t):
    print('sleep', t, '...')
    time.sleep(t)
    print('resume')

def run(args,stop):
    print('bios-boot-tutorial.py:', args)
    logFile.write(args + '\n')
    if subprocess.call(args, shell=True):
        print('bios-subchain.py: exiting because of error')
        if stop == True:
            sys.exit(1)

def retry(args):
    time = 0
    while True:
        if time >= 30 :
            break;
        print('bios-subchain.py:', args)
        logFile.write(args + '\n')
        if subprocess.call(args, shell=True):
            print('*** Retry')
            time=time+1;
            sleep(0.5)
        else:
            break


def background(args):
    print('bios-boot-tutorial.py:', args)
    logFile.write(args + '\n')
    return subprocess.Popen(args, shell=True)

def stepKillAll():
    run('killall kultraind || true',False)
    sleep(1.5)


def startWallet():
    run('rm -rf ' + os.path.abspath(args.wallet_dir),False)
    run('mkdir -p ' + os.path.abspath(args.wallet_dir),False)
    background(args.kultraind + ' --unlock-timeout %d --http-server-address 127.0.0.1:6666 --wallet-dir %s' % (unlockTimeout, os.path.abspath(args.wallet_dir)))
    sleep(1)
    run(args.clultrain + 'wallet create',False)

def importKeys():
    run(args.clultrain + 'wallet import --private-key ' + args.private_key,False)
    run(args.clultrain + 'wallet import --private-key ' + args.initacc_sk,False)
    run(args.clultrain + 'wallet import --private-key  5KG6NiRGhsEP9vTf4WVe312iVQ3uemEXsstsqkT9Wj1MkdY5uJk',False)
    for i in range(0, len(account_sk_list)):
        run(args.clultrain + 'wallet import --private-key ' + account_sk_list[i],False)
    for i in range(0, len(rand_sk_lst)):
        run(args.clultrain + 'wallet import --private-key ' + rand_sk_lst[i],False)

def stepStartWallet():
    startWallet()
    importKeys()


def getUserName(i):
    if local == True:
        return args.subchain + accounts[i];
    else:
        return accounts[i];




# addSubChainUse
def addSubChainUser():
    print "addSubChainUser start..."
    endindex = endl;
    if args.producerNum :
        endindex = args.producerNum;
    for i in range(startl,endindex+1):
        userName = getUserName(i);
        pk = account_pk_list[i]
        print "add new user:" + userName + "(" + pk + ") belongs to chain(" + args.subchain + ")"
        retry(args.clultrain + 'create account -u ultrainio ' + userName + ' ' + pk)
        sleep(0.5)
    print "addSubChainUser end..."
    sleep(2)


# add balance to user
def addBalanceToUser():
    print "addBalanceToUser start..."
    endindex = endl;
    if args.producerNum :
        endindex = args.producerNum;
    for i in range(startl,endindex+1):
        userName = getUserName(i);
        print "transfer to  user(" + userName + ") 20.0000 UGAS"
        retry(args.clultrain + 'transfer ultrainio ' + userName + ' "20.0000 UGAS"')
        sleep(0.5)
    print "addBalanceToUser end..."
    sleep(2)

# reg producer
def regProducer():
    print "regProducer start..."
    endindex = endl;
    if args.producerNum :
        endindex = args.producerNum;
    for i in range(startl,endindex+1):
        userName = getUserName(i);
        userPK = account_pk_list[i]
        pk = pk_list[i]
        bls_key = bls_pk_list[i];
        print "empoweruser(user:"+userName+" to chain:"+args.subchain+")"
        retry(args.clultrain+'system empoweruser '+userName+' '+args.subchain+' -p '+userName+'@active');
        sleep(1)
        print "reg producer:" + userName + "(" + pk + " "+bls_key+ ") belongs to chain(" + args.subchain + ")"
        retry(args.clultrain + 'system regproducer ' + userName +' '+pk+' '+bls_key + ' ' + userName+' https://'+userName+'.com '+args.subchain +' -p ultrainio@active -u')
        sleep(0.5)
    print "regProducer end..."
    sleep(2)

# delegateStark
def delegateStark():
    print "delegateStark start..."

    endindex = endl;
    if args.producerNum :
        endindex = args.producerNum;
    for i in range(startl,endindex+1):
        userName = getUserName(i);
        print "delegatecons:" + userName + " 42000.0000 UGAS"
        retry(args.clultrain + 'push action ultrainio delegatecons \'{"from":"utrio.stake", "receiver":"'+userName+'", "stake_cons_quantity":"42000.0000 UGAS"}\' -p utrio.stake@active')
        sleep(0.5)
    print "delegateStark end..."
    sleep(2)

# clear subchain
def clearSubchain():
    print "clearSubchain start..."
    print "clearSubchain end..."

# addSubchain()
def addSubchain():
    print "addSubchain start..."
    print "add a new subchain info(name:"+args.subchain+")"
    typeId=1;
    if args.chainType :
        array=args.chainType.split(',')
        typeId=array[0]
    retry(args.clultrain + ' push action ultrainio regsubchain \'{"chain_name": "'+args.subchain+'", "chain_type": "'+typeId+'","genesis_time":"2019-01-01 21:00:00"}\' -p ultrainio@active');
    sleep(1)
    print "addSubchain end..."

# add chain_type
# clu push action ultrainio regchaintype '{"type_id": "1", "min_producer_num": "40", "max_producer_num": "200", "sched_step": "10", "consensus_period": "10"}' -p ultrainio@active
def addChainType():
    print "addChainType start..."
    typeId=1;
    minP=4
    maxP=6
    step=2
    if args.chainType :
        array=args.chainType.split(',')
        typeId=array[0]
        minP=array[1]
        maxP=array[2]
        step=array[3]
    retry(args.clultrain + ' push action ultrainio regchaintype \'{"type_id": "'+typeId+'", "min_producer_num": "'+minP+'", "max_producer_num": "'+maxP+'", "sched_step": "'+step+'", "consensus_period": "10"}\' -p ultrainio@active');
    print "addChainType end..."


def showSubchain():
    run(args.clultrain + ' get table ultrainio ultrainio chains',False);

# Command Line Arguments
commands = [
    ('k', 'kill',           stepKillAll,                True,    "Kill all nodultrain and kultraind processes"),
    ('w', 'wallet',         stepStartWallet,            True,    "Start kultraind, create wallet, fill with keys"),
    ('A', 'addSubChainType', addChainType, True, "add a new subchain type"),
    ('N', 'newSubchain', addSubchain, True, "add a new subchain"),
    ('U', 'addUser', addSubChainUser, True, "add subchain users in mainchain"),
    ('B', 'addBalance', addBalanceToUser, False, "add balance to subchain users"),
    ('P', 'regProd', regProducer, True, "register producers"),
    ('D', 'delegate', delegateStark, True, "delegate cons"),
    ('S', 'show', showSubchain, True, "show sunchains"),
    # ('C', 'clearSubchain', clearSubchain, True, "clear subchain's block and users"),
];
parser = argparse.ArgumentParser()
parser.add_argument('-sub', '--subchain', type=str, help="set subchain name info")
parser.add_argument('-ct', '--chainType', type=str, help="chain type data")
parser.add_argument('-a', '--all', action='store_true', help="Do everything marked with (*)")
parser.add_argument('-p', '--programpath', metavar='', help="set programpath params")
parser.add_argument('--clultrain', metavar='', help="Clultrain command", default=defaultclu % '/root/workspace')
parser.add_argument('--kultraind', metavar='', help="Path to kultraind binary", default=defaultkul % '/root/workspace')
parser.add_argument('--contracts-dir', metavar='', help="Path to contracts directory",
                    default=defaultcontracts_dir % '/root/workspace')
parser.add_argument('--log-path', metavar='', help="Path to log file", default='./output.log')
parser.add_argument('-pn', '--producerNum', type=int, help="set producerNum to create")
parser.add_argument('--wallet-dir', metavar='', help="Path to wallet directory", default='./wallet/')
parser.add_argument('--public-key', metavar='', help="ULTRAIN Public Key", default='UTR5t23dcRcnpXTTT7xFgbBkrJoEHvKuxz8FEjzbZrhkpkj2vmh8M', dest="public_key")
parser.add_argument('--private-Key', metavar='', help="ULTRAIN Private Key", default='5HvhChtH919sEgh5YjspCa1wgE7dKP61f7wVmTPsedw6enz6g7H', dest="private_key")
parser.add_argument('--initacc-pk', metavar='', help="ULTRAIN Public Key", default='UTR6XRzZpgATJaTtyeSKqGhZ6rH9yYn69f5fkLpjVx6y2mEv5iQTn', dest="initacc_pk")
parser.add_argument('--initacc-sk', metavar='', help="ULTRAIN Private Key", default='5KZ7mnSHiKN8VaJF7aYf3ymCRKyfr4NiTiqKC5KLxkyM56KdQEP', dest="initacc_sk")

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

# print(args.clultrain)
# print(args.kultraind)
# print(args.contracts_dir)


# log info
logFile = open(args.log_path, 'a')
logFile.write('\n\n' + '*' * 80 + '\n\n\n')



# Entry Point
haveCommand = False
for (flag, command, function, inAll, help) in commands:
    if getattr(args, command) or inAll and args.all:
        if function:
            haveCommand = True
            function()
if not haveCommand:
    print('bios-subchain.py: Tell me what to do. -a does almost everything. -h shows options.')
