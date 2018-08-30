#!/bin/bash
cmd=`docker inspect yufeng-1 | grep "IPAddress" | sed -n 2p | sed 's/\"IPAddress\"://g' | sed 's/"//g' | sed 's/,//g'`
cmd="/root/workspace/yufengshen/_runultrain.sh $cmd"
docker ps | grep yufeng-[1-7]$ | awk '{print $1}' | xargs -i docker exec {} bash -c "rm -rf /tmp/* "
docker ps | grep yufeng-[1-7]$ | awk '{print $1}' | xargs -i docker exec {} bash -c "mkdir -p /tmp/ "
docker ps | grep yufeng-[1-7]$ | awk '{print $1}' | xargs -i docker exec {} bash -c "$cmd"
#docker ps | grep yufeng-[1-7]$ | awk '{print $1}' | xargs -i docker exec {} bash -c "rm -rf /root/.local/share/ultrainio/nodultrain/ "
