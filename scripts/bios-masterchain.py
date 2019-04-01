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
#from account_info import *
from account_info_master import *
#from account_info_sub1 import *
#from account_info_sub2 import *
#from account_info_sub3 import *

logFile = None

unlockTimeout = 999999999
defaultclu = '%s/ultrain-core/build/programs/clultrain/clultrain --wallet-url http://127.0.0.1:6666 '
defaultkul = '%s/ultrain-core/build/programs/kultraind/kultraind'
defaultcontracts_dir = '%s/ultrain-core/build/contracts/'


initaccount = accounts

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


def addMasterChainInfo():
    masterproducerinfo = ""
    for i in range(1, args.num_master_prods + 1):
        masterproducerinfo += '{"owner":"%s","producer_key":"%s","bls_key":"%s"},' % ((initaccount[i]), pk_list[i], bls_pk_list[i] )
    masterproducerinfo = masterproducerinfo[:-1]
    retry(args.clultrain + ' push action ultrainio setmasterchaininfo \'{"chaininfo":{"owner": "ultrainio",\
        "master_prods":[%s],"block_height":%s,"block_id":"%s","master_chain_ext":[]}}\' -p ultrainio ' % \
          ( masterproducerinfo, args.num_master_block, args.master_block_id) )

# Command Line Arguments
commands = [
    ('A', 'addMaterChain', addMasterChainInfo, True, "addMasterchain info"),
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
parser.add_argument('--num-master-prods', metavar='', help="Number of master producers to register", type=int, default=5, dest="num_master_prods")
parser.add_argument('--num-master-block', metavar='', help="Number of master chain block height ", type=int, default=0, dest="num_master_block")
parser.add_argument('--master-block-id', metavar='', help="Number of master chain block height id ", type=str, default="", dest="master_block_id")

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
