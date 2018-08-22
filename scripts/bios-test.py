#!/usr/bin/env python3
 
import argparse
import json
import numpy
import os
import random
import re
import subprocess
import sys
import time
import string

args = None
logFile = None

unlockTimeout = 999999999

systemAccounts = [
    'utrio.bpay',
    'utrio.msig',
    'utrio.names',
    'utrio.ram',
    'utrio.ramfee',
    'utrio.saving',
    'utrio.stake',
    'utrio.token',
    'utrio.vpay',
]

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

def randomTransfer():
    for i in list('12345abcdefghijklmnopqrstuvwxyz'):
        for j in list('12345abcdefghijklmnopqrstuvwxyz'):
            simple_run(args.clultrain + 'transfer -f user.%s user.%s "0.%s SYS" ' %(i, j, random.randint(1, 999)))
    sleep(1)
    
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
    for a in systemAccounts:
        run(args.clultrain + 'create account ultrainio ' + a + ' ' + args.public_key)

def stepInstallSystemContracts():
    run(args.clultrain + 'set contract utrio.token ' + args.contracts_dir + 'ultrainio.token/')
    run(args.clultrain + 'set contract utrio.msig ' + args.contracts_dir + 'ultrainio.msig/')
    sleep(20)

def stepCreateTokens():
    run(args.clultrain + 'push action utrio.token create \'["ultrainio", "10000000000.0000 SYS"]\' -p utrio.token')
    run(args.clultrain + 'push action utrio.token issue \'["ultrainio", "10000000.0000 SYS", "memo"]\' -p ultrainio')
    sleep(15)

def stepSetSystemContract():
    retry(args.clultrain + 'set contract ultrainio ' + args.contracts_dir + 'ultrainio.system/')
    run(args.clultrain + 'push action ultrainio setpriv' + jsonArg(['utrio.msig', 1]) + '-p ultrainio@active')
    sleep(15)
    
def stepCreateStakedAccounts():
    for i in list('12345abcdefghijklmnopqrstuvwxyz'):
        retry(args.clultrain + 'system newaccount --transfer ultrainio user.%s %s --stake-net "0.0010 SYS" --stake-cpu "0.0010 SYS" --buy-ram "0.1000 SYS" ' % (i,args.public_key))
        retry(args.clultrain + 'transfer ultrainio user.%s "500.0000 SYS"' % (i))

    sleep(15)

def stepTransfer():
    while True:
        randomTransfer()

# Command Line Arguments

parser = argparse.ArgumentParser()

commands = [
    ('k', 'kill',           stepKillAll,                True,    "Kill all nodeos and kultraind processes"),
    ('w', 'wallet',         stepStartWallet,            True,    "Start kultraind, create wallet, fill with keys"),
#    ('b', 'boot',           stepStartBoot,              True,    "Start boot node"),
    ('s', 'sys',            createSystemAccounts,       True,    "Create system accounts (utrio.*)"),
    ('c', 'contracts',      stepInstallSystemContracts, True,    "Install system contracts (token, msig)"),
    ('t', 'tokens',         stepCreateTokens,           True,    "Create tokens"),
    ('S', 'sys-contract',   stepSetSystemContract,      True,    "Set system contract"),
    ('T', 'stake',          stepCreateStakedAccounts,   True,    "Create staked accounts"),
#    ('p', 'reg-prod',       stepRegProducers,           True,    "Register producers"),
#    ('P', 'start-prod',     stepStartProducers,         True,    "Start producers"),
#    ('v', 'vote',           stepVote,                   True,    "Vote for producers"),
#    ('R', 'claim',          claimRewards,               True,    "Claim rewards"),
#    ('x', 'proxy',          stepProxyVotes,             True,    "Proxy votes"),
#    ('q', 'resign',         stepResign,                 True,    "Resign utrio"),
#    ('m', 'msg-replace',    msigReplaceSystem,          False,   "Replace system contract using msig"),
    ('X', 'xfer',           stepTransfer,               True,   "Random transfer tokens (infinite loop)"),
#    ('l', 'log',            stepLog,                    True,    "Show tail of node's log"),
]

parser.add_argument('--public-key', metavar='', help="EOSIO Public Key", default='UTR7vfv95bvjB54jZ69yaQqLxZkWNaC9xAjYG7Dq1bW1zpJKD2tkP', dest="public_key")
parser.add_argument('--private-Key', metavar='', help="EOSIO Private Key", default='5JNuk2NHzJhhc5KCgZDnkD1fj9T6ThTScejzXjQPajWddm4PVma', dest="private_key")
parser.add_argument('--clultrain', metavar='', help="Clultrain command", default='/root/workspace/yufengshen/ultrain-core/build/programs/clultrain/clultrain --wallet-url http://127.0.0.1:6666 ')
parser.add_argument('--nodeos', metavar='', help="Path to nodeos binary", default='../../build/programs/nodeos/nodeos')
parser.add_argument('--kultraind', metavar='', help="Path to kultraind binary", default='/root/workspace/yufengshen/ultrain-core/build/programs/kultraind/kultraind')
parser.add_argument('--contracts-dir', metavar='', help="Path to contracts directory", default='/root/workspace/yufengshen/ultrain-core/build/contracts/')
parser.add_argument('--nodes-dir', metavar='', help="Path to nodes directory", default='./nodes/')
parser.add_argument('--genesis', metavar='', help="Path to genesis.json", default="./genesis.json")
parser.add_argument('--wallet-dir', metavar='', help="Path to wallet directory", default='./wallet/')
parser.add_argument('--log-path', metavar='', help="Path to log file", default='./output.log')
parser.add_argument('--symbol', metavar='', help="The utrio.system symbol", default='SYS')
parser.add_argument('--user-limit', metavar='', help="Max number of users. (0 = no limit)", type=int, default=3000)
parser.add_argument('--max-user-keys', metavar='', help="Maximum user keys to import into wallet", type=int, default=10)
parser.add_argument('--ram-funds', metavar='', help="How much funds for each user to spend on ram", type=float, default=0.1)
parser.add_argument('--min-stake', metavar='', help="Minimum stake before allocating unstaked funds", type=float, default=0.9)
parser.add_argument('--max-unstaked', metavar='', help="Maximum unstaked funds", type=float, default=10)
parser.add_argument('--producer-limit', metavar='', help="Maximum number of producers. (0 = no limit)", type=int, default=0)
parser.add_argument('--min-producer-funds', metavar='', help="Minimum producer funds", type=float, default=1000.0000)
parser.add_argument('--num-producers-vote', metavar='', help="Number of producers for which each user votes", type=int, default=20)
parser.add_argument('--num-voters', metavar='', help="Number of voters", type=int, default=10)
parser.add_argument('--num-senders', metavar='', help="Number of users to transfer funds randomly", type=int, default=10)
parser.add_argument('--producer-sync-delay', metavar='', help="Time (s) to sleep to allow producers to sync", type=int, default=80)
parser.add_argument('-a', '--all', action='store_true', help="Do everything marked with (*)")
parser.add_argument('-H', '--http-port', type=int, default=8000, metavar='', help='HTTP port for clultrain')

for (flag, command, function, inAll, help) in commands:
    prefix = ''
    if inAll: prefix += '*'
    if prefix: help = '(' + prefix + ') ' + help
    if flag:
        parser.add_argument('-' + flag, '--' + command, action='store_true', help=help, dest=command)
    else:
        parser.add_argument('--' + command, action='store_true', help=help, dest=command)

args = parser.parse_args()

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
