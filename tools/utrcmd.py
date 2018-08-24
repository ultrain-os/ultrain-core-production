#! /usr/bin/python
import os
import time
import sys
buildpath=sys.argv[1]
clultrain=buildpath+"/programs/clultrain/clultrain"
contractPath=buildpath+"/contract/"
#Keys=($($clultrain create key | awk -F: '{print $2}'))
#PrivKey=${Keys[0]}
#PubKey=${Keys[1]}
PubKey="UTR6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV"
PrivKey="5KQwrPbwdL6PhXujxW37FSSQZ1JiwsST4cqQzDeyXtP79zkvFD3"
print("pub:"+PubKey)
print("pri:"+PrivKey)
### create default wallet
#WalletPwd=$($clultrain wallet create | tail -n 1 | sed 's/\"//g')
#if test -z $WalletPwd
#then
#   echo "Wallet password is empty, quit."
#   exit 0
#fi
# echo "Password for default, please save it safely : " $WalletPwd
# WalletPwd=PW5JDL5AjogThU1avkC3cxE7xThEmeeya2bmvrZqJxYCcGq2UWY6B
# $clultrain wallet unlock --password $WalletPwd 
oscmd=clultrain+" wallet import "+ PrivKey
os.system(oscmd)
time.sleep(1)
usrList=["user","tester","exchange","utrio.names","utrio.ram"," utrio.ramfee","utrio.bpay","utrio.msig","utrio.upay","utrio.saving","utrio.stake","utrio.token","utrio.vpay"]
for username in usrList:
    oscmd=clultrain+" create account ultrainio "+username+ " "+PubKey+" "+PubKey
    print (oscmd)
    os.system(oscmd)


oscmd=clultrain+" set contract utrio.token "+buildpath+"/contracts/ultrainio.token -p utrio.token "
os.system(oscmd)
time.sleep(1)
oscmd=clultrain+" push action utrio.token create \'[ \"ultrainio\", \"1000000000.0000 SYS\", 0, 0, 0]\' -p utrio.token"
os.system(oscmd)
time.sleep(1)
oscmd=clultrain+" push action utrio.token issue \'[ \"ultrainio\", \"300000000.0000 SYS\", \"memo\"]\' -p ultrainio"
os.system(oscmd)
time.sleep(1)
oscmd=clultrain+" set contract ultrainio "+buildpath+"/contracts/ultrainio.system -p ultrainio"
os.system(oscmd)
time.sleep(1)
oscmd=clultrain+" set contract ultrainio "+buildpath+"/contracts/ultrainio.system -p ultrainio "
os.system(oscmd)
