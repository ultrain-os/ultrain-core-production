#!/usr/bin/env bash

echo "deploy all files"

# program
# nodultrain
echo "cp /root/miner_setup/files/program/nodultrain /root/"
chmod +x /root/nodultrain
# wss
echo "cp /root/miner_setup/files/program/wssultrain /root/"
cp /root/miner_setup/files/program/wssultrain /root/
chmod +x /root/wssultrain
# ultrainmng
echo "cp /root/miner_setup/files/ultrainmng/ /root/ -rf"
cp /root/miner_setup/files/ultrainmng/ /root/ -rf
chmod +x /root/ultrainmng/tool/
chmod +x /root/ultrainmng/src/sideChainService.js
# votingrand
echo "cp /root/miner_setup/files/voterand/ /root/ -rf"
cp /root/miner_setup/files/voterand/ /root/ -rf
chmod +x /root/voterand/migrations/votingRandService.js
sleep 1

#scripts
echo "cp /root/miner_setup/files/scripts/runultrain-h.sh /root/"
cp /root/miner_setup/files/scripts/runultrain-h.sh /root/
chmod +x /root/runultrain-h.sh
echo "cp /root/miner_setup/files/scripts/logrotate.sh /root/"
cp /root/miner_setup/files/scripts/logrotate.sh /root/
chmod +x /root/logrotate.sh
echo "cp /root/miner_setup/files/scripts/runlogr.sh /root/"
cp /root/miner_setup/files/scripts/runlogr.sh /root/
chmod +x /root/runlogr.sh


# config
# nodultrain
echo "mkdir /root/.local/share/ultrainio/nodultrain/config/ -p"
mkdir /root/.local/share/ultrainio/nodultrain/config/ -p
echo "cp /root/miner_setup/files/config/nodultrain/config.ini /root/.local/share/ultrainio/nodultrain/config/"
cp /root/miner_setup/files/config/nodultrain/config.ini /root/.local/share/ultrainio/nodultrain/config/
sleep 1
# wss
echo "mkdir /root/.local/share/ultrainio/wssultrain/config/ -p"
mkdir /root/.local/share/ultrainio/wssultrain/config/ -p
echo "cp /root/miner_setup/files/config/wssultrain/config.ini /root/.local/share/ultrainio/wssultrain/config/"
cp /root/miner_setup/files/config/wssultrain/config.ini /root/.local/share/ultrainio/wssultrain/config/
sleep 1
#ultrainmng
echo "mkdir /root/.local/share/ultrainio/ultrainmng/config/ -p"
mkdir /root/.local/share/ultrainio/ultrainmng/config/ -p
echo "cp /root/miner_setup/files/config/ultrainmng/config.ini /root/.local/share/ultrainio/ultrainmng/config/"
cp /root/miner_setup/files/config/ultrainmng/config.ini /root/.local/share/ultrainio/ultrainmng/config/
echo "cp /root/miner_setup/files/config/ultrainmng/seedconfig.json /root/.local/share/ultrainio/ultrainmng/config/"
cp /root/miner_setup/files/config/ultrainmng/seedconfig.json /root/.local/share/ultrainio/ultrainmng/config/

echo "deploy all files done."





