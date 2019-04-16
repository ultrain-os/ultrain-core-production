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

logFile = None

unlockTimeout = 999999999
defaultclu = '%s/ultrain-core/build/programs/clultrain/clultrain --wallet-url http://127.0.0.1:6666 '
defaultkul = '%s/ultrain-core/build/programs/kultraind/kultraind'
defaultcontracts_dir = '%s/ultrain-core/build/contracts/'

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
    while time <=20:
        print('bios-subchain.py:', args)
        logFile.write(args + '\n')
        if subprocess.call(args, shell=True):
            print('*** Retry')
            time=time+1;
            sleep(1)
        else:
            break


# find index
def findIndex(userTag) :
    index=0;
    for a in accounts :
        print a;
        if a == userTag :
            break;
        index = index+1
    return index

# transfer producer
# sample : python transfer_reg.py -T -s 11 -d 12 -u user.11.111
def transferReg():
    print "transfer reg start:"
    user = args.user
    source = args.source;
    dest = args.dest;
    userArray=user.split('.')
    userTag=userArray[len(userArray)-1]
    #print userTag
    index = findIndex("."+userTag)
    userPK=account_pk_list[index]
    minerPK=pk_list[index]
    print "transfer producer("+user+" accountPK: "+userPK+") from chain("+source+") to chain("+dest+")"
    sleep(1)
    print "empoweruser(user:"+user+" to chain:"+dest+")"
    #retry(args.clultrain+'system empoweruser '+user+' '+dest+' '+userPK+' '+userPK+' 1  -p '+user+'@active');
    sleep(5)
    bls_key = bls_pk_list[index];
    print "reg producer:" + user + "(" + userPK + " "+bls_key+ ") belongs to chain(" + dest + ")"
    #print(args.clultrain + 'system regproducer ' + user +' '+userPK+' '+bls_key+' https://'+user+'.com '+dest+' '+user+' -p ultrainio@active')
    retry(args.clultrain + 'system regproducer ' + user +' '+minerPK+' '+bls_key+' '+user+' https://'+user+'.com '+dest+' -p ultrainio@active -u')

# set schedule
def setSchedule():
    print "set chain schedule"

    if args.schedPeriod is None or args.confirmPeriod is None :
        print "schedPeriod or confirmPeriod is none.."
        return
    retry(args.clultrain + 'push action ultrainio setsched \'{"is_enabled": "1", "sched_period": "'+args.schedPeriod+'", "expire_time": "'+args.confirmPeriod+'"}\' -p ultrainio@active')


def closeSchedule():
    print "close schedule"
    retry(args.clultrain + 'push action ultrainio setsched \'{"is_enabled": "0", "sched_period": "600", "expire_time": "5"}\' -p ultrainio@active')

def serachTable(contract,scope,table):
    return "get table "+contract+" "+scope+" "+table

def showSubchains():
    retry(args.clultrain+serachTable("ultrainio","ultrainio","subchains"));






# Command Line Arguments
commands = [
    ('T', 'transfer', transferReg, False, "transfer a producer to another chain"),
    ('S', 'set', setSchedule, False, "set side chain schedule info"),
    ('C', 'close', closeSchedule, False, "close auto schdule"),
    ('SHOW', 'show', showSubchains, False, "show subchains info")
];
parser = argparse.ArgumentParser()
parser.add_argument('-s', '--source', type=str, help="subchain name form")
parser.add_argument('-d', '--dest', type=str, help="subchain name to", )
parser.add_argument('-u', '--user', help='producer name')
parser.add_argument('-p', '--programpath', metavar='', help="set programpath params")
parser.add_argument('-sp', '--schedPeriod', type=str, help="set schedule period(minute)")
parser.add_argument('-cp', '--confirmPeriod', type=str, help="set confirm period(minute)")
parser.add_argument('--clultrain', metavar='', help="Clultrain command", default=defaultclu % '/root/workspace')
parser.add_argument('--kultraind', metavar='', help="Path to kultraind binary", default=defaultkul % '/root/workspace')
parser.add_argument('--contracts-dir', metavar='', help="Path to contracts directory",
                    default=defaultcontracts_dir % '/root/workspace')
parser.add_argument('--log-path', metavar='', help="Path to log file", default='./output.log')

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
    print('transfer_reg.py: Tell me what to do. -a does almost everything. -h shows options.')
