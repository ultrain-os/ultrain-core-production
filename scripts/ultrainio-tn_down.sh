#!/bin/bash
#
# ultrainio-tn_down.sh is used by the ultrainio-tn_bounce.sh and ultrainio-tn_roll.sh scripts.
# It is intended to terminate specific ULTRAIN.IO daemon processes.
#


if [ "$PWD" != "$ULTRAINIO_HOME" ]; then
    echo $0 must only be run from $ULTRAINIO_HOME
    exit -1
fi

prog=nodultrain

DD=var/lib/node_$ULTRAINIO_NODE
runtest=`cat $DD/$prog.pid`
echo runtest = $runtest
running=`ps -e | grep $runtest | grep -cv grep `

if [ $running -ne 0 ]; then
    echo killing $prog

    pkill -15 $prog

    for (( a = 1;11-$a; a = $(($a + 1)) )); do
        echo waiting for safe termination, pass $a
        sleep 2
        running=`ps -e | grep $runtest | grep -cv grep`
        echo running = $running
        if [ $running -eq 0 ]; then
            break;
        fi
    done
fi

if [ $running -ne 0 ]; then
    echo killing $prog with SIGTERM failed, trying with SIGKILL
    pkill -9 $runtest
fi
