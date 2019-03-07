#!/usr/bin/env bash

#--------------------------------------------
# 参数信息：
#
# NAME_PREFIX docker名前缀
# DOCKER_COUNT 需要创建的docker个数，必须>=1
# IMAGE_NAME docker image名称
# PORT_MAP 端口映射，如8855:8888
# FILE_DIR 本地目录映射,如 /home/sidechain
#
# 例子： sh createDocker.sh zuofei 7 sidechain/ubuntu:v1 8855:8888
#--------------------------------------------

echo "create dockers ..."

#args
NAME_PREFIX=$1
DOCKER_COUNT=$2
IMAGE_NAME=$3
PORT_MAP=$4
FILE_DIR=$5

#max docker count
MAX_DOCKER_COUNT=8

# arg num check
if [ $# != 5 ] ; then
echo "input args is invalid"
exit 1;
fi

# docker count check
if test $DOCKER_COUNT -lt 1; then
 echo "docker count must great than 1"
exit 1;
fi

if test $DOCKER_COUNT -gt $MAX_DOCKER_COUNT; then
 echo "docker count must less than "$MAX_DOCKER_COUNT
exit 1;
fi


for i in $(seq 1 $DOCKER_COUNT)
do
    if [ ${i} -eq $DOCKER_COUNT ]; then
		docker run -itd --name ${NAME_PREFIX}-${i}  -h ${NAME_PREFIX}-${i}  -v /etc/localtime:/etc/localtime:ro \
	  	-v ${FILE_DIR}/:/root/workspace -p ${PORT_MAP} -v ${FILE_DIR}/log:/log ${IMAGE_NAME} bash
	else
		docker run -itd --name ${NAME_PREFIX}-${i}  -h ${NAME_PREFIX}-${i}  -v /etc/localtime:/etc/localtime:ro \
	  	-v ${FILE_DIR}/:/root/workspace  -v ${FILE_DIR}/log:/log ${IMAGE_NAME} bash
	fi
done


echo "create dockerd succesfully.."
echo "\ndockers information"
docker ps -a | grep ${NAME_PREFIX}-]
