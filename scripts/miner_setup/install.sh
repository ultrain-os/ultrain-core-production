#!/usr/bin/env bash

echo "deploy all files"

# program
# nodultrain
echo "cp ~/miner_setup/files/program/nodultrain ~/"
cp ~/miner_setup/files/program/nodultrain ~/
chmod +x ~/nodultrain
# wss
echo "cp ~/miner_setup/files/program/wssultrain ~/"
cp ~/miner_setup/files/program/wssultrain ~/
chmod +x ~/wssultrain
# ultrainmng
echo "cp ~/miner_setup/files/ultrainmng/ ~/ -rf"
cp ~/miner_setup/files/ultrainmng/ ~/ -rf
chmod +x ~/ultrainmng/tool/
chmod +x ~/ultrainmng/src/sideChainService.js
# votingrand
echo "cp ~/miner_setup/files/voterand/ ~/ -rf"
cp ~/miner_setup/files/voterand/ ~/ -rf
chmod +x ~/voterand/migrations/votingRandService.js
sleep 1

#scripts
echo "cp ~/miner_setup/files/scripts/runultrain-h.sh ~/"
cp ~/miner_setup/files/scripts/runultrain-h.sh ~/
chmod +x ~/runultrain-h.sh

# config
# nodultrain
echo "mkdir ~/.local/share/ultrainio/nodultrain/config/ -p"
mkdir ~/.local/share/ultrainio/nodultrain/config/ -p
echo "cp ~/miner_setup/files/config/nodultrain/config.ini ~/.local/share/ultrainio/nodultrain/config/"
cp ~/miner_setup/files/config/nodultrain/config.ini ~/.local/share/ultrainio/nodultrain/config/
sleep 1
# wss
echo "mkdir ~/.local/share/ultrainio/wssultrain/config/ -p"
mkdir ~/.local/share/ultrainio/wssultrain/config/ -p
echo "cp ~/miner_setup/files/config/wssultrain/config.ini ~/.local/share/ultrainio/wssultrain/config/"
cp ~/miner_setup/files/config/wssultrain/config.ini ~/.local/share/ultrainio/wssultrain/config/
sleep 1
#ultrainmng
echo "mkdir ~/.local/share/ultrainio/ultrainmng/config/ -p"
mkdir ~/.local/share/ultrainio/ultrainmng/config/ -p
echo "cp ~/miner_setup/files/config/ultrainmng/config.ini ~/.local/share/ultrainio/ultrainmng/config/"
cp ~/miner_setup/files/config/ultrainmng/config.ini ~/.local/share/ultrainio/ultrainmng/config/
echo "cp ~/miner_setup/files/config/ultrainmng/seedconfig.json ~/.local/share/ultrainio/ultrainmng/config/"
cp ~/miner_setup/files/config/ultrainmng/seedconfig.json ~/.local/share/ultrainio/ultrainmng/config/

#mongo-process
echo "cp ~/miner_setup/files/mongo/mongo_process.py ~/"
cp ~/miner_setup/files/mongo/mongo_process.py ~/
echo "chmod +x ~/mongo_process.py"
chmod +x ~/mongo_process.py

echo "deploy all files done."
