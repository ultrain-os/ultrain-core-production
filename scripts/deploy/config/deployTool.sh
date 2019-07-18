#!/usr/bin/env bash
# 服务器上部署，创世前动作
echo "clear old dir========"
echo "rm /root/deploy -rf"
rm /root/deploy -rf
echo "rm /root/workspace/ultrain-core/ -rf"
rm /root/workspace/ultrain-core/ -rf


echo "tar -xvf deploy.tar to /root========"
tar -xvf deploy.tar -C /root/

echo "make new dir========"
echo "mkdir /root/workspace/ultrain-core -p"
mkdir /root/workspace/ultrain-core -p
mkdir /root/workspace/ultrain-core/scripts -p


echo "cp /root/deploy/build /root/workspace/ultrain-core/build -r"
cp /root/deploy/build /root/workspace/ultrain-core/build -r
echo "cp /root/deploy/scripts/ /root/workspace/ultrain-core/scripts -r"
cp /root/deploy/scripts/ /root/workspace/ultrain-core/scripts -r
cd /root/workspace/ultrain-core/scripts/

/root/workspace/ultrain-core/build/programs/clultrain/clultrain system listproducers


