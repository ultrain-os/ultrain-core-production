#!/usr/local/bin/python2

import os
import datetime
import time
import random
import copy
#import pymysql

class Host(object):

	def __init__(self, ip, name):
		self.ip = ip
		self.name = name
		self.peers = []
		self.isleaf = False

class Container(object):

	def __init__(self, id, name, ip, hostip):
		self.id = id
		self.name = name
		self.ip = ip
		self.hostip = hostip
		self.peers = []
		self.isleaf = False

class Node(object):

	def __init__(self, ip, name):
		self.id=ip


# global parameters
hosts={}
allIDs=[]
allNames=[]
allIPs=[]
allContainers=[]
allHosts=[]
allAccounts=[]
allPriKeys = []
allPubKeys =[]
allAsk=[]
allApk=[]
allBsk=[]
allBpk=[]
offlineNodes = []

# for i in range(0,5):
# 	containers = []
# 	for j in range(0,5):
# 		container = Container("id-%s-%s" % (i,j),"10.0.%s.%s" % (i,j))
# 		containers.append(container)
# 	hostip="172.16.10.%s" % i
# 	hosts[hostip]=containers
#
# for host in hosts:
# 	print(hosts)
# 	print(host)
# 	for con in hosts[host]:
# 		print(con.ip,con.id)

# load containers' parameters in hosts
def load_parameters():
	global hosts,allIDs,allNames,allIPs,allContainers,allHosts,allAccounts,allPriKeys,allPubKeys,allAsk,allApk,allBsk,allBpk,offlineNodes
	with open('hosts-h.txt') as f:
		elements = f.read().splitlines()
		allNames = elements[0::2]
		allIPs = elements[1::2]

	for i in range(0, len(allIPs)):
		host = Host(allIPs[i], allNames[i], )
		allHosts.append(host)

	with open('keypair.txt') as f:
		keys = f.read().splitlines()
	allAccounts = keys[6::8]
	allPriKeys = keys[0::8]
	allPubKeys = keys[1::8]
	allAsk = keys[2::8]
	allApk = keys[3::8]
	allBsk = keys[4::8]
	allBpk = keys[5::8]

	print(allIDs)
	print(allNames)
	print(allIPs)
#print(allBsk)

def get_offline_nodes():
	offline_list = []
	try:
		my_con = pymysql.connect(host='47.101.62.161',
								 port=3306,
								 user='root',
								 passwd='12345678',
								 db='trade',
								 charset='utf8'
								 )

	# my_cousor = my_con.cursor()
	# sql_select_all = 'select  distinct node_ip from log_info_dynamic;'
	# sql_select_online = "select  distinct \
	# 					node_ip	\
	# 					from log_info_dynamic \
	# 					where gmt_modify between date_add(now(), interval - 2777 minute) and now()"
	#
	# my_cousor.execute(sql_select_all)
	# get_all = my_cousor.fetchall()
	# my_cousor.execute(sql_select_online)
	# get_online = my_cousor.fetchall()
	#
	# get_offline = list(set(get_all).difference(set(get_online)))
	#
	# offline_list = [i[0] for i in get_online]
	#
	# my_cousor.close()
	# my_con.close()
	except pymysql.InternalError as e:
		print("mysql is not avialable.")

	return offline_list

def make_star(connects=3):
	total_num = len(allIPs)
	for i in range(0,total_num):
		for j in range(1,connects+1):
			k = (i+j) % total_num
			allHosts[i].peers.append(allIPs[k])

