#!/bin/bash
docker ps | grep yufeng-[1-7]$ | awk '{print $1}' | xargs -i docker exec {} bash -c "killall 'nodultrain' "
docker ps | grep yufeng-[1-7]$ | awk '{print $1}' | xargs -i docker exec {} bash -c "killall 'kultraind' "
docker ps | grep yufeng-[1-7]$ | awk '{print $1}' | xargs -i docker exec {} bash -c "rm -rf /root/.local/share/ultrainio/nodultrain "

