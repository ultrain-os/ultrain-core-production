#!/bin/bash
NAME=$1
ULTRAIN_PATH=$2
if [ ! -d "config/IPs" ]; then
  mkdir -p config/IPs
fi
for i in `docker ps  --filter  name=$NAME- | grep $NAME-  | awk '{print $1}'`;
do echo $i;
docker inspect $i -f '{{.Config.Hostname}}';
docker inspect $i -f '{{range .NetworkSettings.Networks}}{{.IPAddress}}{{end}}';#'{{.NetworkSettings.IPAddress}}' # '{{.NetworkSettings.Networks.globalnet.IPAddress}}';
done > config/IPs/dockerinfo.txt
python2 generateconfig.py $3 $4 $5 $6 $7 $8 $9 ${10} ${11} &>generateconfig.log  &
docker ps | grep $NAME- | awk '{print $1}' | xargs -i docker exec -d {} bash -c "mkdir -p /root/.local/share/ultrainio/nodultrain/config"
docker ps | grep $NAME- | awk '{print $1}' | xargs -i docker exec -d {} bash -c "cp $ULTRAIN_PATH/scripts/config/config/{}.ini /root/.local/share/ultrainio/nodultrain/config/config.ini"
cmd="$ULTRAIN_PATH/scripts/_runultrain.sh "
docker ps | grep $NAME- | awk '{print $1}' | xargs -i docker exec -d {} bash -c "rm -rf /tmp/* "
docker ps | grep $NAME- | awk '{print $1}' | xargs -i docker exec -d {} bash -c "mkdir -p /tmp/ "
docker ps | grep $NAME- | awk '{print $1}' | xargs -i docker exec -d {} bash -c "$cmd $ULTRAIN_PATH"
cmd="nohup $ULTRAIN_PATH/scripts/logrotate.sh &"
docker ps | grep $NAME- | awk '{print $1}' | xargs -i docker exec -d {} bash -c "$cmd"
docker ps | grep $NAME- | awk '{print $1}' | xargs -i docker exec -d {} bash -c "mkdir -p /root/.local/share/ultrainio/wssultrain/config"
docker ps | grep $NAME- | awk '{print $1}' | xargs -i docker exec -d {} bash -c "cp $ULTRAIN_PATH/scripts/ws/config.ini /root/.local/share/ultrainio/wssultrain/config/config.ini"
wscmd="$ULTRAIN_PATH/build/programs/wssultrain/wssultrain --http-server-address 127.0.0.1:7777 > /log/ws-\${HOSTNAME}.log 2>&1 &"
docker ps | grep $NAME- | awk '{print $1}' | xargs -i docker exec -d {} bash -c "$wscmd"

