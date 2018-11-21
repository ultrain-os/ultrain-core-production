#!/bin/bash
if [ ! -d "config/IPs" ]; then
  mkdir -p config/IPs
fi
for i in `docker ps | grep yhc | awk '{print $1}'`; 
do echo $i; 
docker inspect $i -f '{{.Config.Hostname}}';
docker inspect $i -f '{{.NetworkSettings.IPAddress}}' # '{{.NetworkSettings.Networks.globalnet.IPAddress}}';
done > config/IPs/dockerinfo.txt
python generateconfig.py  &>> generateconfig.log  &
docker ps | grep yhc-[1-7]$ | awk '{print $1}' | xargs -i docker exec -d {} bash -c "mkdir -p /root/.local/share/ultrainio/nodultrain/config"
docker ps | grep yhc-[1-7]$ | awk '{print $1}' | xargs -i docker cp config/config/{}.ini {}:/root/.local/share/ultrainio/nodultrain/config/config.ini
#cmd=`docker inspect yhc-1 | grep "IPAddress" | sed -n 2p | sed 's/\"IPAddress\"://g' | sed 's/"//g' | sed 's/,//g'`
cmd="/root/workspace/ultrain-core/scripts/_runultrain.sh "
docker ps | grep yhc-[1-7]$ | awk '{print $1}' | xargs -i docker exec -d {} bash -c "rm -rf /tmp/* "
docker ps | grep yhc-[1-7]$ | awk '{print $1}' | xargs -i docker exec -d {} bash -c "mkdir -p /tmp/ "
docker ps | grep yhc-[1-7]$ | awk '{print $1}' | xargs -i docker exec -d {} bash -c "$cmd"
cmd="nohup /root/workspace/ultrain-core/scripts/logrotate.sh &"
docker ps | grep yhc-[1-7]$ | awk '{print $1}' | xargs -i docker exec -d {} bash -c "$cmd"
#docker ps | grep yufeng-[1-7]$ | awk '{print $1}' | xargs -i docker exec {} bash -c "rm -rf /root/.local/share/ultrainio/nodultrain/ "
