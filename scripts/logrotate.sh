#!/bin/bash

conf="/etc/logrotate.d/ultrain-log-${HOSTNAME}.conf"
rm -f ${conf}
touch ${conf}
echo -e "/root/workspace/log/${HOSTNAME}.log\n { size 300M\n copytruncate\n  rotate 4\n  maxage 100\n }\n" >> ${conf}

while :
do
    logrotate ${conf}
    sleep 600
done