def write_dot_file():
	fdot = "dot/%s.dot" % datetime.datetime.now().strftime('%Y-%m-%d-%H-%M-%S')
	with open(fdot, "a") as file:
		file.write("digraph graphname {\n")
		file.write("nodesep=1;\noverlap=scalexy;\nnode [color=lightblue2, style=filled]\nedge [color=Blue, style=dashed]\n")
		for i in range(0,len(allIPs)):
			for peer in allHosts[i].peers:
				index_peer = allIPs.index(peer)
				if allIPs[i] in offlineNodes:	color = "color=Red"
				else:	color = ""
				constr = "\"%s\"[label=\"%s\\n%s\" %s];\n" % (allIPs[i],allNames[i],allIPs[i],color)
				if peer in offlineNodes:	color = "color=Red"
				else:	color = ""
				constr = constr + "\"%s\"[label=\"%s\\n%s\" %s];\n" % (peer,allNames[index_peer],peer,color)
				# constr = constr + "\"%s\" -> \"%s\"[label=\"%s -> %s\"];\n" % (allIPs[i],peer,allIPs[i],peer)
				constr = constr + "\"%s\" -> \"%s\";\n" % (allIPs[i], peer)
				file.write(constr)
		file.write("}")
	# cmd = "circo %s -Tpng -o%s.png" % (fdot,fdot)
	cmd = "neato %s -Tpdf -o%s.pdf" % (fdot, fdot)
	os.system(cmd)

def write_config_file():
	insert_genesis_time()
	index_key = 0
	genesisHost="172.17.0.80"
	seedHosts=["172.17.0.93","172.17.0.81"]
	mongoHosts=[]
	for host in allHosts:
		os.system("rm -rf config/" + host.ip)
		os.system("mkdir config/" + host.ip)
		fname = "config/%s/config.ini" % (host.ip)
		print(fname)
		if not os.path.isfile(fname):  # file does not exist
			cmd = "cp template.txt " + fname
			os.system(cmd)

		if host.ip == genesisHost:
			insert_leader_sk(fname)
		else:
			insert_keys(fname, index_key)
			print(host.ip,host.name,allAccounts[index_key][8:],allAsk[index_key][4:],allApk[index_key][4:])
			index_key = index_key + 1

		if host.ip in seedHosts:
			insert_non_producing(fname)
			insert_max_clients(fname)
			insert_seed_peer_address(fname,host.ip,seedHosts)

		# add udp seed(except mongo)
		if host.ip not in mongoHosts:
			insert_udp_seed(fname,host.ip,seedHosts)

		# add mongo host info
		if host.ip  in mongoHosts:
			insert_non_producing(fname)
			insert_seed_peer_address(fname,host.ip,seedHosts);

#print(host.ip,host.name,allAccounts[index_key-1][8:],allAsk[index_key-1][4:],allApk[index_key-1][4:])
#for peer in host.peers:
#	print(host.ip,peer)
#	insert_peer(fname,peer)

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
	after_time = (datetime.datetime.now()+datetime.timedelta(minutes=2)).strftime("%Y-%m-%dT%H:%M") + ":00"
	newcontent = "genesis-time = %s\n" % after_time
	index_line = content.index("#insert_genesis-time\n")
	content.insert(index_line+1,newcontent)
	writefile(fname,content)

def insert_leader_sk(fname):
	content = readfile(fname)
	newcontent = "my-account-as-committee = %s\nmy-sk-as-committee = %s\nmy-sk-as-account = %s\nmy-bls-sk = %s\n" % ("genesis", \
																													 "5079f570cde7801c70a19fb2c7e292d09923218f2684c8a1121c2da7a02a5dc3369c31f242bfc5093815511e4a4eda297f4b8772a7ff98f7806ce7a80ffffb35", \
																													 "5KkYKJbWHZ9zneyQMLPUZVFEeszCZmRCXNf5CbQQdPFL9FfsuED", \
																													 "26bf5ac96faa98fa97be285639d17fbca7d8f5ef")
	index_line = content.index("#insert_my_keys\n")
	content.insert(index_line+1, newcontent)
	writefile(fname,content)

def insert_non_producing(fname):
	content = readfile(fname)
	newcontent = "is-non-producing-node = 1\n"
	print(newcontent)
	index_line = content.index("#insert_if_producing-node\n")
	content.insert(index_line+1, newcontent)
	content.insert(index_line+2, "plugin = ultrainio::txn_test_gen_plugin\n")

	writefile(fname,content)

