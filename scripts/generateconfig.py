#!/usr/local/bin/python2

import os
import datetime
import time
import random
import copy

class Host(object):

	def __init__(self, ip, constainers):
		self.ip = ip
		self.containers = constainers

class Container(object):

	def __init__(self, id, name, ip, hostip):
		self.id = id
		self.name = name
		self.ip = ip
		self.hostip = hostip
		self.peers = []
		self.isleaf = False

class Node(object):

	def __init__(self, ip):
		self.id=ip

# global parameters
hosts={}
allIDs=[]
allNames=[]
allIPs=[]
allAccounts=[]
allPriKeys = []
allPubKeys =[]

pk_list = [
    "9bb728c7e29e2f7a0c34c954382ea3ad8e4a93bf3455f9438afa086b3d67304b3b2a7dd684f0cda7d71ca9116128b1b7e49f31b2b10c0174b21b98c82161294da6d88bdbc92a897006e547c5ee5a3277f74fb4f9c0b9d8339fc3de8e6f467aac558859cfd0b9d3eea828e29b33b4f819f14ffbbcc7bea2ac585228fa8f12c42f",
    "149474beeef356cf558385aa40cc495312d40d821a1828a4bef44b542a91c60e8ad7bd88fe6293192c0f354b074d63413d95f157844e060df3b14224bedff2f2a2d901b96101b1840ecff5f4c63a67cc2acbae6943b395b0229d13fe1ffbb219c435e1fc978143fb03a5b46fa3c4c5f67b9fd9089b06b4c25cf15a699c2a1f7c",
    "8c1901520bee618f50db3c2a0ce0645e470b1551ee4a3beabd9ead29e301312915849b579c6edf3fc06eb309ab61e3a9257ec4161b181deacb019217ffb2972da72d967e0873f67f1c4434e5b11a3f96676b89f128b098f74b028226b49ba8c7dcce635ccaeb723b7e0d5e187024341f6227e6291570274b71982a9caab22f2c",
    "397b3b7c0e40f78df06deb35a5bfd9a8b39cd0e0c0c75a2dc4775c935b0dbbdb92590bd92965e309c013e642e13ca8619fc3a435d7f7322bcd8a6eb63793e4162d961b100681e45ac30767f76f50926331886d8a29ae7c0bcdbd1d6834c12783856c0b2a915536fb3d33213070869d2bff00a810a69f680e3c6071a4d0b9744d",
    "011fcc9c00fea448b506a2a770fe5ec4b3be78967bf05cc78cb94fdcf929862563ee0b01cd66d131f6d3a5d509b06fb95975e49f0539b401d8dff4a9876660ee22f387e02342c404112768e9fed8042591471baf4d657fc938df61b28711b7205af8ac9d44a9397306c724417d9c71956308140f3aea76a865048fa6e86b79b7",
    "2e9444984ea33e3ffd8fddc5b9f1bf512b18faf6b9a2f232eb33b36709baf5e92bb38c723ca333da2c571bdcbf81dffbdd07117dee4ebfda5251101c3fb1f559492576980c374546dd2776d1506466463f40338426a421872324b69ecd1cd849610607f76fc624840e805db991cda853047063733a181ddf07cb407f71525888",
    "4875ba243fcb67515719e1b6ee31655bed7319216c20f9b99a5111a4c8b03bf662c5040cc02fd9bd50836cec3331bdb7c1009aa14ac765b8d3111ebe3583674031bb2ea591059d36c1603a2fb204d494d871d9b972e5a1b4e8d2c31ee2db01a0dbf1932ccdd568fb752f1b0b9ad859f77f374f834246dc4e8755315b3d32b941",
    "878f302b3489b142a15e690c4f5d90093aa07e833836ee813209451ffa901a2efe3e527fe0bd62b37cc6583595c6813ea528ad0bc724cc8e92779e9674534623a696f849e01b93d5d1b0e10e3f223bc47dddaedd71cbaf7671cf7cedc1f9d923cec0eeb5bbb9b9d487ee0aa28c4ae2f1750aab4d48c5f6a8394df18a0c285129"
]

sk_list = [
    "0cae125e5d21e20fb37869c7e46209fe83ad0e2c",
    "78c5b674977281c5e61b3750b726955295762acf",
    "4e92f58ed3a6739d34a658ad0afbf4c00547577f",
    "5da5447ae2bb31c5fcd0945e76573753e00ac329",
    "35090a34fc60933a334194d0f97e05961058c112",
    "7b470519fde7fe6529b20787721c0da38b3b113a",
    "2adecb04e6a724d94eadbd70ae34c5ada5ef243a",
    "1566abe742d2f91514ed2cf0c92332a04ab2f30a"
]

