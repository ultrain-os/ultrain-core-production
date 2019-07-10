#!/usr/bin/env bash
# please move the folder to ~/miner_setup
echo "start to archive files...."

#files
echo "rm ~/miner_setup/files -rf"
rm ~/miner_setup/files -rf
sleep 1

echo "mkdir ~/miner_setup/files -p"
mkdir ~/miner_setup/files -p
sleep 1

#program
echo "mkdir ~/miner_setup/files/program -p"
mkdir ~/miner_setup/files/program -p
sleep 1

#nodultrain
echo "cp ~/nodultrain ~/miner_setup/files/program/"
cp ~/nodultrain ~/miner_setup/files/program/
sleep 1

#wssultrain
echo "cp ~/wssultrain ~/miner_setup/files/program/"
cp ~/wssultrain ~/miner_setup/files/program/
sleep 1

#ultrainmng
echo "cp ~/ultrainmng ~/miner_setup/files/ -rf"
cp ~/ultrainmng ~/miner_setup/files/ -rf
sleep 1

#votingrand
echo "cp ~/voterand ~/miner_setup/files/ -rf"
cp ~/voterand ~/miner_setup/files/ -rf
sleep 1

#config
echo "mkdir ~/miner_setup/files/config -p"
mkdir ~/miner_setup/files/config -p
echo "mkdir ~/miner_setup/files/config/ultrainmng -p"
mkdir ~/miner_setup/files/config/ultrainmng -p
echo "cp ~/.local/share/ultrainio/ultrainmng/config/config.ini ~/miner_setup/files/config/ultrainmng/"
cp ~/.local/share/ultrainio/ultrainmng/config/config.ini ~/miner_setup/files/config/ultrainmng/
echo "cp ~/.local/share/ultrainio/ultrainmng/config/seedconfig.json ~/miner_setup/files/config/ultrainmng/"
cp ~/.local/share/ultrainio/ultrainmng/config/seedconfig.json ~/miner_setup/files/config/ultrainmng/
sleep 1
echo "mkdir ~/miner_setup/files/config/wssultrain -p"
mkdir ~/miner_setup/files/config/wssultrain -p
echo "cp ~/.local/share/ultrainio/wssultrain/config/config.ini ~/miner_setup/files/config/wssultrain/"
cp ~/.local/share/ultrainio/wssultrain/config/config.ini ~/miner_setup/files/config/wssultrain/
echo "mkdir ~/miner_setup/files/config/nodultrain -p"
mkdir ~/miner_setup/files/config/nodultrain -p
echo "cp ~/.local/share/ultrainio/nodultrain/config/config.ini ~/miner_setup/files/config/nodultrain/"
cp ~/.local/share/ultrainio/nodultrain/config/config.ini ~/miner_setup/files/config/nodultrain/
sleep 1

#scripts
echo "mkdir ~/miner_setup/files/scripts -p"
mkdir ~/miner_setup/files/scripts -p
echo "cp ~/runultrain-h.sh ~/miner_setup/files/scripts/"
cp ~/runultrain-h.sh ~/miner_setup/files/scripts/

#mongo-process
echo "mkdir ~/miner_setup/files/mongo"
mkdir ~/miner_setup/files/mongo
echo "cp ~/mongo_process.py ~/miner_setup/files/mongo/"
cp ~/mongo_process.py ~/miner_setup/files/mongo/













