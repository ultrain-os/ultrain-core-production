#!/bin/bash
NAME=$1
docker ps | grep $NAME-[1-7]$ | awk '{print $1}' | xargs -i docker exec -d {} bash -c "killall 'nodultrain' "
docker ps | grep $NAME-[1-7]$ | awk '{print $1}' | xargs -i docker exec -d {} bash -c "killall 'kultraind' "
docker ps | grep $NAME-[1-7]$ | awk '{print $1}' | xargs -i docker exec -d {} bash -c "killall 'wssultrain' "
docker ps | grep $NAME-[1-7]$ | awk '{print $1}' | xargs -i docker exec -d {} bash -c "killall 'sleep' "
docker ps | grep $NAME-[1-7]$ | awk '{print $1}' | xargs -i docker exec -d {} bash -c "killall 'logrotate.sh' "
docker ps | grep $NAME-[1-7]$ | awk '{print $1}' | xargs -i docker exec -d {} bash -c "rm -rf /root/.local/share/ultrainio/nodultrain "
docker ps | grep $NAME-[1-7]$ | awk '{print $1}' | xargs -i docker exec -d {} bash -c "/usr/local/bin/pm2 stop SideChainBlockInfoService"
docker ps | grep $NAME-[1-7]$ | awk '{print $1}' | xargs -i docker exec -d {} bash -c "/usr/local/bin/pm2 stop sideChainService"
