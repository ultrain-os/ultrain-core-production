#!/bin/bash
ULTRAIN_PATH=$1
rm -f /log/ws-${HOSTNAME}.log
$ULTRAIN_PATH/ultrain-core/build/programs/wssultrain/wssultrain --http-server-address 127.0.0.1:7777 > /log/ws-${HOSTNAME}.log  2>&1 &

