#!/usr/bin/env bash

echo "deploy all files"

# program
# nodultrain
echo "cp /root/mongo_setup/files/program/nodultrain /root/"
cp /root/mongo_setup/files/program/nodultrain /root/
chmod +x /root/nodultrain
# wss
echo "cp /root/mongo_setup/files/program/wssultrain /root/"
cp /root/mongo_setup/files/program/wssultrain /root/
chmod +x /root/wssultrain
# ultrainmng
echo "cp /root/mongo_setup/files/ultrainmng/ /root/ -rf"
cp /root/mongo_setup/files/ultrainmng/ /root/ -rf
chmod +x /root/ultrainmng/tool/
chmod +x /root/ultrainmng/src/sideChainService.js
# votingrand
echo "cp /root/mongo_setup/files/voterand/ /root/ -rf"
cp /root/mongo_setup/files/voterand/ /root/ -rf
chmod +x /root/voterand/migrations/votingRandService.js
sleep 1

#scripts
echo "cp /root/mongo_setup/files/scripts/runultrain-h.sh /root/"
cp /root/mongo_setup/files/scripts/runultrain-h.sh /root/
chmod +x /root/runultrain-h.sh
echo "cp /root/mongo_setup/files/scripts/logrotate.sh /root/"
cp /root/mongo_setup/files/scripts/logrotate.sh /root/
chmod +x /root/logrotate.sh
echo "cp /root/mongo_setup/files/scripts/runlogr.sh /root/"
cp /root/mongo_setup/files/scripts/runlogr.sh /root/
chmod +x /root/runlogr.sh


# config
# nodultrain
echo "mkdir /root/.local/share/ultrainio/nodultrain/config/ -p"
mkdir /root/.local/share/ultrainio/nodultrain/config/ -p
echo "cp /root/mongo_setup/files/config/nodultrain/config.ini /root/.local/share/ultrainio/nodultrain/config/"
cp /root/mongo_setup/files/config/nodultrain/config.ini /root/.local/share/ultrainio/nodultrain/config/
sleep 1
# wss
echo "mkdir /root/.local/share/ultrainio/wssultrain/config/ -p"
mkdir /root/.local/share/ultrainio/wssultrain/config/ -p
echo "cp /root/mongo_setup/files/config/wssultrain/config.ini /root/.local/share/ultrainio/wssultrain/config/"
cp /root/mongo_setup/files/config/wssultrain/config.ini /root/.local/share/ultrainio/wssultrain/config/
sleep 1
#ultrainmng
echo "mkdir /root/.local/share/ultrainio/ultrainmng/config/ -p"
mkdir /root/.local/share/ultrainio/ultrainmng/config/ -p
echo "cp /root/mongo_setup/files/config/ultrainmng/config.ini /root/.local/share/ultrainio/ultrainmng/config/"
cp /root/mongo_setup/files/config/ultrainmng/config.ini /root/.local/share/ultrainio/ultrainmng/config/
echo "cp /root/mongo_setup/files/config/ultrainmng/seedconfig.json /root/.local/share/ultrainio/ultrainmng/config/"
cp /root/mongo_setup/files/config/ultrainmng/seedconfig.json /root/.local/share/ultrainio/ultrainmng/config/

# start nod by ws
echo "start nod by ws"
echo "nohup /root/nodultrain --worldstate /root/mongo_setup/files/ws/1f1155433d9097e0f67de63a48369916da91f19cb1feff6ba8eca2e5d978a2b2-312000.ws &"
rm /root/.local/share/ultrainio/nodultrain/data/ -rf
nohup /root/nodultrain --worldstate /root/mongo_setup/files/ws/1f1155433d9097e0f67de63a48369916da91f19cb1feff6ba8eca2e5d978a2b2-312000.ws &>> /log/${DATE}-${HOSTNAME}.log &
sleep 1

# start ultrainmng
echo "pm2 start /root/ultrainmng/src/sideChainService.js"
pm2 start /root/ultrainmng/src/sideChainService.js

echo "deploy all files done."
