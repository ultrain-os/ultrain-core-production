#!/bin/bash
ULTRAIN_PATH=$1
DATE=`date +%Y-%m-%d-%H-%M`
nohup $ULTRAIN_PATH/nodultrain  &>> /root/log/${DATE}-${HOSTNAME}.log  &
