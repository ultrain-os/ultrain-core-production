#!/bin/bash
NAME=$1
docker ps | grep $NAME- | awk '{print $1}' | xargs -i docker exec -d {} bash -c "/usr/local/bin/pm2 restart sideChainService"


