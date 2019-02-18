#!/usr/local/bin/python2
import argparse
import os
import datetime
from account_info import *

parser = argparse.ArgumentParser()
parser.add_argument('-m', '--masterchain', action='store_true', help="set current master chain")
parser.add_argument('-sub', '--subchain', type=str, help="set subchain name info")
parser.add_argument('-mainHttp', '--mainchainHttp', type=str, help="set mainchainHttpEndpoint for ultranmng")
parser.add_argument('-httpAlias', '--httpAlias', type=str, help="set http alias for noultrain")
parser.add_argument('-ws', '--worldstate', action='store_true', help="enable worldstate ")
parser.add_argument('-tcp', '--tcp', action='store_true', help="enable tcp,ignore udp")
args = parser.parse_args()
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

    adjustaccounts = ["genesis",]
    if args.masterchain:
        for a in accounts[1:]:
            adjustaccounts.append("master"+a)
    elif args.subchain:
        for a in accounts[1:]:
            adjustaccounts.append("user"+"."+args.subchain+a)
    else:
        for a in accounts[1:]:
            adjustaccounts.append("user"+a)
    allAccounts = adjustaccounts
    allIDs = allIDs + IDs
    allNames = allNames + Names
    allIPs = allIPs + IPs
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
            if con.name[len(con.name)-1::len(con.name)] != '7':
                insert_keys(fname, index_key)
            if con.name[len(con.name)-1::len(con.name)] == '7':
                insert_keys(fname, index_key)
                insert_non_producing(fname)
            update_ultrainmng_config(fname,hosts[dockerinfo])
            insert_worldstate_config(fname)
            print(hostip,con.ip,con.id)
            if args.tcp == False:
                insert_udp_seed(fname,hosts[dockerinfo][0].ip);
            else :
                insert_peer(fname,hosts[dockerinfo][0].ip);

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
	after_time = (datetime.datetime.utcnow()+datetime.timedelta(minutes=2)).strftime("%Y-%m-%dT%H:%M") + ":00"
	newcontent = "genesis-time = %s\n" % after_time
	index_line = content.index("#insert_genesis-time\n")
	content.insert(index_line+1,newcontent)
	writefile(fname,content)

# update ultrainmng  config
def update_ultrainmng_config(fname,ipList):
    content = readfile(fname)
    newcontent = "";
    nonProducingNodeIp=ipList[len(ipList)-1].ip;
    newcontent = "subchainHttpEndpoint = http://"+nonProducingNodeIp+":8888\n"
    # if args.masterchain:
    #     newcontent = newcontent+"masterchain = 1\n"
    #if is subchain,add mainchain http endpoint
    if args.subchain:
        httpUrl = "";
        if args.mainchainHttp:
            httpUrl = "http://"+args.mainchainHttp;
        newcontent = newcontent+"mainchainHttpEndpoint = "+httpUrl+"\n"
    else :
        newcontent = newcontent+"masterchain = 1\n"
    index_line = content.index("#ultrainmng_subchainHttpEndpoint\n")
    content.insert(index_line+1, newcontent)
    writefile(fname,content)

# insert worldstate config to enable worldstate
def insert_worldstate_config(fname):
    newcontent = "worldstate-control = false"
    if args.worldstate:
        newcontent = "worldstate-control = true"
    content = readfile(fname)
    index_line = content.index("#world_state_config\n")
    content.insert(index_line+1, newcontent)
    writefile(fname,content)


def insert_non_producing(fname):
    content = readfile(fname)
    newcontent = "is-non-producing-node = 1\nhttp-server-address = 0.0.0.0:8888\nhttp-alias = 172.16.10.5:8888\n"
    if args.httpAlias:
        newcontent = "is-non-producing-node = 1\nhttp-server-address = 0.0.0.0:8888\nhttp-alias = " + args.httpAlias + "\n"
    else:
        if args.masterchain:
            newcontent = "is-non-producing-node = 1\nhttp-server-address = 0.0.0.0:8888\nhttp-alias = 172.16.10.5:9999\n"
        if args.subchain == "11":
            newcontent = "is-non-producing-node = 1\nhttp-server-address = 0.0.0.0:8888\nhttp-alias = 172.16.10.5:8888\n"
        elif args.subchain == "12":
            newcontent = "is-non-producing-node = 1\nhttp-server-address = 0.0.0.0:8888\nhttp-alias = 172.16.10.5:8899\n"
    print(newcontent)
    index_line = content.index("#insert_if_producing-node\n")
    content.insert(index_line + 1, newcontent)
    writefile(fname, content)

def insert_peer(fname,peer):
    content = readfile(fname)
    newcontent = "p2p-peer-address = %s:20122\nrpos-p2p-peer-address = %s:20123\n" % (peer,peer)
    index_line = content.index("#insert_peers\n")
    content.insert(index_line+1, newcontent)
    writefile(fname,content)

def insert_udp_seed(fname,seed):
    content = readfile(fname)
    newcontent = "udp-seed = %s\n" % (seed)
    index_line = content.index("#insert_udp_seeds\n")
    content.insert(index_line+1, newcontent)
    writefile(fname,content)

def insert_keys(fname,index_key):
	content = readfile(fname)
	newcontent = "my-account-as-committee = %s\nmy-sk-as-committee = %s\nmy-sk-as-account = %s\nmy-bls-sk = %s\n" % \
				 (allAccounts[index_key],allPriKeys[index_key],account_sk_list[index_key], bls_sk_list[index_key])
	index_line = content.index("#insert_my_keys\n")
	content.insert(index_line+1, newcontent)
	writefile(fname, content)

load_parameters()
write_config_file()
