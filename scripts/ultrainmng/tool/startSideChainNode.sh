#!/bin/bash
NAME=$1
ULTRAIN_PATH=$2
docker ps | grep $NAME-[1-7]$ | awk '{print $1}' | xargs -i docker exec -d {} bash -c "/usr/local/bin/pm2 start $ULTRAIN_PATH/ultrain-core/scripts/ultrainmng/src/sideChainService.js"
