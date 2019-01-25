#!/bin/bash
ULTRAIN_PATH=$1
rm -f /log/${HOSTNAME}.log
nohup $ULTRAIN_PATH/nodultrain  > /log/${HOSTNAME}.log  2>&1 &
