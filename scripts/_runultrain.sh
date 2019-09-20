#!/bin/bash
ULTRAIN_PATH=$1
rm -f /log/${HOSTNAME}.log
nohup $ULTRAIN_PATH/scripts/_runultrain.py $ULTRAIN_PATH/build &>> /log/${HOSTNAME}.log  &