accounts = [
"genesis",
"user.111",
"user.112",
"user.113",
"user.114",
"user.115",
"user.121",
"user.122",
]
dockerinfo = "dockerinfo"
def select_sort(lists):
    count = len(lists)
    for i in range(0, count):
        min = i
        for j in range(i + 1, count):
            if int(lists[min].name[len(lists[min].name)-1::len(lists[min].name)]) > int(lists[j].name[len(lists[j].name)-1::len(lists[j].name)]):
                min = j
        lists[min], lists[i] = lists[i], lists[min]
    return lists
# load containers' parameters in hosts
def load_parameters():
    global hosts,allIDs,allNames,allIPs,allAccounts,allPriKeys,allPubKeys
    print(dockerinfo)
    with open('config/IPs/'+dockerinfo+'.txt') as f:
       elements = f.read().splitlines()
    IDs = elements[0::3]
    Names = elements[1::3]
    IPs = elements[2::3]

    containers = []
    for i in range(0,len(IDs)):
        container = Container(IDs[i],Names[i],IPs[i],dockerinfo)
        containers.append(container)
    hosts[dockerinfo] = select_sort(containers)

    allIDs = allIDs + IDs
    allNames = allNames + Names
    allIPs = allIPs + IPs
    allAccounts = accounts
    allPriKeys = sk_list 
    allPubKeys = pk_list 

    print(allIDs)
    print(allNames)
    print(allIPs)

def write_config_file():
        insert_genesis_time()	
        hostip = "config"
        os.system("rm -rf config/" + hostip)
        os.system("mkdir config/" + hostip)
        index_key = 0
        for con in hosts[dockerinfo]:
            fname = "config/%s/%s.ini" % (hostip, con.id)
            print(fname)
            if not os.path.isfile(fname):  # file does not exist
                        cmd = "cp template.txt " + fname
                        os.system(cmd)
            insert_keys(fname, index_key)
            if con.name[len(con.name)-1::len(con.name)] == '7':
                insert_non_producing(fname)
            print(hostip,con.ip,con.id)
            insert_peer(fname,hosts[dockerinfo][0].ip)
            index_key+=1
        os.system("rm -f  template.txt")

def readfile(fname):
	fileold = open(fname, "r")
	content = fileold.readlines()
	fileold.close()
	return content

def writefile(fname,content):
	filenew = open(fname, "w")
	filenew.writelines(content)
	filenew.close()

def insert_genesis_time():
	fname_orig = "template_orig.txt"
	fname = "template.txt"
	cmd = "cp %s %s" % (fname_orig, fname)
	os.system(cmd)
	content = readfile(fname)
	after_time = (datetime.datetime.now()+datetime.timedelta(minutes=2)).strftime("%Y-%m-%d %H:%M") + ":00"
	newcontent = "genesis-time = %s\n" % after_time
	index_line = content.index("#insert_genesis-time\n")
	content.insert(index_line+1,newcontent)
	writefile(fname,content)

def insert_leader_sk(fname):
	content = readfile(fname)
	newcontent = "my-account-as-committee = %s\nmy-sk-as-committee = %s\n" % ("genesis", \
				"9bb728c7e29e2f7a0c34c954382ea3ad8e4a93bf3455f9438afa086b3d67304b3b2a7dd684f0cda7d71ca9116128b1b7e49f31b2b10c0174b21b98c82161294da6d88bdbc92a897006e547c5ee5a3277f74fb4f9c0b9d8339fc3de8e6f467aac558859cfd0b9d3eea828e29b33b4f819f14ffbbcc7bea2ac585228fa8f12c42f\n")
	index_line = content.index("#insert_my_keys\n")
	content.insert(index_line+1, newcontent)
	writefile(fname,content)

def insert_non_producing(fname):
	content = readfile(fname)
	newcontent = "is-non-producing-node = 1\nhttp-server-address = 0.0.0.0:8888\nhttp-alias = 172.16.10.4:8877\n"
	print(newcontent)
	index_line = content.index("#insert_if_producing-node\n")
	content.insert(index_line+1, newcontent)
	writefile(fname,content)

def insert_peer(fname,peer):
	content = readfile(fname)
        newcontent = "p2p-peer-address = %s:20122\nrpos-p2p-peer-address = %s:20123\n" % (peer,peer)
	print(newcontent)
	index_line = content.index("#insert_peers\n")
	content.insert(index_line+1, newcontent)
	writefile(fname,content)

def insert_keys(fname,index_key):
	content = readfile(fname)
	newcontent = "my-account-as-committee = %s\nmy-sk-as-committee = %s\n" % \
				 (allAccounts[index_key],allPriKeys[index_key])
	index_line = content.index("#insert_my_keys\n")
	content.insert(index_line+1, newcontent)
	writefile(fname, content)

load_parameters()
write_config_file()

