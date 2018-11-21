#!/bin/bash
docker ps | grep yhc-[1-7]$ | awk '{print $1}' | xargs -i docker exec -d {} bash -c "killall 'nodultrain' "
docker ps | grep yhc-[1-7]$ | awk '{print $1}' | xargs -i docker exec -d {} bash -c "killall 'kultraind' "
docker ps | grep yhc-[1-7]$ | awk '{print $1}' | xargs -i docker exec -d {} bash -c "killall 'sleep' "
docker ps | grep yhc-[1-7]$ | awk '{print $1}' | xargs -i docker exec -d {} bash -c "killall 'logrotate.sh' "
docker ps | grep yhc-[1-7]$ | awk '{print $1}' | xargs -i docker exec -d {} bash -c "rm -rf /root/.local/share/ultrainio/nodultrain "
