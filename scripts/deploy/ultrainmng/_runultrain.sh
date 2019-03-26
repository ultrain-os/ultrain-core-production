#!/bin/bash
ULTRAIN_PATH=$1
WSSPATH=$2
DATE=`date +%Y-%m-%d-%H-%M`
nohup $ULTRAIN_PATH/nodultrain $WSSPATH &>> /root/log/${DATE}-${HOSTNAME}.log  &