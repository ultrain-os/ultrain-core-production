#!/bin/bash
NAME=$1
docker ps | grep $NAME- | awk '{print $1}' | xargs -i docker exec -d {} bash -c "killall 'nodultrain' "
docker ps | grep $NAME- | awk '{print $1}' | xargs -i docker exec -d {} bash -c "killall 'kultraind' "
docker ps | grep $NAME- | awk '{print $1}' | xargs -i docker exec -d {} bash -c "killall 'wssultrain' "
docker ps | grep $NAME- | awk '{print $1}' | xargs -i docker exec -d {} bash -c "killall 'sleep' "
docker ps | grep $NAME- | awk '{print $1}' | xargs -i docker exec -d {} bash -c "killall 'logrotate.sh' "
docker ps | grep $NAME- | awk '{print $1}' | xargs -i docker exec -d {} bash -c "rm -rf /root/.local/share/ultrainio/nodultrain "
docker ps | grep $NAME- | awk '{print $1}' | xargs -i docker exec -d {} bash -c "rm -rf /root/.local/share/ultrainio/wssultrain "
docker ps | grep $NAME- | awk '{print $1}' | xargs -i docker exec -d {} bash -c "rm -rf /root/log/ "
docker ps | grep $NAME- | awk '{print $1}' | xargs -i docker exec -d {} bash -c "rm -rf /log/ "
docker ps | grep $NAME- | awk '{print $1}' | xargs -i docker exec -d {} bash -c "killall  rand.sh"
