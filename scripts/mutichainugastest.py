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
curblockheight = 0
if os.path.isfile("/root/workspace/ultrain-core/scripts/mutichain.cfg") :
    conf.read("/root/workspace/ultrain-core/scripts/mutichain.cfg")
    curindex = conf.getint('config', 'index')
    curblockheight = conf.getint('config', 'blockheight')
basis_value = curindex*150
maxnum=4
prefixStr="token.a."
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

schduleaccounts = generateAccounts(basis_value, basis_value + 150);
sidechainacc = schduleaccounts[0:150]
sidechainacc1 = schduleaccounts[0:50]
sidechainacc2 = schduleaccounts[50:100]
sidechainacc3 = schduleaccounts[100:150]


print  schduleaccounts;



local= True;
sub1HttpUrlLocal = "172.16.10.5:8888"
sub2HttpUrlLocal = "172.16.10.5:8899"
sub3HttpUrlLocal = "172.16.10.5:8888"
sub1HttpUrl = "172.31.7.20:8888"
sub2HttpUrl = "172.31.12.250:8888"
sub3HttpUrl = "172.31.8.22:8888"

def getSubchainUrlPrefix(chainIndex):
    if (chainIndex == 1) :
        if local == True :
            return sub1HttpUrlLocal;
        else :
            return  sub1HttpUrl;

    if (chainIndex == 2) :
        if local == True :
            return sub2HttpUrlLocal;
        else :
            return  sub2HttpUrl;

    if (chainIndex == 3) :
        if local == True :
            return sub3HttpUrlLocal;
        else :
            return  sub3HttpUrl;



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
            if retyrnum > 500:
                break
        else:
            break

def sleep(t):
    print('sleep', t, '...')
    time.sleep(t)
    print('resume')

def sendEmail(msg,successFlag):
    """
    邮件通知
    :param str: email content
    :return:
    """
    receiver = "sidechain@ultrain.io"
    if local == True :
        receiver = "chenxiaojian@ultrain.io" ;

    try:
        sender = "739884701@qq.com"
        subject = '今日测试账户转账ugas同步'
        if local == True:
            subject = '今日测试账户转账ugas同步(docker环境)'
        status = "--失败"
        if successFlag == True :
            status = "--成功"
        subject = subject+status;
        username = "739884701"
        password = "cfiawwwcqltbbcic"
        host = "smtp.qq.com"
        s = "{0}".format(msg)

        msg = MIMEText(s, 'plain', 'utf-8')  # 中文需参数‘utf-8’，单字节字符不需要
        msg['Subject'] = Header(subject, 'utf-8')
        msg['From'] = sender
        msg['To'] = receiver

        smtp = smtplib.SMTP_SSL()
        smtp.connect(host)
        smtp.login(username, password)
        smtp.sendmail(sender, receiver.split(","), msg.as_string())
        smtp.quit()
        print("邮件已通知, 请查收")
    except Exception as e:
        print("邮件配置有误", e)

def createmutiaccounts():
    for a in sidechainacc:
        if args.deleysec == 0:
            retry(args.clultrain + 'create account -u ultrainio ' + a + ' ' + args.initacc_pk)
        else:
            retry(args.clultrain + 'create account -u ultrainio ' + a + ' ' + args.initacc_pk+" --delay-sec "+str(args.deleysec))
    sleep(10)
    print "sidechain";
    print sidechainacc1;
    if args.subchainNum >= 1:
        for a in sidechainacc1:
            if args.deleysec == 0:
                retry(args.clultrain + ' system empoweruser %s %s %s %s 1 -p %s@active' %(a,"11",args.initacc_pk,args.initacc_pk,a))
            else:
                retry(args.clultrain + ' system empoweruser %s %s %s %s 1 --delay-sec %s -p %s@active' %(a,"11",args.initacc_pk,args.initacc_pk,str(args.deleysec),"ultrainio"))
    if args.subchainNum >= 2:
        for a in sidechainacc2:
            if args.deleysec == 0:
                retry(args.clultrain + ' system empoweruser %s %s %s %s 1 -p %s@active' %(a,"12",args.initacc_pk,args.initacc_pk,a))
            else:
                retry(args.clultrain + ' system empoweruser %s %s %s %s 1 --delay-sec %s -p %s@active' %(a,"12",args.initacc_pk,args.initacc_pk,str(args.deleysec),"ultrainio"))
    if args.subchainNum >= 3:
        for a in sidechainacc3:
            if args.deleysec == 0:
                retry(args.clultrain + ' system empoweruser %s %s %s %s 1 -p %s@active' %(a,"13",args.initacc_pk,args.initacc_pk,a))
            else:
                retry(args.clultrain + ' system empoweruser %s %s %s %s 1 --delay-sec %s -p %s@active' %(a,"13",args.initacc_pk,args.initacc_pk,str(args.deleysec),"ultrainio"))


def initTransfer():
    for a in sidechainacc:
        retry(args.clultrain + 'transfer ultrainio %s "100 UGAS" "ultrainio"' %(a))

