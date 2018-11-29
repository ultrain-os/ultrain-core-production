#!/bin/bash
NAME=$1
ULTRAIN_PATH=$2
if [ ! -d "config/IPs" ]; then
  mkdir -p config/IPs
fi
for i in `docker ps  --filter  name=$NAME | grep $NAME  | awk '{print $1}'`;
do echo $i;
docker inspect $i -f '{{.Config.Hostname}}';
docker inspect $i -f '{{range .NetworkSettings.Networks}}{{.IPAddress}}{{end}}';#'{{.NetworkSettings.IPAddress}}' # '{{.NetworkSettings.Networks.globalnet.IPAddress}}';
done > config/IPs/dockerinfo.txt
python generateconfig.py  &>> generateconfig.log  &
docker ps | grep $NAME-[1-7]$ | awk '{print $1}' | xargs -i docker exec -d {} bash -c "mkdir -p /root/.local/share/ultrainio/nodultrain/config"
docker ps | grep $NAME-[1-7]$ | awk '{print $1}' | xargs -i docker exec -d {} bash -c "cp $ULTRAIN_PATH/ultrain-core/scripts/config/config/{}.ini /root/.local/share/ultrainio/nodultrain/config/config.ini"
cmd="$ULTRAIN_PATH/ultrain-core/scripts/_runultrain.sh "
docker ps | grep $NAME-[1-7]$ | awk '{print $1}' | xargs -i docker exec -d {} bash -c "rm -rf /tmp/* "
docker ps | grep $NAME-[1-7]$ | awk '{print $1}' | xargs -i docker exec -d {} bash -c "mkdir -p /tmp/ "
docker ps | grep $NAME-[1-7]$ | awk '{print $1}' | xargs -i docker exec -d {} bash -c "$cmd $ULTRAIN_PATH"
cmd="nohup $ULTRAIN_PATH/ultrain-core/scripts/logrotate.sh &"
docker ps | grep $NAME-[1-7]$ | awk '{print $1}' | xargs -i docker exec -d {} bash -c "$cmd"
#docker ps | grep $NAME-[1-7]$ | awk '{print $1}' | xargs -i docker exec {} bash -c "rm -rf /root/.local/share/ultrainio/nodultrain/ "
