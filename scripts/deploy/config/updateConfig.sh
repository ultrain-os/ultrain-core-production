#!/usr/bin/env bash
NAME=$1
cp /root/deploy/scripts/deploy/config/$NAME/hosts.txt /root/$NAME/
cp /root/deploy/scripts/deploy/config/$NAME/hosts-h.txt /root/$NAME/
cp /root/deploy/scripts/deploy/config/$NAME/keypair.txt /root/$NAME/
cp /root/deploy/scripts/deploy/config/$NAME/specialHost.py /root/$NAME/
cp /root/deploy/scripts/deploy/config/$NAME/template_orig.txt /root/$NAME/
cp /root/deploy/scripts/deploy/config/$NAME/toplogy-h.py /root/$NAME/
cp /root/deploy/scripts/deploy/config/controller.py /root/$NAME/
cp /root/deploy/scripts/deploy/config/fabfile4azure.py /root/$NAME/
cp /root/deploy/scripts/deploy/config/$NAME/keypiair-real.txt /root/$NAME/

