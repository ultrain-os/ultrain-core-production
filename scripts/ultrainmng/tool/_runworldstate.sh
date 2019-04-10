#!/bin/bash
ULTRAIN_PATH=$1
nohup $ULTRAIN_PATH/wssultrain --http-server-address 127.0.0.1:7777 &>> /log/wss-${DATE}-${HOSTNAME}.log  &

