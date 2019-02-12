#!/bin/bash
# achive all required files to a tar
# sample：sh /home/sidechain/ultrain-core/scripts/deploy/archive-prod.sh /home/sidechain/ultrain-core/ /root/
# sample(docker): sh /root/workspace/ultrain-core/scripts/deploy/archive-prod.sh /root/workspace/ultrain-core/ /root/
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
mkdir $DEPLOY_PATH/deploy/ultrainmng -p
cp $ULTRAIN_PATH/scripts/ultrainmng/node_modules $DEPLOY_PATH/deploy/ultrainmng/node_modules -r
cp $ULTRAIN_PATH/scripts/deploy/ultrainmng/config.ini $DEPLOY_PATH/deploy/ultrainmng/config.ini
cp $ULTRAIN_PATH/scripts/deploy/ultrainmng/seedconfig.json $DEPLOY_PATH/deploy/ultrainmng/seedconfig.json
cp $ULTRAIN_PATH/scripts/ultrainmng/tool $DEPLOY_PATH/deploy/ultrainmng/tool -rf
mkdir $DEPLOY_PATH/deploy/ultrainmng/src
cp $ULTRAIN_PATH/scripts/ultrainmng/deploy/sideChainService.js $DEPLOY_PATH/deploy/ultrainmng/src/sideChainService.js
cd $DEPLOY_PATH/deploy && tar -czvf ultrainmng.tar ./ultrainmng/
rm $DEPLOY_PATH/deploy/ultrainmng -rf
mkdir $DEPLOY_PATH/deploy/ultrainmng -p
cp $ULTRAIN_PATH/scripts/deploy/ultrainmng/config.ini $DEPLOY_PATH/deploy/ultrainmng/config.ini
cp $ULTRAIN_PATH/scripts/deploy/ultrainmng/seedconfig.json $DEPLOY_PATH/deploy/ultrainmng/seedconfig.json

# 打包deploy.tar
cd $DEPLOY_PATH && tar -czvf deploy.tar ./deploy






