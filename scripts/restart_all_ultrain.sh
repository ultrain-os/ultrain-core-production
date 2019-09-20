#!/bin/bash
NAME=$1
ULTRAIN_PATH=$2
cmd="$ULTRAIN_PATH/scripts/_runultrain.sh "
docker ps | grep $NAME- | awk '{print $1}' | xargs -i docker exec -d {} bash -c "killall  nodultrain"
docker ps | grep $NAME- | awk '{print $1}' | xargs -i docker exec -d {} bash -c "killall  wssultrain"
docker ps | grep $NAME- | awk '{print $1}' | xargs -i docker exec -d {} bash -c "$cmd $ULTRAIN_PATH"
wscmd="$ULTRAIN_PATH/build/programs/wssultrain/wssultrain --http-server-address 127.0.0.1:7777 > /log/ws-\${HOSTNAME}.log 2>&1 &"
docker ps | grep $NAME- | awk '{print $1}' | xargs -i docker exec -d {} bash -c "$wscmd"
docker ps | grep $NAME- | awk '{print $1}' | xargs -i docker exec -d {} bash -c "/usr/local/bin/pm2 restart votingRandService"
