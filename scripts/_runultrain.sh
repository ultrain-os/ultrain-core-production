#!/bin/bash
ULTRAIN_PATH=$1
rm -f /log/${HOSTNAME}.log
nohup $ULTRAIN_PATH/ultrain-core/scripts/_runultrain.py $ULTRAIN_PATH/ultrain-core/build &>> /log/${HOSTNAME}.log  &