def insert_max_clients(fname):
	content = readfile(fname)
	newcontent = "max-clients = 32\n"
	print(newcontent)
	index_line = content.index("#max-clients\n")
	content.insert(index_line+1, newcontent)
	writefile(fname,content)

def insert_seed_peer_address(fname,ip,seedHosts):
	content = readfile(fname)
	index_line = content.index("#insert_peers\n")
	index = 1;
	for seedIp in seedHosts :
		if ip != seedIp :
			newcontent = "p2p-peer-address = %s:20122\nrpos-p2p-peer-address = %s:20123\n" % (seedIp,seedIp)
			content.insert(index_line+index, newcontent)
			index = index+1;
			print(newcontent)
	writefile(fname,content)

def insert_udp_seed(fname,ip,seedHosts):
	content = readfile(fname)
	index_line = content.index("#insert_udp_seeds\n")
	index = 1;
	for seedIp in seedHosts :
		newcontent = "udp-seed  = %s\n" % (seedIp)
		content.insert(index_line+index, newcontent)
		index = index+1;
	writefile(fname,content)

def insert_peer(fname, peer):
	content = readfile(fname)
	newcontent = "p2p-peer-address = %s:20122\nrpos-p2p-peer-address = %s:20123\n" % (peer,peer)
	print(newcontent)
	index_line = content.index("#insert_peers\n")
	content.insert(index_line+1, newcontent)
	writefile(fname,content)

def insert_keys(fname,index_key):
	#print(allBsk[index_key][7:])
	content = readfile(fname)
	newcontent = "my-account-as-committee = %s\nmy-sk-as-committee = %s\nmy-sk-as-account = %s\nmy-bls-sk = %s\n" % \
				 (allAccounts[index_key][8:],allPriKeys[index_key][4:],allAsk[index_key][4:],allBsk[index_key][7:])
	index_line = content.index("#insert_my_keys\n")
	content.insert(index_line+1, newcontent)
	writefile(fname, content)

g_node_index = 0
def make_tree(nodes_num, tree_depth, cur_depth, subnets=2):
	global g_node_index
	fnode = g_node_index

	for i in range(1, subnets+1):
		g_node_index = g_node_index + 1
		if g_node_index > nodes_num-1:
			break
		allHosts[fnode].peers.append(allIPs[g_node_index])
		if cur_depth == tree_depth:
			allHosts[g_node_index].isleaf = True
		if cur_depth < tree_depth:
			make_tree(nodes_num, tree_depth, cur_depth + 1, subnets)

def connect_tree_leafs(nodes_num):
	leafHosts = []
	for host in allHosts:
		if host.isleaf == True:
			leafHosts.append(host)
	for host in leafHosts:
		if host.isleaf == True:
			index_con = leafHosts.index(host)
			if index_con+1 < len(leafHosts):
				host.peers.append(leafHosts[index_con+1].ip)
			if index_con+1 == len(leafHosts):
				host.peers.append(leafHosts[0].ip)

def rv_depth(nodes_num, subnets=2):
	depth = 0;
	a = [0]
	while nodes_num > sum([subnets ** x for x in a]):
		depth = depth + 1
		a.append(depth)

	return depth, sum([subnets ** x for x in a]) - nodes_num

def make_radom_network(connects=3):
	total_num = len(allIPs)
	for i in range(0,total_num):
		random.seed(time.time())
		tempIPS = copy.deepcopy(allIPs)
		tempIPS.remove(allIPs[i])
		peers =	random.sample(tempIPS,connects)
		for peer in peers:
			index_num = allIPs.index(peer)
			if allHosts[i].ip in allHosts[index_num].peers:
				peers.remove(peer)
		allHosts[i].peers.extend(peers)

# def clear_dir():


load_parameters()

#make_star(2)

# nodes_num = len(allIPs)
# subnet = 2
# tree_depth, deficit_nodes = rv_depth(nodes_num,subnet)
# make_tree(nodes_num,tree_depth,1,subnet)
# connect_tree_leafs(nodes_num)

#make_radom_network(connects=2)

#write_dot_file()
write_config_file()

