#!/usr/bin/env python
from account_info import *

def generate_keypair():
    with open('keypair.txt') as f:
       elements = f.read().splitlines()
    allMinerPris = elements[0::6]
    allMinerPubs = elements[1::6]
    allPriKeys = elements[2::6]
    allPubKeys = elements[3::6]
    allAccounts = elements[4::6]  
    nullvalue = elements[5::6]
    # for i in range(0,len(allMinerPris)):
    #     print(allMinerPris[i])
    # for i in range(0,len(allMinerPubs)):
    #     print(allMinerPubs[i])
    # for i in range(0,len(allAccounts)):
    #     print(allAccounts[i])
    # for i in range(0,len(allPriKeys)):
    #     print(allPriKeys[i])
    # for i in range(0,len(allPubKeys)):
    #     print(allPubKeys[i])
    print("allMinerPris length:",len(allMinerPris))
    print("allMinerPubs length:",len(allMinerPubs))
    print("allPriKeys length:",len(allPriKeys))
    print("allPubKeys length:",len(allPubKeys))
    print("blsPriKeys length:",len(bls_sk_list))
    print("blsPubKeys length:",len(bls_pk_list))
    print("allAccounts length:",len(allAccounts))
    content = ""
    for i in range(0,len(allMinerPris)):
         content=content+  allMinerPris[i] +"\n"\
                        + allMinerPubs[i] +"\n"\
                        + allPriKeys[i] +"\n"\
                        + allPubKeys[i] +"\n"\
                        + "bls_sk:"+bls_sk_list[i+1] +"\n"\
                        + "bls_pk:"+bls_pk_list[i+1] +"\n"\
                        + allAccounts[i] +"\n"+"\n"
    
    writefile("newfile.txt",content)
def generate_newkeypair():
    content = ""
    for i in range(0,len(accounts)):
         content=content+ "pri:"+sk_list[i] +"\n"\
                        + "pub:"+pk_list[i] +"\n"\
                        + "ask:"+account_sk_list[i] +"\n"\
                        + "apk:"+account_pk_list[i] +"\n"\
                        + "bls_sk:"+bls_sk_list[i] +"\n"\
                        + "bls_pk:"+bls_pk_list[i] +"\n"\
                        + "account:"+accounts[i] +"\n"+"\n"\

    writefile("newfile.txt",content)
def readfile(fname):
	fileold = open(fname, "r")
	content = fileold.readlines()
	fileold.close()
	return content

def writefile(fname,content):
	filenew = open(fname, "w")
	filenew.writelines(content)
	filenew.close()

generate_newkeypair()

