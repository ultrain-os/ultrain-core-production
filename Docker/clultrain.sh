#!/bin/bash

# Usage:
# Go into cmd loop: sudo ./clultrain.sh
# Run single cmd:  sudo ./clultrain.sh <clultrain paramers>

PREFIX="docker-compose exec nodultraind clultrain"
if [ -z $1 ] ; then
  while :
  do
    read -e -p "clultrain " cmd
    history -s "$cmd"
    $PREFIX $cmd
  done
else
  $PREFIX "$@"
fi
