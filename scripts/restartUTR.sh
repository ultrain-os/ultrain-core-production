#!/bin/bash
NAME=$1
ULTRAIN_PATH=$2
cmd="$ULTRAIN_PATH/ultrain-core/scripts/_runultrain.sh "
docker ps | grep $NAME-[1-7]$ | awk '{print $1}' | xargs -i docker exec -d {} bash -c "killall -2  nodultrain"
docker ps | grep $NAME-[1-7]$ | awk '{print $1}' | xargs -i docker exec -d {} bash -c "killall -2  wssultrain"
docker ps | grep $NAME-[1-7]$ | awk '{print $1}' | xargs -i docker exec -d {} bash -c "$cmd $ULTRAIN_PATH"
wscmd="$ULTRAIN_PATH/ultrain-core/build/programs/wssultrain/wssultrain --http-server-address 127.0.0.1:7777 > /log/ws-${HOSTNAME}.log 2>&1 &"
docker ps | grep $NAME-[1-7]$ | awk '{print $1}' | xargs -i docker exec -d {} bash -c "$wscmd"