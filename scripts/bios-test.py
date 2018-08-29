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

accounts = [
    'user.11',
    'user.12',
    'user.13',
    'user.14',
    'user.15',
    'user.21',
    'user.22',
    'user.23',
    'user.24',
    'user.25',
    'user.31',
    'user.32',
    'user.33',
    'user.34',
    'user.35',
    'user.41',
    'user.42',
    'user.43',
    'user.44',
    'user.45',
    'user.51',
    'user.52',
    'user.53',
    'user.54',
    'user.55',
]

pk_list = ["b3f88e7694995cf2d46fb9bbe172b1e9d2ae8ea372ec26c01a6603bd415dc64d",
           "e2e7339522395916f941c49b3d58dfc4c0c61e0e3910fcf568b3c2ce2005e32b",
           "92f7b32418e79b2a4ba716f6745c361381411f0537376e438b2399486ed0c8dc",
           "4141e8c7a4780df3cf840ed556d52108b08a3bc2ead12bece6bc06b9d9487eb2",
           "933c5ceddf3d27af114351112c131f1bb4001a6a6669449365b204441db181a3",
           "6fadc36ba297d6db53ec0a094c27a32ee266ab17a63cfa149609edfe881c7118",
           "4ae81777689da3f6c6972effa4857cd32ddd3466fef42cb281babc0198546faa"
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
    for i in accounts:
        for j in accounts:
            simple_run(args.clultrain + 'transfer -f %s %s "0.%s SYS" ' %(i, j, random.randint(1, 999)))
    sleep(3)

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
    retry(args.clultrain + 'set contract utrio.token ' + args.contracts_dir + 'ultrainio.token/')
    retry(args.clultrain + 'set contract utrio.msig ' + args.contracts_dir + 'ultrainio.msig/')
    sleep(20)

def stepCreateTokens():
    run(args.clultrain + 'push action utrio.token create \'["ultrainio", "1000000000.0000 SYS"]\' -p utrio.token')
    run(args.clultrain + 'push action utrio.token issue \'["ultrainio", "600000000.0000 SYS", "memo"]\' -p ultrainio')
    sleep(15)

def stepSetSystemContract():
    retry(args.clultrain + 'set contract ultrainio ' + args.contracts_dir + 'ultrainio.system/')
    retry(args.clultrain + 'push action ultrainio setpriv' + jsonArg(['utrio.msig', 1]) + '-p ultrainio@active')
    sleep(15)

def stepCreateStakedAccounts():
    for a in accounts:
        retry(args.clultrain + 'system newaccount --transfer ultrainio %s %s --stake-net "11000000.1234 SYS" --stake-cpu "11000000.5678 SYS" --buy-ram "1000.000 SYS" ' % (a,args.public_key))
        retry(args.clultrain + 'transfer ultrainio %s "5000.0000 SYS"' % (a))
    sleep(15)

def stepRegProducers():
    for i in range(0, args.num_producers):
        retry(args.clultrain + 'system regproducer %s %s https://%s.com 0123 ' % (accounts[i], pk_list[i], accounts[i]))
    sleep(1)
    run(args.clultrain + 'system listproducers')

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
    ('p', 'reg-prod',       stepRegProducers,           True,    "Register producers"),
#    ('P', 'start-prod',     stepStartProducers,         True,    "Start producers"),
#    ('v', 'vote',           stepVote,                   True,    "Vote for producers"),
#    ('R', 'claim',          claimRewards,               True,    "Claim rewards"),
#    ('x', 'proxy',          stepProxyVotes,             True,    "Proxy votes"),
#    ('q', 'resign',         stepResign,                 True,    "Resign utrio"),
#    ('m', 'msg-replace',    msigReplaceSystem,          False,   "Replace system contract using msig"),
#    ('X', 'xfer',           stepTransfer,               True,   "Random transfer tokens (infinite loop)"),
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
parser.add_argument('--num-producers', metavar='', help="Number of producers to register", type=int, default=7, dest="num_producers")
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
