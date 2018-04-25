#!/bin/bash
#
# ultrainio-tn_bounce is used to restart a node that is acting badly or is down.
# usage: ultrainio-tn_bounce.sh [arglist]
# arglist will be passed to the node's command line. First with no modifiers
# then with --replay and then a third time with --resync
#
# the data directory and log file are set by this script. Do not pass them on
# the command line.
#
# in most cases, simply running ./ultrainio-tn_bounce.sh is sufficient.
#

pushd $ULTRAINIO_HOME

if [ ! -f programs/nodultrain/nodultrain ]; then
    echo unable to locate binary for nodultrain
    exit 1
fi

config_base=etc/ultrainio/node_
if [ -z "$ULTRAINIO_NODE" ]; then
    DD=`ls -d ${config_base}[012]?`
    ddcount=`echo $DD | wc -w`
    if [ $ddcount -ne 1 ]; then
        echo $HOSTNAME has $ddcount config directories, bounce not possible. Set environment variable
        echo ULTRAINIO_NODE to the 2-digit node id number to specify which node to bounce. For example:
        echo ULTRAINIO_NODE=06 $0 \<options\>
        cd -
        exit 1
    fi
    OFS=$((${#DD}-2))
    export ULTRAINIO_NODE=${DD:$OFS}
else
    DD=${config_base}$ULTRAINIO_NODE
    if [ ! \( -d $DD \) ]; then
        echo no directory named $PWD/$DD
        cd -
        exit 1
    fi
fi

bash $ULTRAINIO_HOME/scripts/ultrainio-tn_down.sh
bash $ULTRAINIO_HOME/scripts/ultrainio-tn_up.sh $*
