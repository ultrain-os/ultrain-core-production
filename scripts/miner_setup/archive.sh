#!/usr/bin/env bash
# please move the folder to /root/miner_setup
echo "start to archive files...."

#files
echo "rm /root/miner_setup/files -rf"
rm /root/miner_setup/files -rf
sleep 1

echo "mkdir /root/miner_setup/files -p"
mkdir /root/miner_setup/files -p
sleep 1

#program
echo "mkdir /root/miner_setup/files/program -p"
mkdir /root/miner_setup/files/program -p
sleep 1

#nodultrain
echo "cp /root/nodultrain /root/miner_setup/files/program/"
cp /root/nodultrain /root/miner_setup/files/program/
sleep 1

#wssultrain
echo "cp /root/wssultrain /root/miner_setup/files/program/"
cp /root/wssultrain /root/miner_setup/files/program/
sleep 1

#ultrainmng
echo "cp /root/ultrainmng /root/miner_setup/files/ -rf"
cp /root/ultrainmng /root/miner_setup/files/ -rf
sleep 1

#votingrand
echo "cp /root/voterand /root/miner_setup/files/ -rf"
cp /root/voterand /root/miner_setup/files/ -rf
sleep 1

#config
echo "mkdir /root/miner_setup/files/config -p"
mkdir /root/miner_setup/files/config -p
echo "mkdir /root/miner_setup/files/config/ultrainmng -p"
mkdir /root/miner_setup/files/config/ultrainmng -p
echo "cp /root/.local/share/ultrainio/ultrainmng/config/config.ini /root/miner_setup/files/config/ultrainmng/"
cp /root/.local/share/ultrainio/ultrainmng/config/config.ini /root/miner_setup/files/config/ultrainmng/
echo "cp /root/.local/share/ultrainio/ultrainmng/config/seedconfig.json /root/miner_setup/files/config/ultrainmng/"
cp /root/.local/share/ultrainio/ultrainmng/config/seedconfig.json /root/miner_setup/files/config/ultrainmng/
sleep 1
echo "mkdir /root/miner_setup/files/config/wssultrain -p"
mkdir /root/miner_setup/files/config/wssultrain -p
echo "cp /root/.local/share/ultrainio/wssultrain/config/config.ini /root/miner_setup/files/config/wssultrain/"
cp /root/.local/share/ultrainio/wssultrain/config/config.ini /root/miner_setup/files/config/wssultrain/
echo "mkdir /root/miner_setup/files/config/nodultrain -p"
mkdir /root/miner_setup/files/config/nodultrain -p
#echo "cp /root/.local/share/ultrainio/nodultrain/config/config.ini /root/miner_setup/files/config/nodultrain/"
#cp /root/.local/share/ultrainio/nodultrain/config/config.ini /root/miner_setup/files/config/nodultrain/
sleep 1

#scripts
echo "mkdir /root/miner_setup/files/scripts -p"
mkdir /root/miner_setup/files/scripts -p
echo "cp /root/runultrain-h.sh /root/miner_setup/files/scripts/"
cp /root/runultrain-h.sh /root/miner_setup/files/scripts/
echo "cp /root/runlogr.sh /root/miner_setup/files/scripts/"
cp /root/runlogr.sh /root/miner_setup/files/scripts/
echo "cp /root/logrotate.sh /root/miner_setup/files/scripts/"
cp /root/logrotate.sh /root/miner_setup/files/scripts/
echo "cp /root/logrotate.sh /root/miner_setup/files/scripts/"
echo "pm2 start /root/ultrainmng/src/sideChainService.js -o /log/sideChainService.pm2.log -e /log/sideChainService.pm2.error.log" > /root/miner_setup/files/scripts/startmng.sh
echo "pm2 start /root/voterand/migrations/votingRandService.js -o /log/votingRandService.pm2.log -e /log/votingRandService.pm2.error.log" > /root/miner_setup/files/scripts/startvote.sh












