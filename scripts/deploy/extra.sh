#!/usr/bin/env bash
# extra tar file and deploy

echo "clear old dir========"
echo "rm /root/deploy -rf"
rm /root/deploy -rf
echo "rm /root/workspace/ultrain-core/ -rf"
rm /root/workspace/ultrain-core/ -rf


echo "tar -xvf deploy.tar to /root========"
tar -xvf /root/deploy.tar -C /root/

echo "make new dir========"
echo "mkdir /root/workspace/ultrain-core -p"
mkdir /root/workspace/ultrain-core -p
mkdir /root/workspace/ultrain-core/scripts -p


echo "cp /root/deploy/build /root/workspace/ultrain-core/build -r"
cp /root/deploy/build /root/workspace/ultrain-core/build -r
echo "cp /root/deploy/scripts /root/workspace/ultrain-core/ -r"
cp /root/deploy/scripts/ /root/workspace/ultrain-core/ -r
cd /root/workspace/ultrain-core/scripts/



