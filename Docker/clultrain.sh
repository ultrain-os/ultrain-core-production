#!/bin/bash

# Usage:
# Go into cmd loop: sudo ./clultrain.sh
# Run single cmd:  sudo ./clultrain.sh <clultrain paramers>

PREFIX="docker exec docker_nodultrain_1 clultrain"
if [ -z $1 ] ; then
  while :
  do
    read -e -p "clultrain " cmd
    history -s "$cmd"
    $PREFIX $cmd
  done
else
  $PREFIX $@
fi
