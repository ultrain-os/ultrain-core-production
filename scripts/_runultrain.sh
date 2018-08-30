#!/bin/bash
nohup /root/workspace/yufengshen/_runultrain.py /root/workspace/yufengshen/ultrain-core/build/ $1 &> /log/${HOSTNAME}.log  &
