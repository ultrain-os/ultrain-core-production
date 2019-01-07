#!/bin/bash
NAME=$1
ULTRAIN_PATH=$2
cmd="$ULTRAIN_PATH/ultrain-core/scripts/_runultrain.sh "
docker ps | grep $NAME-[1-7]$ | awk '{print $1}' | xargs -i docker exec -d {} bash -c "killall -2  nodultrain"
docker ps | grep $NAME-[1-7]$ | awk '{print $1}' | xargs -i docker exec -d {} bash -c "$cmd $ULTRAIN_PATH"