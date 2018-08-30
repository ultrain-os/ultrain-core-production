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
"user.111",
"user.112",
"user.113",
"user.114",
"user.115",
"user.121",
"user.122",
"user.123",
"user.124",
"user.125",
"user.131",
"user.132",
"user.133",
"user.134",
"user.135",
"user.141",
"user.142",
"user.143",
"user.144",
"user.145",
"user.151",
"user.152",
"user.153",
"user.154",
"user.155",
"user.211",
"user.212",
"user.213",
"user.214",
"user.215",
"user.221",
"user.222",
"user.223",
"user.224",
"user.225",
"user.231",
"user.232",
"user.233",
"user.234",
"user.235",
"user.241",
"user.242",
"user.243",
"user.244",
"user.245",
"user.251",
"user.252",
"user.253",
"user.254",
"user.255",
"user.311",
"user.312",
"user.313",
"user.314",
"user.315",
"user.321",
"user.322",
"user.323",
"user.324",
"user.325",
"user.331",
"user.332",
"user.333",
"user.334",
"user.335",
"user.341",
"user.342",
"user.343",
"user.344",
"user.345",
"user.351",
"user.352",
"user.353",
"user.354",
"user.355",
"user.411",
"user.412",
"user.413",
"user.414",
"user.415",
"user.421",
"user.422",
"user.423",
"user.424",
"user.425",
"user.431",
"user.432",
"user.433",
"user.434",
"user.435",
"user.441",
"user.442",
"user.443",
"user.444",
"user.445",
"user.451",
"user.452",
"user.453",
"user.454",
]

#pk_list = ["b3f88e7694995cf2d46fb9bbe172b1e9d2ae8ea372ec26c01a6603bd415dc64d",
#           "e2e7339522395916f941c49b3d58dfc4c0c61e0e3910fcf568b3c2ce2005e32b",
#           "92f7b32418e79b2a4ba716f6745c361381411f0537376e438b2399486ed0c8dc",
#           "4141e8c7a4780df3cf840ed556d52108b08a3bc2ead12bece6bc06b9d9487eb2",
#           "933c5ceddf3d27af114351112c131f1bb4001a6a6669449365b204441db181a3",
#           "6fadc36ba297d6db53ec0a094c27a32ee266ab17a63cfa149609edfe881c7118",
#           "4ae81777689da3f6c6972effa4857cd32ddd3466fef42cb281babc0198546faa"
#           ]