def transerMasterToSubchain():
    if args.subchainNum >= 1:
        for a in sidechainacc1:
            retry(args.clultrain + 'transfer %s utrio.bank "10 UGAS" "11"' %(a));
    if args.subchainNum >= 2:
        for a in sidechainacc2:
            retry(args.clultrain + 'transfer %s utrio.bank "10 UGAS" "12"' %(a));
    if args.subchainNum >= 3:
        for a in sidechainacc3:
            retry(args.clultrain + 'transfer %s utrio.bank "10 UGAS" "13"' %(a));

def transerSubToMaster():
    if args.subchainNum == 1:
        for a in sidechainacc1:
            retry(args.clultrain + 'transfer %s utrio.bank "10 UGAS" "ultrainio"' %(a));
    if args.subchainNum == 2:
        for a in sidechainacc2:
            retry(args.clultrain + 'transfer %s utrio.bank "10 UGAS" "ultrainio"' %(a));
    if args.subchainNum == 3:
        for a in sidechainacc3:
            retry(args.clultrain + 'transfer %s utrio.bank "10 UGAS" "ultrainio"' %(a));





def checkUgasInfo(ip,acclist,compareNum,tolerantNum,errorArray):
    index = 1;
    for a in acclist:
        print "index:"+str(index)
        print "check:"+a;
        j = json.loads(requests.get("http://"+ip+"/v1/chain/get_account_info",data = json.dumps({"account_name":a})).text)
        #balance = j.core_liquid_balance;
        balanceStr = j["core_liquid_balance"];
        array = balanceStr.split(' ', 1 )
        balance = float(array[0])
        #print balance;
        if balance > compareNum-tolerantNum and balance < compareNum+tolerantNum :
            print a+": ugas:"+str(balance)+"->right";
            #errorArray.append(a+" num:"+str(compareNum)+",real:"+str(balance));
        else:
            errorArray.append(a+" num:"+str(compareNum)+",real:"+str(balance));
            print a+": ugas:"+str(balance)+"->error";
        index = index+1;



def verifyUgas():
    chaininfostr = "";
    successFlag = True;
    errorSub1Array=[]
    errorSub2Array=[]
    errorSub3Array=[]
    if args.subchainNum >= 1:
        checkUgasInfo("127.0.0.1:8888",sidechainacc1,90,1,errorSub1Array);
        checkUgasInfo(getSubchainUrlPrefix(1),sidechainacc1,10,1,errorSub1Array);
        chaininfostr+="子链1 未同步正确的ugas的数量："+ str(len(errorSub1Array))+"\n";
        if len(errorSub1Array) > 0 :
            successFlag = False;
    if args.subchainNum >= 2:
        checkUgasInfo("127.0.0.1:8888",sidechainacc2,90,1,errorSub2Array);
        checkUgasInfo(getSubchainUrlPrefix(2),sidechainacc2,10,1,errorSub2Array);
        chaininfostr+="子链2 未同步正确的ugas的数量："+ str(len(errorSub2Array))+"\n";
        if len(errorSub2Array) > 0 :
            successFlag = False;
    if args.subchainNum >= 3:
        checkUgasInfo("127.0.0.1:8888",sidechainacc3,90,1,errorSub3Array);
        checkUgasInfo(getSubchainUrlPrefix(3),sidechainacc3,10,1,errorSub3Array);
        chaininfostr+="子链3 未同步正确的ugas的数量："+ str(len(errorSub3Array))+"\n";
        if len(errorSub3Array) > 0 :
            successFlag = False;

    sendEmail(chaininfostr,successFlag)

# Command Line Arguments

parser = argparse.ArgumentParser()

commands = [
    ('c', 'createacc',      createmutiaccounts,       False,    "createmutiaccounts"),
    ('if', 'initTransfer',      initTransfer,       False,    "init transfer"),
    ('tms', 'transferMasToSub',      transerMasterToSubchain,       False,    "transfer ugas from master to subchain"),
    ('tsm', 'transferSubToMaster',      transerSubToMaster,       False,    "transfer ugas from subchain to master"),
    ('verify', 'verify',      verifyUgas,       False,    "verify ugas"),
]

parser.add_argument('--initacc-pk', metavar='', help="ULTRAIN Public Key", default='UTR6XRzZpgATJaTtyeSKqGhZ6rH9yYn69f5fkLpjVx6y2mEv5iQTn', dest="initacc_pk")
parser.add_argument('--initacc-sk', metavar='', help="ULTRAIN Private Key", default='5KZ7mnSHiKN8VaJF7aYf3ymCRKyfr4NiTiqKC5KLxkyM56KdQEP', dest="initacc_sk")
parser.add_argument('--clultrain', metavar='', help="Clultrain command", default=defaultclu % '/root/workspace')
parser.add_argument('-sn', '--subchainNum', type=int, default=3)
parser.add_argument('-si', '--subchainIndex', type=int, default=1)
parser.add_argument('-ds', '--deleysec', type=int, default=0)

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
