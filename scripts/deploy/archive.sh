#!/bin/bash
# achive all required files to a tar
# sample：sh /home/sidechain/ultrain-core/scripts/deploy/archive.sh /home/sidechain/ultrain-core/ /root/
# sample(docker): sh /root/workspace/ultrain-core/scripts/deploy/archive.sh /root/workspace/ultrain-core/ /root/
ULTRAIN_PATH=$1
DEPLOY_PATH=$2

# 删除原文件
rm $DEPLOY_PATH/deploy -rf

# 拷贝build目录
mkdir $DEPLOY_PATH/deploy -p
cp $ULTRAIN_PATH/build $DEPLOY_PATH/deploy/build -r

# 拷贝scripts
mkdir $DEPLOY_PATH/deploy/scripts -p
cp $ULTRAIN_PATH/scripts $DEPLOY_PATH/deploy/ -r

# 拷贝ultrainmngU
cp $ULTRAIN_PATH/scripts/ultrainmng $DEPLOY_PATH/deploy/ultrainmng -r
rm $DEPLOY_PATH/deploy/ultrainmng/deploy -rf
cp $ULTRAIN_PATH/scripts/deploy/ultrainmng/config.ini $DEPLOY_PATH/deploy/ultrainmng/config.ini
cp $ULTRAIN_PATH/scripts/deploy/ultrainmng/seedconfig.json $DEPLOY_PATH/deploy/ultrainmng/seedconfig.json
cd $DEPLOY_PATH/deploy && tar -czvf ultrainmng.tar ./ultrainmng/
rm $DEPLOY_PATH/deploy/ultrainmng -rf

# 打包deploy.tar
cd $DEPLOY_PATH && tar -czvf deploy.tar ./deploy






