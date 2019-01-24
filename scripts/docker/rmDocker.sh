#!/usr/bin/env bash
#batch remove all related dockers
NAME=$1
docker stop $(docker ps -a | grep $NAME- | awk '{print $1}')
docker rm $(docker ps -a | grep Exited | grep $NAME- | awk '{print $1}')
