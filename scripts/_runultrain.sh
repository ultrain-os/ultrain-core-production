#!/bin/bash
rm -f /log/${HOSTNAME}.log
nohup /root/workspace/ultrain-core/scripts/_runultrain.py /root/workspace/ultrain-core/build &>> /log/${HOSTNAME}.log  &
