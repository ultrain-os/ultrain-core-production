#!/bin/bash
NAME=$1
ULTRAIN_PATH=$2
docker ps | grep $NAME- | awk '{print $1}' | xargs -i docker exec -d {} bash -c "/usr/local/bin/pm2 start $ULTRAIN_PATH/ultrain-core/scripts/ultrainmng/src/sideChainService.js -o /log/sideChainService.pm2.log -e /log/sideChainService.pm2.error.log"
