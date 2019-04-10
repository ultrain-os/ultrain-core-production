#!/bin/bash
set -x
#./stopchains.sh
rm -rf deploy
tar xf deploy.tar
# update deploy config
sh /root/deploy/scripts/deploy/config/updateConfig.sh deploy-master
sh /root/deploy/scripts/deploy/config/updateConfig.sh deploy-env01
sh /root/deploy/scripts/deploy/config/updateConfig.sh deploy-env02
sh /root/deploy/scripts/deploy/config/updateConfig.sh deploy-env03
# deploy file
cp deploy/build/programs/nodultrain/nodultrain deploy-master
cp deploy/build/programs/wssultrain/wssultrain deploy-master
cp deploy/ultrainmng.tar deploy-master
cp deploy/voterand.tar deploy-master
cp deploy/ultrainmng/seedconfig.json deploy-master/ultrainmng/seedconfig.json
cp deploy/ultrainmng/config.ini deploy-master/ultrainmng/config.ini
cd deploy-master && python controller.py deploy
cd ..
cp deploy/build/programs/nodultrain/nodultrain deploy-env01
cp deploy/build/programs/wssultrain/wssultrain deploy-env01
cp deploy/ultrainmng.tar deploy-env01
cp deploy/voterand.tar deploy-env01
cp deploy/ultrainmng/seedconfig.json deploy-env01/ultrainmng/seedconfig.json
cp deploy/ultrainmng/config.ini deploy-env01/ultrainmng/config.ini
cd deploy-env01 && python controller.py deploy
cd ..
cp deploy/build/programs/nodultrain/nodultrain deploy-env02
cp deploy/build/programs/wssultrain/wssultrain deploy-env02
cp deploy/ultrainmng.tar deploy-env02
cp deploy/voterand.tar deploy-env02
cp deploy/ultrainmng/seedconfig.json deploy-env02/ultrainmng/seedconfig.json
cp deploy/ultrainmng/config.ini deploy-env02/ultrainmng/config.ini
cd deploy-env02 && python controller.py deploy
cd ..
cp deploy/build/programs/nodultrain/nodultrain deploy-env03
cp deploy/build/programs/wssultrain/wssultrain deploy-env03
cp deploy/ultrainmng.tar deploy-env03
cp deploy/voterand.tar deploy-env03
cp deploy/ultrainmng/seedconfig.json deploy-env03/ultrainmng/seedconfig.json
cp deploy/ultrainmng/config.ini deploy-env03/ultrainmng/config.ini
cd deploy-env03 && python controller.py deploy
cd ..
fab -f sendfile.py deploy
