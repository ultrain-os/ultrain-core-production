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

pk_list = ["369c31f242bfc5093815511e4a4eda297f4b8772a7ff98f7806ce7a80ffffb35",
"b7e0a16fdca44d4ece1b14d8e7e6207402a6447115ca7d2d7edb08958e6d8ed5",
"4031a95071a092eca8646d3999c438bdfde368d4837770755af65a11b4520a48",
"2b43a3d8e0523a85141bbfca41006cf9abc47587d0ff3c46d257551ec05f1677",
"b9a55c3c661abd8c539b7a7c05c8176036f87aeeb6b117a138327cfdb374cc23",
"8f6e3b3336276138023617f0ae0e6fd0c37d27aa9995f9803fe78df4941dd3ec",
"3696fdfb2a9716ae4b1bca0d5b0a861a1e5d64562994aeb515eed49290c9f1c2",
"578451935c370d9c7fbcdd77e35a40e49bf0a5311e065035778ac27e6263b10d",
 ]

sk_list = ["5079f570cde7801c70a19fb2c7e292d09923218f2684c8a1121c2da7a02a5dc3369c31f242bfc5093815511e4a4eda297f4b8772a7ff98f7806ce7a80ffffb35",
           "97d157bf7fdedfeaef46216e246cc83f9c25574b49bacf83a1862ef0d82233ecb7e0a16fdca44d4ece1b14d8e7e6207402a6447115ca7d2d7edb08958e6d8ed5",
           "b5b3b423eed6fd6f255fe1e4570e3a9f9878a36870b584b1d231ca778ad95ede4031a95071a092eca8646d3999c438bdfde368d4837770755af65a11b4520a48",
           "5cfdb237515f969620f857658baac5485266f1c143fee80208e0da9935bbfaae2b43a3d8e0523a85141bbfca41006cf9abc47587d0ff3c46d257551ec05f1677",
           "6b8ed821703f369f1a6b7c767bf076b093a17a5f839ad66d77d808f86a68dd7db9a55c3c661abd8c539b7a7c05c8176036f87aeeb6b117a138327cfdb374cc23",
           "fe512a00e3c5d22cc8a849a0713df76733ed730bd66cb85b08dee5e7d6e7b15a8f6e3b3336276138023617f0ae0e6fd0c37d27aa9995f9803fe78df4941dd3ec",
           "ab0c2e8ea8ba037f169dbb384605b227016c617d48495fd78250b2809dfa06203696fdfb2a9716ae4b1bca0d5b0a861a1e5d64562994aeb515eed49290c9f1c2",
           "0bccefe21fa6beb22335e1b200c8a4fdd9d250527d643f33bb4d080d6941ff1a578451935c370d9c7fbcdd77e35a40e49bf0a5311e065035778ac27e6263b10d",
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
				"5079f570cde7801c70a19fb2c7e292d09923218f2684c8a1121c2da7a02a5dc3369c31f242bfc5093815511e4a4eda297f4b8772a7ff98f7806ce7a80ffffb35\n")
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
        newcontent = "p2p-peer-address = %s:9876\nrpos-p2p-peer-address = %s:9875\n" % (peer,peer)
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

