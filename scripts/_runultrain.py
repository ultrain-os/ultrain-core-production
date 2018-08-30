#! /usr/bin/python
import string
import socket
import os
import sys
savedstdout=sys.stdout
with open('out.txt', 'w+') as file:
    sys.stdout = file
buildpath=sys.argv[1]
center_node = sys.argv[2]
cmdpath=buildpath+"/programs/nodultrain/nodultrain"
plugin=" --plugin ultrainio::monitor_api_plugin --plugin ultrainio::chain_api_plugin --plugin ultrainio::wallet_api_plugin --plugin ultrainio::history_api_plugin --plugin ultrainio::chain_plugin --plugin ultrainio::http_plugin "
log=" -l /root/workspace/logging.json "
oscmd="rm -rf  ~/ultrainio-wallet/default.wallet"
os.system(oscmd)
myname = socket.getfqdn(socket.gethostname(  ))
myaddr = socket.gethostbyname(myname)
logfile=" >/log/"+myname+".log &"
name_index = (int)(myname.split('-')[-1])
addr_index=myaddr.split('.')[-1]
peer_pre=myaddr[:myaddr.rindex('.')]

pk_list = ["b7e0a16fdca44d4ece1b14d8e7e6207402a6447115ca7d2d7edb08958e6d8ed5",
"4031a95071a092eca8646d3999c438bdfde368d4837770755af65a11b4520a48",
"2b43a3d8e0523a85141bbfca41006cf9abc47587d0ff3c46d257551ec05f1677",
"b9a55c3c661abd8c539b7a7c05c8176036f87aeeb6b117a138327cfdb374cc23",
"8f6e3b3336276138023617f0ae0e6fd0c37d27aa9995f9803fe78df4941dd3ec",
"3696fdfb2a9716ae4b1bca0d5b0a861a1e5d64562994aeb515eed49290c9f1c2",
"578451935c370d9c7fbcdd77e35a40e49bf0a5311e065035778ac27e6263b10d",
 ]

sk_list = ["97d157bf7fdedfeaef46216e246cc83f9c25574b49bacf83a1862ef0d82233ecb7e0a16fdca44d4ece1b14d8e7e6207402a6447115ca7d2d7edb08958e6d8ed5",
           "b5b3b423eed6fd6f255fe1e4570e3a9f9878a36870b584b1d231ca778ad95ede4031a95071a092eca8646d3999c438bdfde368d4837770755af65a11b4520a48",
           "5cfdb237515f969620f857658baac5485266f1c143fee80208e0da9935bbfaae2b43a3d8e0523a85141bbfca41006cf9abc47587d0ff3c46d257551ec05f1677",
           "6b8ed821703f369f1a6b7c767bf076b093a17a5f839ad66d77d808f86a68dd7db9a55c3c661abd8c539b7a7c05c8176036f87aeeb6b117a138327cfdb374cc23",
           "fe512a00e3c5d22cc8a849a0713df76733ed730bd66cb85b08dee5e7d6e7b15a8f6e3b3336276138023617f0ae0e6fd0c37d27aa9995f9803fe78df4941dd3ec",
           "ab0c2e8ea8ba037f169dbb384605b227016c617d48495fd78250b2809dfa06203696fdfb2a9716ae4b1bca0d5b0a861a1e5d64562994aeb515eed49290c9f1c2",
           "0bccefe21fa6beb22335e1b200c8a4fdd9d250527d643f33bb4d080d6941ff1a578451935c370d9c7fbcdd77e35a40e49bf0a5311e065035778ac27e6263b10d",
           ]

my_key = " --my-pk-as-committee " + pk_list[name_index - 1] + " --my-sk-as-committee " + sk_list[name_index - 1]

genesis_pk = " --genesis-leader-pk b8656c9770bfc70f0d79ea1e8e2321440e4f48327e3e282388eb28cc4501c99e"
genesis_sk = " --genesis-leader-sk 74f5591d9c70a6168a04592e5f253cfe63e0528c1015f85af6c2d81cf0de08f9b8656c9770bfc70f0d79ea1e8e2321440e4f48327e3e282388eb28cc4501c99e"

cmdpath += " --http-server-address 127.0.0.1:8888 -l /root/workspace/logging.json " + plugin

#addr1=(int)(addr_index)-1
#peer_addr1=peer_pre + "." + str(addr1) + ":9876"
peer_addr1 = center_node + ":9876"
oscmd=cmdpath + " --p2p-peer-address " + peer_addr1 + genesis_pk + my_key

if name_index==1:
    oscmd += genesis_sk

if name_index==7:
    oscmd += " --is-non-producing-node 1 "

os.system(oscmd)
