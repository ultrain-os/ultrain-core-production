#!/usr/bin/python
# -*- coding: UTF-8 -*-
from email.header import Header
from email.mime.text import MIMEText
import smtplib
import argparse
import json
import os
import random
import re
import subprocess
import sys
import time
import string
import requests
import datetime
from ConfigParser import ConfigParser
args = None
logFile = None

defaultclu = '%s/ultrain-core/build/programs/clultrain/clultrain --wallet-url http://127.0.0.1:6666 '
schduleaccounts=[]

conf = ConfigParser()
curindex = 0
if os.path.isfile("/root/workspace/ultrain-core/scripts/mutichain.cfg") :
    conf.read("/root/workspace/ultrain-core/scripts/mutichain.cfg")
    curindex = conf.getint('config', 'index')
basis_value = curindex*100000
maxnum=5
prefixStr="test.a."
output=['1']*maxnum
print "basis_value："
print basis_value

def out() :
    str = prefixStr
    for i in range(0, maxnum):
        str = '%s%s'%(str,output[maxnum-i-1])
    #print(str)
    return str;

def addAcc():
    for j in range(0,maxnum):
        if output[j] == '5':
            output[j] = 'a'
        else :
            output[j]=chr(ord(output[j])+1);

        if output[j] == chr(ord('z')+1):
            output[j]='1'
        else:
            break

def generateAccounts(startNum,endNum) :
    print "startNum:";
    print startNum;
    print " endNum:";
    print endNum;
    outNum=["1"]*(endNum-startNum);
    t=0;
    for i in range(0, endNum):
        #print i;
        str = out();
        if i >= startNum :
            #print "i >= startNum";
            #print  str;
            outNum[t]=str;
            t=t+1;
        addAcc();
    return outNum;

schduleaccounts = generateAccounts(basis_value, basis_value + 100000);
sidechainacc = schduleaccounts[0:100000]


#print  schduleaccounts;



def run(args):
    print('bios-boot-tutorial.py:', args)
    logFile.write(args + '\n')
    if subprocess.call(args, shell=True):
        print('bios-boot-tutorial.py: exiting because of error')
        sys.exit(1)
def retry(args):
    retyrnum = 0
    while True:
        print('bios-boot-tutorial.py:', args)
        logFile.write(args + '\n')
        if subprocess.call(args, shell=True):
            print('*** Retry')
            retyrnum += 1
            if retyrnum > 30:
                break
        else:
            break

def sleep(t):
    print('sleep', t, '...')
    time.sleep(t)
    print('resume')


def createmutiaccounts():
    count =0;
    for a in sidechainacc:
        retry(args.clultrain + 'create account -u ultrainio ' + a + ' ' + args.initacc_pk)
        count = count +1;
        if count % 10000 == 0 :
            sleep(10);
    configindex = curindex + 1
    if os.path.isfile("/root/workspace/ultrain-core/scripts/mutichain.cfg") :
        conf.set('config', 'index', str(configindex))
    else:
        conf.add_section("config")
        conf.set('config', 'index', str(configindex))
    with open('/root/workspace/ultrain-core/scripts/mutichain.cfg', 'w') as fw:
        conf.write(fw)



def deleteHistory():
    if os.path.isfile("/root/workspace/ultrain-core/scripts/mutichain.cfg") :
        os.remove("/root/workspace/ultrain-core/scripts/mutichain.cfg");


def verifyaccrestest():
    successFlag=True;
    chain1nofindacc = []
    chain2nofindacc = []
    chain3nofindacc = []
    chain1nofindres = []
    chain2nofindres = []
    chain3nofindres = []

    url1 = sub1HttpUrl;
    if local == True :
        url1 = sub1HttpUrlLocal;

    url2 = sub2HttpUrl;
    if local == True:
        url2 = sub2HttpUrlLocal;

    url3 = sub3HttpUrl;
    if local == True:
        url3 = sub3HttpUrlLocal;

    if args.subchainNum >=1:
        print "check chain1 num"
        getaccresinfo(url1,sidechainacc1,chain1nofindacc,4,20,chain1nofindres)
        print("chain1 not find account number:",int(len(chain1nofindacc))," notfind resource number:",int(len(chain1nofindres)))
        chaininfostr = getchaincontext("11",sidechainacc1,chain1nofindacc,chain1nofindres)
        if (int(len(chain1nofindacc)) > 0 or int(len(chain1nofindres)) > 0) :
            successFlag = False;
    if args.subchainNum >=2:
        print "check chain2 num"
        getaccresinfo(url2,sidechainacc2,chain2nofindacc,2,30,chain2nofindres)
        print("chain2 not find account number:",int(len(chain2nofindacc))," notfind resource number:",int(len(chain2nofindres)))
        chaininfostr += getchaincontext("12",sidechainacc2,chain2nofindacc,chain2nofindres)
        if (int(len(chain2nofindacc)) > 0 or int(len(chain2nofindres)) > 0) :
            successFlag = False;
    if args.subchainNum >=3:
        print "check chain3 num"
        getaccresinfo(url3,sidechainacc3,chain3nofindacc,4,25,chain3nofindres)
        print("chain3 not find account number:",int(len(chain3nofindacc))," notfind resource number:",int(len(chain3nofindres)))
        chaininfostr += getchaincontext("13",sidechainacc3,chain3nofindacc,chain3nofindres)
        if (int(len(chain3nofindacc)) > 0 or int(len(chain3nofindres)) > 0) :
            successFlag = False;
    (verifysucblock,verifyfailblock) = merkleproof()
    totalverifyblock = verifysucblock + verifyfailblock
    merklestr = "merkle proof \n验证主链块数量为" + str(len(totalverifyblock))
    merklestr += "\n块高列表为:"
    for a in totalverifyblock:
        merklestr += str(a)+","
    merklestr += "\nmerkle proof 验证失败块数量为:" + str(len(verifyfailblock))
    if len(verifyfailblock) != 0:
        successFlag = False
        merklestr += "\n失败块列表为:"
        for a in verifyfailblock:
            merklestr += str(a)+","
    chaininfostr += merklestr
    configindex = curindex + 1
    confblockheight = curblockheight
    if os.path.isfile("/root/workspace/ultrain-core/scripts/mutichain.cfg") :
        conf.set('config', 'index', str(configindex))
        conf.set('config', 'blockheight', str(confblockheight))
    else:
        conf.add_section("config")
        conf.set('config', 'index', str(configindex))
        conf.set('config', 'blockheight', str(confblockheight))
    with open('/root/workspace/ultrain-core/scripts/mutichain.cfg', 'w') as fw:
        conf.write(fw)



    print(chaininfostr)
    sendEmail(chaininfostr,successFlag)

# Command Line Arguments

parser = argparse.ArgumentParser()

commands = [
    ('c', 'createacc',      createmutiaccounts,       False,    "createmutiaccounts"),
    ('d', 'delete',   deleteHistory,      False,    "delete cfg file"),
]

parser.add_argument('--initacc-pk', metavar='', help="ULTRAIN Public Key", default='UTR6XRzZpgATJaTtyeSKqGhZ6rH9yYn69f5fkLpjVx6y2mEv5iQTn', dest="initacc_pk")
parser.add_argument('--initacc-sk', metavar='', help="ULTRAIN Private Key", default='5KZ7mnSHiKN8VaJF7aYf3ymCRKyfr4NiTiqKC5KLxkyM56KdQEP', dest="initacc_sk")
parser.add_argument('--clultrain', metavar='', help="Clultrain command", default=defaultclu % '/root/workspace')

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
print(args.clultrain)

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
