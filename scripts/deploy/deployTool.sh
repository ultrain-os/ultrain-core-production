#!/usr/bin/env bash
# 服务器上部署，创世前动作
echo "clear old dir========"
echo "rm /root/deploy -rf"
rm /root/deploy -rf
echo "rm /root/workspace/ultrain-core/ -rf"
rm /root/workspace/ultrain-core/ -rf
echo "rm /root/nodultrain"
rm /root/nodultrain

echo "tar -xvf deploy.tar========"
tar -xvf deploy.tar

echo "make new dir========"
echo "mkdir /root/workspace/ultrain-core -p"
mkdir /root/workspace/ultrain-core -p

echo "copy promgram,data and scipts"
echo "cp /root/deploy/build/programs/nodultrain/nodultrain /root/nodultrain"
cp /root/deploy/build/programs/nodultrain/nodultrain /root/nodultrain
echo "cp /root/deploy/build /root/workspace/ultrain-core/build -r"
cp /root/deploy/build /root/workspace/ultrain-core/build -r
echo "cp /root/deploy/scripts/ /root/workspace/ultrain-core/scripts -r"
cp /root/deploy/scripts/ /root/workspace/ultrain-core/scripts -r