pk_list = ["b7e0a16fdca44d4ece1b14d8e7e6207402a6447115ca7d2d7edb08958e6d8ed5",
"4031a95071a092eca8646d3999c438bdfde368d4837770755af65a11b4520a48",
"2b43a3d8e0523a85141bbfca41006cf9abc47587d0ff3c46d257551ec05f1677",
"b9a55c3c661abd8c539b7a7c05c8176036f87aeeb6b117a138327cfdb374cc23",
"8f6e3b3336276138023617f0ae0e6fd0c37d27aa9995f9803fe78df4941dd3ec",
"3696fdfb2a9716ae4b1bca0d5b0a861a1e5d64562994aeb515eed49290c9f1c2",
"578451935c370d9c7fbcdd77e35a40e49bf0a5311e065035778ac27e6263b10d",
"9ea2644ec77f3ecde8a47db1c197e63088c91af3edf186c54038f90650056c3d",
"eb185504c078a9c87f6042746d03566183b0217cb4a12ae202c235b175955441",
"a380f12af673da43e1e8f836f4a2621200ff737342892d80c3a8ef597e949413",
"1aef98b82a525ae61220b7e34270eb7362cb2537805694e6179afb73ac638313",
"cecbfd84857edc1a5fa2e945bec767dbfc816f5dc3b9a2d98920d6826f839cfc",
"c1052bdd25d892f446268deb6c36b1c94252145067fb1990d5afe8150d789fd8",
"c7e4c4fd86b08bd15939bcb82a3f2bc7621bfb606797f483ecd9afd8c61dd94a",
"c42fbeec466677bb873abb62ab8363942f60dd30cdfb4345f7b658c1d375ee3d",
"b5b96cd09fb3bf098c9a8b6c8453e641618ab7477bdcc02695d5c64351b4474e",
"7038618da8a7bcc68797ea5e529d48749501b48d8d50b636ca7c7177c232c7cd",
"9a5621d38478261d77919ce89c64e1bcfd892a54f9aab3f55c861120a632b293",
"2a8aad345502e8351294886151306dfade499a8891df991e2da449c40481479f",
"3491bda3185f56632c25da89ccc7c61a7140546c8e9014021d845dc399265f3c",
"aaacec343513dda6ac912e23a16e7c4a97855cedb732599c99d22b0ad9124e9b",
"1263638be8d736c9cc313c2bbcbda1b95c0677187c286f2f7db15ab3bbf6f08f",
"9fb6e571e31e3c053cc40eb5ac71fc9a1e062c41e219b902dcfc7fb5e05aac13",
"f17273e68bc07668464dd5e8a8d96dc2a9d479b5948627d04ec03e721c172028",
"77b038699c3f4bc665b71187e64678c39fd6e83230aaeae9ba1c720f22959c94",
"febb254bb517abe8302bc702ffba746b788ca8135d31008e0c2ba91263346430",
"bef327a19755173824c85e299f1ecb0d79f7d655a06a362236e5033efda5d3e7",
"e229c2d7a28a0c606d514d0bc94cd1b7219da8cd49bc80dedef24d49028aadec",
"e14a187953ed64a29fccc5c9f5f402b7d327672bf08589cc45060c3bc268114f",
"127d47deed92b0d06f7abfc6a74ba7bee94908cf47c23399ca06d772c6e9e8a3",
"90d5f71464791141f580b30e36e575b2cf51741a3b4a811a416d319f9af15834",
"1148e1559ef544f396e94843ac85291e6997c9b019fca62a0eb959ff892f6398",
"08ffd968a7d5b18d95fe8341192766f9f7b1d698234e7000832b73b6986b149d",
"24a8dc0cb0fbeacaf3c2fce88c1b7b04c80a41a14c03f64a71ceade9db51f712",
"0626c726da6efb60434db31bb1dbf6e60d2e7754b8e53bd9bcd4f9085eef2231",
"2201163f42e5ae2cbaa6ca4b5935930d555dbc6a9552ad439a69f828cd533f68",
"b46a99f26f6d4290d02f6eeec52a7477072717543435cd571fdf11179d5f1b93",
"256bbd582d2d8c2d7c16564492cb9471276b57795abb58d674e7cd703f29f2f4",
"f4fa66cbbc970780ecf80a48b4eaa1afd80800f7ef95cf83956acacb5d717a03",
"dd3ed6bfc08020f54d0d569ce9e9c246a02e26343e81a6ea3bd3415d2863fd6d",
"21dd0d2f9ee890469f9bc91af57690edb0a5694f580c0ccbb2e6e57a64f93abe",
"6f199296848c0d0dbae6ff9802fda7c3e5da666aadf52452d97d08a923cd3ef2",
"9ae46fd2ae6688dd1da7b97e4e821affa0febc04f4d23f922134210998e0571e",
"27b2fe7b394fc16bc00d6644306a46cb0536eafb459f1db86845b7b00638df0e",
"8828ecc32c987f223b8a63e3e3bbebf9ef3394511b4fd3d0ccfa61befcc1b343",
"b10795055c234da092b1660ff2d0e6ddcf44e0d9be33272728bf1e0c5c24ad1c",
"40da9622e65568ebd55046c8fe0b1759e888a349ffbe79875c7cd218006e1084",
"e9f369ef15ca48d6623ea8bd0a4cd673b1f6a599edf24bbb8ad08ee129b55dea",
"17ba9478a057c634d2f8ca5bbc7d2dfa651b9308f128723d2a632a39bcd4b900",
"ae82fa352eca43bb1f8cff098014e0e1ba9f54bed88d5befe92e0913242f3cf4",
"18d0bd1664c0843be054473ab7a9895986fee484dcaadf452a4bc8b28fc19425",
"85f59e966f6df5f9a916909936d11f4787c5ece7df32f206094831071567da96",
"3b2a69e0416f9cf3be51703dcf83e01a5b749672e102819cadbb9460e48cf9da",
"ea0935edafc112bec56e5e92d7dc6115af1e9c2e309c3dc81a677cc8cbaa93e4",
"540f9eb5a30f27b0972d9a75a483e0d95a358e35d9a29b8298db9677fd8a26cb",
"48c1565930dee7bc90315000204e9fdbb4a454e63b97b4a1d640a371698109a9",
"09d9dd2b34b6d459d99fc1831a65f54bb05bfdb7079cc3209fb81ac311f201f6",
"e1472ffb38a9b895c3d845119026e0a4f899f9139bf1522e9019563dddf66650",
"6ffa14f75f04e1a0d533be500a12e63296b12b3d642cdf898aa3f46d3d472684",
"100ca825d8031f3d965b4cc9187f81a32403972e6f41727c7a4923d5859350fb",
"237a95da1df88825a5184e5ef17478846b67e6f698dfc347ea116db71a13cc83",
"6c43c41865abdb8be448edf3c57494f6b35b3c8266819d1d3a523be0718fab5a",
"ae37c040f2630bde1ece12d22a03e705bb44b173dd7225f6da01fb1803bb6463",
"2c939dacd500c4bfbadc5c3eaa4f885d90539c04c3904908d6caba5a46b92cad",
"5068fdd1aa86d8de1a2c3a067273ad3b59149a4c71486edf064cfa73fb772a08",
"bfb401fca7fee9b0d22d32bc0f1e23311bf1aa4c169a89111fc0b1dfb1d19429",
"8f346758ee41c40b3cb2529f4f42c00612ebe1349aa657763f24dec9ee2a2c2e",
"3da6b8f635db2c3081c18ed465eb3f96efe98c964675422d455581f723bd81f4",
"e4d77ba5edf199414c4f93bbb50416f98118db7254e0c902da35c78779effb56",
"debc916a912481cb7945c3b10b8d9e39f1e51e67d901970e26a923a9fdad5d37",
"f0da13b6d9317e2f0e2426cd7938de8f0a9d27a76f1caeb5ea202ee8150ee231",
"1dd1e67c67cd8c201d3fdf02cf1cd94b86f7c3158d5095e4c82e6c65b40ed6e5",
"1b21dab935b232c6bd9ab7c777b039f0cf2d02173e1c59c8028bc7e4bde6f4d5",
"5c91c32161c7695885bdc67d052094540733ac3ea74142f685b7e787f65a3674",
"89c2c878bf7a9710b615a5e0d805c7d40422971bb94c9b7a4722011d2bc64b42",
"4d91179fb6b4191cfef9880b5d8b4adf46febb426c0f9012067a9e7494c9f2cb",
"f35cdfe0e09f1ebe81f578f90e884929813892b1f2ff9edb9a793df63320275e",
"0cb818dc5f8e3503f384d750bac810a15115d041e178e9443673efddb4d33530",
"b03f409f791e2287bcabbc0991748776ebdb014fc47f2aef3d3c886e2996b96c",
"1ef8de12f718ed524850df9fde5bb566c86f71feb0ee861706fb1f80f001989f",
"40d8f4e39a9e57330986299f99aa14f382a01fe1c03da133ade0fcadff787288",
"54d2bc5dcdb86cdf62ea744484e47b574594171b13440427bea559e51d1f6cc9",
"bd6beaa83315ee770fed18529fabfd1c068b364c95a0d3b0e8e1581d19054780",
"d1cd477aa4f0596526aa56cff1a5febf34ed1c6284132b2730a44c94c89b4cda",
"05edd860aa5b7a2ac369657ffa01736254ac9f1dbe1bb8ea3c8c297393cbc855",
"1b1e277d2c8cb606d4124d3d65b2c592a15bf348a29c4e9a0baffa1eaeb2bf6f",
"f6d66980288d49f526c2f45fdfa85dfb4a5ee92a01c8b1ce11279c9a2a2bafcd",
"f4a7bbea49d9c6f5860e3efe35033c014419e5817aace3333faf529928b7a2ac",
"ee358bb76744e614bb6de369b9841284acf5071b043a13ea2ac7ffd359db4dd9",
"53ea0eddb860435954e96d537a1ab1a3f0101e986a0ae5f208be637c57951eb0",
"7b442f420448be491a4c4f10a6787aeda1854fcb376ba83f92d5b00b61cc70ca",
"101fdd01c352ddae5f01f2185cf37b5253a8126607f5013ef1af7942004788c3",
"204f70919460de9677d8c4e262d165390dd84ee61bc4a64c1724e36381aa10ee",
"040cf0855e2963e2edcb44055d2b80dcd9455354cca8a159f5729bba9541e668",
"24ba064c7ce3e01b1f4bfebba84385f5b55f337fb64ea7b59768312cf4104064",
"5acc38319c05b172034984e92a5e86693fac5c3197ae7b13830bdd9d181a4d32",
"f7e248533ec9d8b7436eab8eaac68fb30330a3fed82682d1ad8efd5674eb4155",
"7f8ea2d823570652b276cc5a195a1582b59dabf810390b980db5e23ca10fbdd8",
"4a611c59cda52c545b6cb3b84028ecee7326d188f55794a84423eb02d7289521",
"35a6e036a7bb35d360b1a9356b8f24b62cb31ffbba952444ce7f7a91a8ed8143",
"e39bdb5078641724c4251a95b8a77968e3f9b08449702257e95969790e252c0c",
"c72165307915143d4243584d9e858cd203c1490d60f27d40f639cecd7147e64d",
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
    subaccounts = accounts[0:args.num_producers]
    for i in subaccounts:
        for j in subaccounts:
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
    funds = 500000000 / args.num_producers / 2
    for i in range(0, args.num_producers):
        retry(args.clultrain + 'system newaccount --transfer ultrainio %s %s --stake-net "%.4f SYS" --stake-cpu "%.4f SYS" --buy-ram "1000.000 SYS" ' % (accounts[i], args.public_key, funds, funds))
        retry(args.clultrain + 'transfer ultrainio %s "5000.0000 SYS"' % (accounts[i]))
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
    ('X', 'xfer',           stepTransfer,               False,   "Random transfer tokens (infinite loop)"),
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
